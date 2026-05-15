// IAGHI: IA Graphics Hardware Interface
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
// Copyright (C) 2026 IASoft (PVT) LTD (contact@iasoft.dev)
//
// This source code is licensed under the PolyForm Noncommercial License 1.0.0.
// A copy of this license is included in the LICENSE file at the root of this project,
// and is also available at <https://polyformproject.org/licenses/noncommercial/1.0.0>.

#pragma once

#include <backends/vulkan/swapchain.hpp>

namespace ghi
{
  class VulkanDevice
  {
public:
    static auto create(const InitInfo &init_info) -> Result<VulkanDevice>;
    auto destroy() -> void;

    auto submit_and_present(VkCommandBuffer cmd, VkFence fence) -> bool;

    auto wait_idle() -> void;

    auto execute_single_time_commands(std::function<void(VkCommandBuffer)> commands) -> Result<void>;

public:
    [[nodiscard]] auto get_handle() const -> VkDevice
    {
      return m_handle;
    }

    auto get_swapchain() -> VulkanSwapchain &
    {
      return m_swapchain;
    }

    auto get_descriptor_pool() -> VkDescriptorPool &
    {
      return m_descriptor_pool;
    }

    auto get_allocator() -> VmaAllocator &
    {
      return m_allocator;
    }

    auto get_min_uniform_buffer_offset_alignment() const -> u32
    {
      return m_physical_device_properties.limits.minUniformBufferOffsetAlignment;
    }

    auto get_min_storage_buffer_offset_alignment() const -> u32
    {
      return m_physical_device_properties.limits.minStorageBufferOffsetAlignment;
    }

    auto get_max_uniform_buffer_range() const -> u32
    {
      return m_physical_device_properties.limits.maxUniformBufferRange;
    }

    auto get_max_storage_buffer_range() const -> u32
    {
      return m_physical_device_properties.limits.maxStorageBufferRange;
    }

private:
    VkDevice m_handle{};
    VkInstance m_instance{};
    VmaAllocator m_allocator{};
    VkPhysicalDevice m_physical_device{};
    VkDebugUtilsMessengerEXT m_debug_messenger{};

    Vec<const char *> m_device_extensions{};
    Vec<const char *> m_instance_extensions{};

    u32 m_graphics_queue_family_index{UINT32_MAX};
    u32 m_compute_queue_family_index{UINT32_MAX};
    u32 m_transfer_queue_family_index{UINT32_MAX};

    VkQueue m_graphics_queue{};
    VkQueue m_compute_queue{};
    VkQueue m_transfer_queue{};

    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    VulkanSwapchain m_swapchain{};

    VkFence m_single_time_command_fence{};
    VkCommandPool m_single_time_command_pool{};

    VkDescriptorPool m_descriptor_pool{};

    VkPhysicalDeviceProperties m_physical_device_properties{};

private:
    auto select_physical_device() -> Result<Pair<VkPhysicalDevice, VkPhysicalDeviceProperties>>;

    friend class VulkanBackend;
    friend class VulkanSwapchain;
  };
} // namespace ghi