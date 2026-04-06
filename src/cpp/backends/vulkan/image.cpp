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

#include <backends/vulkan/image.hpp>

namespace ghi
{
  auto VulkanImage::create(VkDevice device, VmaAllocator allocator, VkFormat format, VkExtent3D extent,
                           VkImageUsageFlags usage, VkImageUsageFlags aspect_flags,u32 layer_count, u32 mip_level_count) -> Result<VulkanImage>
  {
    VulkanImage result{};

    result.m_format = format;
    result.m_extent = extent;
    result.m_layer_count = layer_count;
    result.m_mip_level_count = mip_level_count;

    VkImageCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = mip_level_count,
        .arrayLayers = layer_count,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImageViewCreateInfo view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = (layer_count == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .format = create_info.format,
        .subresourceRange =
            {
                .aspectMask = aspect_flags,
                .baseMipLevel = 0,
                .levelCount = mip_level_count,
                .baseArrayLayer = 0,
                .layerCount = layer_count,
            },
    };

    VmaAllocationCreateInfo alloc_info{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VMA_MEMORY_USAGE_GPU_ONLY,
        .preferredFlags = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    VK_CALL(vkCreateImage(device, &create_info, nullptr, &result.m_handle), "creating image");

    VK_CALL(vmaAllocateMemoryForImage(allocator, result.m_handle, &alloc_info, &result.m_allocation,
                                      &result.m_allocation_info),
            "allocating image memory");
    VK_CALL(vkBindImageMemory(device, result.m_handle, result.m_allocation_info.deviceMemory, 0),
            "binding image memory");

    view_create_info.image = result.m_handle;
    VK_CALL(vkCreateImageView(device, &view_create_info, nullptr, &result.m_view), "creating image view");

    return result;
  }

  auto VulkanImage::destroy(VkDevice device, VmaAllocator allocator) -> void
  {
    vkDestroyImage(device, m_handle, nullptr);
    vkDestroyImageView(device, m_view, nullptr);
    vmaFreeMemory(allocator, m_allocation);
  }
} // namespace ghi