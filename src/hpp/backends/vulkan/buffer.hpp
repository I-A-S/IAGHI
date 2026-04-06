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
  struct VulkanBuffer
  {
    u64 size{};
    VkBuffer handle{};
    VmaAllocation allocation{};
    VmaAllocationInfo alloc_info{};
    const VmaAllocator allocator_ref;

    explicit VulkanBuffer(VmaAllocator allocator) : allocator_ref(allocator)
    {
    }

    static auto create(VmaAllocator allocator, u64 size, VkBufferUsageFlags usage, bool host_visible,
                       const char *debug_name = "<not_set>") -> Result<VulkanBuffer>;

    auto destroy() -> void;

    auto cmd_copy_to_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset = 0,
                            u64 dst_offset = 0) const -> void;
    auto cmd_copy_from_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset = 0, u64 dst_offset = 0)
        -> void;

    friend class VulkanBackend;
    friend class VulkanDeviceLocalBuffer;
    friend class VulkanHostVisibleBuffer;
  };

  class VulkanDeviceLocalBuffer
  {
public:
    VulkanDeviceLocalBuffer(): m_buffer(VK_NULL_HANDLE) {}

    explicit VulkanDeviceLocalBuffer(VulkanBuffer &&data) : m_buffer(std::move(data))
    {
    }

    ~VulkanDeviceLocalBuffer() = default;

    static auto create(VmaAllocator allocator, u64 size, VkBufferUsageFlags usage, const char *debug_name = "<not_set>")
        -> Result<VulkanDeviceLocalBuffer>
    {
      auto buffer = AU_TRY(VulkanBuffer::create(allocator, size, usage, false, debug_name));
      return VulkanDeviceLocalBuffer(std::move(buffer));
    }

    auto destroy() -> void
    {
      m_buffer.destroy();
    }

    auto cmd_copy_to_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset = 0,
                            u64 dst_offset = 0) const -> void
    {
      m_buffer.cmd_copy_to_buffer(cmd, buffer, size, src_offset, dst_offset);
    }

    auto cmd_copy_from_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset = 0, u64 dst_offset = 0)
        -> void
    {
      m_buffer.cmd_copy_from_buffer(cmd, buffer, size, src_offset, dst_offset);
    }

    [[nodiscard]] auto get_size() const -> u64
    {
      return m_buffer.size;
    }

    [[nodiscard]] auto get_handle() const -> VkBuffer
    {
      return m_buffer.handle;
    }

private:
    VulkanBuffer m_buffer;
  };

  class VulkanHostVisibleBuffer
  {
public:
    VulkanHostVisibleBuffer(): m_buffer(VK_NULL_HANDLE) {}

    explicit VulkanHostVisibleBuffer(VulkanBuffer &&data) : m_buffer(std::move(data))
    {
    }

    ~VulkanHostVisibleBuffer() = default;

    static auto create(VmaAllocator allocator, u64 size, VkBufferUsageFlags usage, const char *debug_name = "<not_set>")
        -> Result<VulkanHostVisibleBuffer>
    {
      auto buffer = AU_TRY(VulkanBuffer::create(allocator, size, usage, true, debug_name));
      return VulkanHostVisibleBuffer(std::move(buffer));
    }

    auto destroy() -> void
    {
      m_buffer.destroy();
    }

    auto map(u64 size, u64 offset = 0) -> void *
    {
      if (m_buffer.alloc_info.pMappedData)
        return m_buffer.alloc_info.pMappedData;

      void* data;
      vmaMapMemory(m_buffer.allocator_ref, m_buffer.allocation, &data);
      return data;
    }

    auto unmap() const -> void
    {
      if (!m_buffer.alloc_info.pMappedData)
        vmaUnmapMemory(m_buffer.allocator_ref, m_buffer.allocation);
    }

    auto cmd_copy_to_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset = 0,
                            u64 dst_offset = 0) const -> void
    {
      m_buffer.cmd_copy_to_buffer(cmd, buffer, size, src_offset, dst_offset);
    }

    auto cmd_copy_from_buffer(VkCommandBuffer cmd, VkBuffer buffer, u64 size, u64 src_offset = 0, u64 dst_offset = 0)
        -> void
    {
      m_buffer.cmd_copy_from_buffer(cmd, buffer, size, src_offset, dst_offset);
    }

    [[nodiscard]] auto get_size() const -> u64
    {
      return m_buffer.size;
    }

    [[nodiscard]] auto get_handle() const -> VkBuffer
    {
      return m_buffer.handle;
    }

private:
    VulkanBuffer m_buffer;
  };
}