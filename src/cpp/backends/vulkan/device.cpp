// IAGHI: IA Graphics Hardware Interface
// Copyright (C) 2026 IAS (ias@iasoft.dev)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <backends/vulkan/device.hpp>

namespace ghi
{
  VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                void *pUserData);

  auto VulkanDevice::create(const InitInfo &init_info) -> Result<VulkanDevice>
  {
    VulkanDevice result{};

    result.m_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if AU_PLATFORM_WINDOWS
    result.m_instance_extensions.push_back("VK_KHR_win32_surface");
#elif AU_PLATFORM_ANDROID
    result.m_instance_extensions.push_back("VK_KHR_android_surface");
#elif AU_PLATFORM_LINUX
    result.m_instance_extensions.push_back("VK_KHR_xcb_surface");
    result.m_instance_extensions.push_back("VK_KHR_xlib_surface");
#endif

    result.m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    bool validation_enabled = init_info.validation_enabled;

    auto &logger = auxid::get_thread_logger();

    VK_CALL(volkInitialize(), "Initializing Vulkan loader");

    u32 instance_version{};
    VK_CALL(vkEnumerateInstanceVersion(&instance_version), "Enumerating Vulkan version");

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.applicationVersion = 1;
    application_info.engineVersion = 1;
    application_info.apiVersion = VULKAN_API_VERSION;
    application_info.pApplicationName = application_info.pEngineName =
        init_info.app_name ? init_info.app_name : "IAGHI Application";

    static auto validation_layer_name = "VK_LAYER_KHRONOS_validation";

    if (validation_enabled)
    {
      bool validation_found = false;

      u32 layer_count{};
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      Vec<VkLayerProperties> available_layers(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

      for (const auto &layer : available_layers)
      {
        if (strcmp(validation_layer_name, layer.layerName) == 0)
        {
          validation_found = true;
          break;
        }
      }

      if (!validation_found)
      {
        logger.warn("validation layer '%s' not found. Debugging will be disabled.", validation_layer_name);
        validation_enabled = false;
      }
    }

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};

    if (validation_enabled)
    {
      instance_create_info.enabledLayerCount = 1;
      instance_create_info.ppEnabledLayerNames = &validation_layer_name;

      debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_create_info.pfnUserCallback = debug_callback;

      instance_create_info.pNext = &debug_create_info;

      result.m_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instance_create_info.enabledExtensionCount = result.m_instance_extensions.size();
    instance_create_info.ppEnabledExtensionNames = result.m_instance_extensions.data();
    instance_create_info.pApplicationInfo = &application_info;

    if (instance_version < application_info.apiVersion)
      return fail("IAVis requires graphics hardware that supports at least Vulkan API version '%u'",
                  VULKAN_API_VERSION);

    VK_CALL(vkCreateInstance(&instance_create_info, nullptr, &result.m_instance), "Creating Vulkan instance");
    volkLoadInstance(result.m_instance);

    if (validation_enabled)
    {
      if (vkCreateDebugUtilsMessengerEXT(result.m_instance, &debug_create_info, nullptr, &result.m_debug_messenger) !=
          VK_SUCCESS)
        logger.warn("failed to set up debug messenger");
    }

    result.m_surface = static_cast<VkSurfaceKHR>(
        init_info.surface_creation_callback(result.m_instance, init_info.surface_creation_callback_user_data));
    if (!result.m_surface)
      return fail("failed to create vulkan surface");

    result.m_physical_device = AU_TRY(result.select_physical_device());

    Vec<VkDeviceQueueCreateInfo> device_queue_create_infos;

    Vec<VkQueueFamilyProperties> queue_family_props;
    VK_ENUM_CALL(vkGetPhysicalDeviceQueueFamilyProperties, queue_family_props, result.m_physical_device);

    for (u32 i = 0; i < queue_family_props.size(); i++)
    {
      const auto &props = queue_family_props[i];
      if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
      {
        if (result.m_compute_queue_family_index == UINT32_MAX)
          result.m_compute_queue_family_index = i;
        else if (result.m_graphics_queue_family_index != UINT32_MAX && i != result.m_graphics_queue_family_index)
        {
          result.m_compute_queue_family_index = i;
          break;
        }
      }
    }

    for (u32 i = 0; i < queue_family_props.size(); i++)
    {
      if (const auto &props = queue_family_props[i]; props.queueFlags & VK_QUEUE_TRANSFER_BIT)
      {
        if (result.m_transfer_queue_family_index == UINT32_MAX)
          result.m_transfer_queue_family_index = i;
        else if (i != result.m_graphics_queue_family_index && i != result.m_compute_queue_family_index)
        {
          result.m_transfer_queue_family_index = i;
          break;
        }
      }
    }

    HashMap<u32, u32> queue_family_index_map;

    if (result.m_graphics_queue_family_index != UINT32_MAX)
      queue_family_index_map[result.m_graphics_queue_family_index]++;
    if (result.m_compute_queue_family_index != UINT32_MAX)
      queue_family_index_map[result.m_compute_queue_family_index]++;
    if (result.m_transfer_queue_family_index != UINT32_MAX)
      queue_family_index_map[result.m_transfer_queue_family_index]++;

    Vec<Vec<f32>> priority_storage;
    priority_storage.reserve(queue_family_index_map.size());
    for (auto &[family_index, count] : queue_family_index_map)
    {
      VkDeviceQueueCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      info.queueFamilyIndex = family_index;

      info.queueCount = std::min(count, queue_family_props[family_index].queueCount);

      Vec<f32> priorities;
      priorities.resize(info.queueCount);
      for (u32 i = 0; i < info.queueCount; i++)
        priorities[i] = 1.0f;
      priority_storage.push_back(priorities);

      info.pQueuePriorities = priority_storage.back().data();
      device_queue_create_infos.push_back(info);
    }

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT enable_extended_dynamic_state_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = nullptr,
        .extendedDynamicState = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features enable_vulkan13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enable_extended_dynamic_state_features,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceVulkan12Features enable_vulkan12_features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext = &enable_vulkan13_features,
      .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
      .runtimeDescriptorArray = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 enable_device_features2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &enable_vulkan12_features,
        .features =
            {
                .fillModeNonSolid = true,
            },
    };

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.enabledExtensionCount = result.m_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = result.m_device_extensions.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.queueCreateInfoCount = device_queue_create_infos.size();
    device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
    device_create_info.pNext = &enable_device_features2;

    VK_CALL(vkCreateDevice(result.m_physical_device, &device_create_info, nullptr, &result.m_handle),
            "Creating logical device");

    volkLoadDevice(result.m_handle);

    HashMap<u32, u32> tmp_queue_family_index_map;
    if (result.m_graphics_queue_family_index != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[result.m_graphics_queue_family_index]++;
      if (q_index < queue_family_props[result.m_graphics_queue_family_index].queueCount)
        vkGetDeviceQueue(result.m_handle, result.m_graphics_queue_family_index, q_index, &result.m_graphics_queue);
    }
    if (result.m_compute_queue_family_index != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[result.m_compute_queue_family_index]++;
      if (q_index >= queue_family_props[result.m_compute_queue_family_index].queueCount)
        q_index = 0;

      vkGetDeviceQueue(result.m_handle, result.m_compute_queue_family_index, q_index, &result.m_compute_queue);
    }
    if (result.m_transfer_queue_family_index != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[result.m_transfer_queue_family_index]++;
      if (q_index >= queue_family_props[result.m_transfer_queue_family_index].queueCount)
        q_index = 0;

      vkGetDeviceQueue(result.m_handle, result.m_transfer_queue_family_index, q_index, &result.m_transfer_queue);
    }

    VmaAllocatorCreateInfo allocator_create_info{
        .physicalDevice = result.m_physical_device,
        .device = result.m_handle,
        .instance = result.m_instance,
        .vulkanApiVersion = VULKAN_API_VERSION,
    };
    VmaVulkanFunctions vma_vulkan_functions{};
    VK_CALL(vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vma_vulkan_functions), "Importing VMA functions");
    allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
    VK_CALL(vmaCreateAllocator(&allocator_create_info, &result.m_allocator), "Creating VMA allocator");

    const VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = result.m_graphics_queue_family_index,
    };
    VK_CALL(
        vkCreateCommandPool(result.m_handle, &command_pool_create_info, nullptr, &result.m_single_time_command_pool),
        "Creating single time command pool");

    const VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    VK_CALL(vkCreateFence(result.m_handle, &fence_create_info, nullptr, &result.m_single_time_command_fence),
            "Creating single time command fence");

    result.m_swapchain = AU_TRY(VulkanSwapchain::create(result, init_info.surface_width, init_info.surface_height));

    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000}};

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 4;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = 1000;

    VK_CALL(vkCreateDescriptorPool(result.m_handle, &pool_info, nullptr, &result.m_descriptor_pool),
            "create descriptor pool");

    return result;
  }

  auto VulkanDevice::destroy() -> void
  {
    vkDeviceWaitIdle(m_handle);

    vkDestroyDescriptorPool(m_handle, m_descriptor_pool, nullptr);
    vkDestroyFence(m_handle, m_single_time_command_fence, nullptr);
    vkDestroyCommandPool(m_handle, m_single_time_command_pool, nullptr);

    m_swapchain.destroy(*this);

    vmaDestroyAllocator(m_allocator);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_handle, nullptr);

    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
  }

  auto VulkanDevice::submit_and_present(VkCommandBuffer cmd, VkFence fence) -> bool
  {
    const auto &sync_frame = m_swapchain.m_frames[m_swapchain.m_current_sync_frame_index];
    const auto &image_frame = m_swapchain.m_frames[m_swapchain.m_current_frame_index];

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &sync_frame.image_available_semaphore,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &image_frame.render_finished_semaphore,
    };

    if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, fence) != VK_SUCCESS)
      return false;

    m_swapchain.present(*this);

    return true;
  }

  auto VulkanDevice::wait_idle() -> void
  {
    vkDeviceWaitIdle(m_handle);
  }

  auto VulkanDevice::execute_single_time_commands(std::function<void(VkCommandBuffer)> commands) -> Result<void>
  {
    VkCommandBufferAllocateInfo command_alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_single_time_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    VK_CALL(vkAllocateCommandBuffers(m_handle, &command_alloc_info, &cmd), "Allocating general command buffer");

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &begin_info);

    commands(cmd);

    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT};
    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    if (vkResetFences(m_handle, 1, &m_single_time_command_fence) != VK_SUCCESS)
      return fail("Failed to reset single time command fence");

    if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_single_time_command_fence) != VK_SUCCESS)
      return fail("Failed to submit single time commands");

    VK_CALL(vkWaitForFences(m_handle, 1, &m_single_time_command_fence, VK_TRUE, UINT64_MAX),
            "Waiting for single time command fence");

    VK_CALL(vkResetCommandPool(m_handle, m_single_time_command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT),
            "Resetting single time command pool");

    return {};
  }

  auto VulkanDevice::select_physical_device() -> Result<VkPhysicalDevice>
  {
    bool found = false;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    auto &logger = auxid::get_thread_logger();

    VkPhysicalDeviceProperties props{};
    VkPhysicalDeviceFeatures features{};
    Vec<VkPhysicalDevice> physical_devices;
    VK_ENUM_CALL(vkEnumeratePhysicalDevices, physical_devices, m_instance);

    for (const auto &pd : physical_devices)
    {
      vkGetPhysicalDeviceProperties(pd, &props);
      vkGetPhysicalDeviceFeatures(pd, &features);

      if (props.deviceType != VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        continue;

      u32 queue_count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_count, nullptr);
      Vec<VkQueueFamilyProperties> queues(queue_count);
      vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_count, queues.data());

      for (u32 i = 0; i < queue_count; i++)
      {
        VkBool32 supports_present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, m_surface, &supports_present);

        if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supports_present)
        {
          physical_device = pd;
          m_graphics_queue_family_index = i;
          found = true;
        }
      }
    }

    if (!found)
      return fail("failed to find suitable graphics hardware.");

    logger.info("using the hardware device '%s'", props.deviceName);

    return physical_device;
  }

  VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                void *pUserData)
  {
    AU_UNUSED(pUserData);
    AU_UNUSED(messageType);

    auto &logger = auxid::get_thread_logger();

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
      logger.error("[Validation]: %s", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
      logger.warn("[Validation]: %s", pCallbackData->pMessage);

    return VK_FALSE;
  }
} // namespace ghi