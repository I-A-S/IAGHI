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

#include <backends/vulkan/buffer.hpp>

namespace ghi
{
auto VulkanBuffer::create(VmaAllocator allocator, u64 size, VkBufferUsageFlags usage, bool host_visible,
                                   const char *debug_name) -> Result<VulkanBuffer>
  {
    VulkanBuffer result{allocator};

    VkBufferCreateInfo buffer_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };

    VmaAllocationCreateInfo alloc_create_info{
        .flags = host_visible
                     ? static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                             VMA_ALLOCATION_CREATE_MAPPED_BIT)
                     : static_cast<VmaAllocationCreateFlags>(0),
        .usage = host_visible ? VMA_MEMORY_USAGE_AUTO : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    VK_CALL(vmaCreateBuffer(allocator, &buffer_info, &alloc_create_info, &result.handle, &result.allocation,
                            &result.alloc_info),
            "Creating buffer");

    vmaSetAllocationName(result.allocator_ref, result.allocation, debug_name);

    return result;
  }

  auto VulkanBuffer::destroy() -> void
  {
    vmaDestroyBuffer(allocator_ref, handle, allocation);
    handle = VK_NULL_HANDLE;
  }

  auto VulkanBuffer::cmd_copy_to_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset,
                                               u64 dst_offset) const -> void
  {
    VkBufferCopy copy_region{
        .srcOffset = src_offset,
        .dstOffset = dst_offset,
        .size = size,
    };
    vkCmdCopyBuffer(cmd, handle, buffer, 1, &copy_region);
  }

  auto VulkanBuffer::cmd_copy_from_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset,
                                                 u64 dst_offset) -> void
  {
    VkBufferCopy copy_region{
        .srcOffset = src_offset,
        .dstOffset = dst_offset,
        .size = size,
    };
    vkCmdCopyBuffer(cmd, buffer, handle, 1, &copy_region);
  }
}