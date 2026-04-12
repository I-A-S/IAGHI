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
#include <backends/vulkan/device.hpp>

namespace ghi
{
  auto VulkanBuffer::create(VulkanDevice &device, u64 size, VkBufferUsageFlags usage, bool is_dynamic, bool host_visible,
                            const char *debug_name) -> Result<VulkanBuffer>
  {
    const auto is_uniform_buffer = usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    const auto is_storage_buffer = usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    assert(!is_dynamic || is_uniform_buffer || is_storage_buffer);

    VulkanBuffer result{device};

    const auto allocator = device.get_allocator();

    result.m_size = size;
    result.m_unit_size = size;
    result.m_is_dynamic = is_dynamic;

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

    if (is_dynamic)
    {
      u64 min_alignment = 1;
      u64 max_range = ~0ULL;

      if (is_uniform_buffer) {
        min_alignment = device.get_min_uniform_buffer_offset_alignment();
        max_range = device.get_max_uniform_buffer_range();
      } else if (is_storage_buffer) {
        min_alignment = device.get_min_storage_buffer_offset_alignment();
        max_range = device.get_max_storage_buffer_range();
      }

      if (size > max_range) {
        return fail("Single element size exceeds maximum descriptor binding range");
      }

      u64 aligned_stride = size;
      if (min_alignment > 0) {
        aligned_stride = (aligned_stride + min_alignment - 1) & ~(min_alignment - 1);
      }

      result.m_stride = aligned_stride;

      buffer_info.size = aligned_stride * device.get_swapchain().get_backbuffer_image_count();
    }
    else
    {
      result.m_stride = size;
    }

    VK_CALL(vmaCreateBuffer(allocator, &buffer_info, &alloc_create_info, &result.m_handle,
                               &result.m_allocation, &result.m_alloc_info),
               "Creating buffer");
    vmaSetAllocationName(allocator, result.m_allocation, debug_name);

    return result;
  }

  auto VulkanBuffer::destroy() -> void
  {
    vmaDestroyBuffer(m_device_ref.get_allocator(), m_handle, m_allocation);
    m_handle = VK_NULL_HANDLE;
  }

  auto VulkanBuffer::map() -> void*
  {
    if (m_alloc_info.pMappedData != nullptr)
      return m_alloc_info.pMappedData;

    void* mapped_data = nullptr;
    if (vmaMapMemory(m_device_ref.get_allocator(), m_allocation, &mapped_data) != VK_SUCCESS)
      return nullptr;

    return mapped_data;
  }

  auto VulkanBuffer::unmap() -> void
  {
    if (m_alloc_info.pMappedData != nullptr)
      return;

    vmaUnmapMemory(m_device_ref.get_allocator(), m_allocation);
  }

  auto VulkanBuffer::flush(u64 offset, u64 size) -> void
  {
    vmaFlushAllocation(m_device_ref.get_allocator(), m_allocation, offset, size);
  }

  auto VulkanBackend::create_buffers(Device device, Span<const BufferDesc> descs, Buffer *out_handles) -> Result<void>
  {
  }

  auto VulkanBackend::destroy_buffers(Device device, Span<const Buffer> handles) -> void
  {
  }

  auto VulkanBackend::map_buffer(Device device, Buffer buffer) -> void *
  {
  }

  auto VulkanBackend::unmap_buffer(Device device, Buffer buffer) -> void
  {
  }
} // namespace ghi