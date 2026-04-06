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

#include <backends/vulkan/backend.hpp>

namespace ghi
{
  class VulkanImage
  {
public:
    static auto create(VkDevice device, VmaAllocator allocator, VkFormat format, VkExtent3D extent,
                       VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                       VkImageUsageFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT, u32 layer_count = 1, u32 mip_level_count = 1) -> Result<VulkanImage>;
    auto destroy(VkDevice device, VmaAllocator allocator) -> void;

    [[nodiscard]] auto get_handle() const -> VkImage
    {
      return m_handle;
    }

    [[nodiscard]] auto get_view() const -> VkImageView
    {
      return m_view;
    }

    [[nodiscard]] auto get_format() const -> VkFormat
    {
      return m_format;
    }

    [[nodiscard]] auto get_extent() const -> VkExtent3D
    {
      return m_extent;
    }

    [[nodiscard]] auto get_layer_count() const -> u32
    {
      return m_layer_count;
    }

    [[nodiscard]] auto get_mip_level_count() const -> u32
    {
      return m_mip_level_count;
    }

private:
    VkImage m_handle{};
    VkImageView m_view{};
    VkFormat m_format{};
    VkExtent3D m_extent{};
    u32 m_layer_count{};
    u32 m_mip_level_count{};
    VmaAllocation m_allocation{};
    VmaAllocationInfo m_allocation_info{};
  };
} // namespace ghi