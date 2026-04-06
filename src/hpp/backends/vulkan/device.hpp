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

    auto get_descriptor_pool() -> VkDescriptorPool &
    {
      return m_descriptor_pool;
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

    VkSurfaceKHR m_surface{};

    VulkanSwapchain m_swapchain{};

    VkFence m_single_time_command_fence{};
    VkCommandPool m_single_time_command_pool{};

    VkDescriptorPool m_descriptor_pool{};

private:
    auto select_physical_device() -> Result<VkPhysicalDevice>;

    friend class VulkanBackend;
    friend class VulkanSwapchain;
  };
} // namespace ghi