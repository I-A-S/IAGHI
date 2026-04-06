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

#include <backends/vulkan/swapchain.hpp>
#include <backends/vulkan/device.hpp>

namespace ghi
{
  auto VulkanSwapchain::create(VulkanDevice &device, u32 width, u32 height) -> Result<VulkanSwapchain>
  {
    VulkanSwapchain result{};

    VkSurfaceFormatKHR selected_surface_format{};
    Vec<VkSurfaceFormatKHR> surface_formats;
    VK_ENUM_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR, surface_formats, device.m_physical_device, device.m_surface);
    for (const auto &format : surface_formats)
    {
      selected_surface_format = format;
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
        break;
    }
    result.m_format = selected_surface_format.format;
    result.m_colorspace = selected_surface_format.colorSpace;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    VK_CALL(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.m_physical_device, device.m_surface, &surface_capabilities),
        "Fetching surface capabilities");
    result.m_buffer_count = std::max(NUM_FRAMES_BUFFERED, surface_capabilities.minImageCount);
    if (surface_capabilities.maxImageCount > 0)
      result.m_buffer_count = std::min(result.m_buffer_count, surface_capabilities.maxImageCount);
    result.m_min_extent = surface_capabilities.minImageExtent;
    result.m_max_extent = surface_capabilities.maxImageExtent;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device.m_graphics_queue_family_index,
    };

    VkCommandBufferAllocateInfo command_alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    for (i32 i = 0; i < result.m_buffer_count; i++)
    {
      VK_CALL(vkCreateSemaphore(device.m_handle, &semaphore_create_info, nullptr,
                                &result.m_frames[i].render_finished_semaphore),
              "Creating frame render finished semaphore");
      VK_CALL(vkCreateFence(device.m_handle, &fence_create_info, nullptr, &result.m_frames[i].in_use_fence),
              "Creating frame inflight fence");
      VK_CALL(
          vkCreateCommandPool(device.m_handle, &command_pool_create_info, nullptr, &result.m_frames[i].command_pool),
          "Creating frame command pool");

      command_alloc_info.commandPool = result.m_frames[i].command_pool;
      VK_CALL(vkAllocateCommandBuffers(device.m_handle, &command_alloc_info, &result.m_frames[i].command_buffer),
              "Allocating frame command buffer");
    }

    result.m_handle = VK_NULL_HANDLE;

    AU_TRY_DISCARD(result.recreate(device));

    return result;
  }

  auto VulkanSwapchain::destroy(VulkanDevice &device) -> void
  {
    vkDeviceWaitIdle(device.m_handle);

    for (auto &frame : m_frames)
    {
      frame.depth_image.destroy(device.m_handle, device.m_allocator);
      vkDestroySemaphore(device.m_handle, frame.render_finished_semaphore, nullptr);
      vkDestroyFence(device.m_handle, frame.in_use_fence, nullptr);
      vkDestroyCommandPool(device.m_handle, frame.command_pool, nullptr);
      vkDestroyImageView(device.m_handle, frame.swapchain_image_view, nullptr);
      vkDestroySemaphore(device.m_handle, frame.image_available_semaphore, nullptr);
    }
    vkDestroySwapchainKHR(device.m_handle, m_handle, nullptr);
  }

  auto VulkanSwapchain::recreate(VulkanDevice &device) -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    VK_CALL(vkDeviceWaitIdle(device.m_handle), "Waiting device idle");

    if (m_handle != VK_NULL_HANDLE)
    {
      for (i32 i = 0; i < m_buffer_count; i++)
      {
        vkDestroyImageView(device.m_handle, m_frames[i].swapchain_image_view, nullptr);
        vkDestroySemaphore(device.m_handle, m_frames[i].image_available_semaphore, nullptr);
        m_frames[i].depth_image.destroy(device.m_handle, device.m_allocator);
      }
    }

    m_extent.width = std::min(std::max(m_extent.width, m_min_extent.width), m_max_extent.width);
    m_extent.height = std::min(std::max(m_extent.height, m_min_extent.height), m_max_extent.height);

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.oldSwapchain = m_handle;
    create_info.surface = device.m_surface;
    create_info.imageFormat = m_format;
    create_info.imageColorSpace = m_colorspace;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.queueFamilyIndexCount = 1;
    create_info.pQueueFamilyIndices = &device.m_graphics_queue_family_index;
    create_info.minImageCount = m_buffer_count;
    create_info.imageExtent = m_extent;
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CALL(vkCreateSwapchainKHR(device.m_handle, &create_info, nullptr, &m_handle), "Creating swapchain");
    vkDestroySwapchainKHR(device.m_handle, create_info.oldSwapchain, nullptr);

    Vec<VkImage> swapchain_images;
    VK_ENUM_CALL(vkGetSwapchainImagesKHR, swapchain_images, device.m_handle, m_handle);
    i32 frame_index{0};
    for (const auto &img : swapchain_images)
    {
      VkImageView view{};

      VkImageViewCreateInfo view_create_info{};
      view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_create_info.image = img;
      view_create_info.format = m_format;
      view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      view_create_info.subresourceRange.baseArrayLayer = 0;
      view_create_info.subresourceRange.baseMipLevel = 0;
      view_create_info.subresourceRange.layerCount = 1;
      view_create_info.subresourceRange.levelCount = 1;
      VK_CALL(vkCreateImageView(device.m_handle, &view_create_info, nullptr, &view), "Creating swapchain image view");

      m_frames[frame_index].swapchain_image = img;
      m_frames[frame_index++].swapchain_image_view = view;
    }

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (i32 i = 0; i < m_buffer_count; i++)
    {
      vkDestroySemaphore(device.m_handle, m_frames[i].image_available_semaphore, nullptr);
      VK_CALL(
          vkCreateSemaphore(device.m_handle, &semaphore_create_info, nullptr, &m_frames[i].image_available_semaphore),
          "Creating swapchain semaphore");

      m_frames[i].depth_image =
          AU_TRY(VulkanImage::create(device.m_handle, device.m_allocator, VK_FORMAT_D32_SFLOAT,
                                     {.width = m_extent.width, .height = m_extent.height, .depth = 1},
                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT));
    }

    Vec<ImageBarrier> depth_barriers;
    for (i32 i = 0; i < m_buffer_count; i++)
    {
      depth_barriers.push_back(ImageBarrier{
          .image = reinterpret_cast<Image>(&m_frames[i].depth_image),
          .old_state = EResourceState::Undefined,
          .new_state = EResourceState::DepthTarget,
      });
    }
    AU_TRY_DISCARD(device.execute_single_time_commands([&](VkCommandBuffer cmd) {
      cmd_pipeline_barrier(reinterpret_cast<CommandBuffer>(cmd), 0, nullptr, m_buffer_count, depth_barriers.data());
    }));

    logger.info("recreated swapchain (%ux%ux%u)", m_extent.width, m_extent.height, create_info.minImageCount);

    return {};
  }

  auto VulkanSwapchain::advance_frame(VulkanDevice &device) -> bool
  {
    const auto &frame = m_frames[m_current_sync_frame_index];

    vkWaitForFences(device.m_handle, 1, &frame.in_use_fence, VK_TRUE, UINT64_MAX);

    u32 image_index{};
    const auto result = vkAcquireNextImageKHR(device.m_handle, m_handle, UINT64_MAX, frame.image_available_semaphore,
                                              VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      (void) recreate(device);
      return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      return false;

    vkResetFences(device.m_handle, 1, &frame.in_use_fence);

    m_current_frame_index = image_index;

    return true;
  }

  auto VulkanSwapchain::present(VulkanDevice &device) -> void
  {
    const auto &image_frame = m_frames[m_current_frame_index];

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pImageIndices = &m_current_frame_index;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_handle;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &image_frame.render_finished_semaphore;

    vkQueuePresentKHR(device.m_graphics_queue, &present_info);

    m_current_sync_frame_index = (m_current_sync_frame_index + 1) % m_buffer_count;
  }
} // namespace ghi