// IAGHI: IA Graphics Hardware Interface
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
// Copyright (C) 2026 IASoft (PVT) LTD (contact@iasoft.dev)
//
// This source code is licensed under the PolyForm Noncommercial License 1.0.0.
// A copy of this license is included in the LICENSE file at the root of this project,
// and is also available at <https://polyformproject.org/licenses/noncommercial/1.0.0>.

#pragma once

#include <backends/vulkan/backend.hpp>

namespace ghi
{
  class VulkanDevice;

  class VulkanBuffer
  {
public:
    explicit VulkanBuffer(VulkanDevice &device_ref) : m_device_ref(device_ref)
    {
    }

    static auto create(VulkanDevice &device, u64 size, VkBufferUsageFlags usage, bool is_dynamic, bool is_frame_bound,
                       bool host_visible, const char *debug_name = "<not_set>") -> Result<VulkanBuffer>;

    auto destroy() -> void;

    auto map(u32 frame_index) -> void *;
    auto unmap() -> void;
    auto flush(u64 offset = 0, u64 size = VK_WHOLE_SIZE) -> void;
    auto invalidate(u64 offset = 0, u64 size = VK_WHOLE_SIZE) -> void;

public:
    auto get_device() -> VulkanDevice &
    {
      return m_device_ref;
    }

    auto get_handle() const -> VkBuffer
    {
      return m_handle;
    }

    auto get_stride() -> u64
    {
      return m_stride;
    }

    auto is_dynamic() -> bool
    {
      return m_is_dynamic;
    }

    auto is_frame_bound() -> bool
    {
      return m_is_frame_bound;
    }

    auto get_unit_size() -> u64
    {
      return m_unit_size;
    }

    auto get_size() -> u64
    {
      return m_size;
    }

    auto get_unit_count() -> u64
    {
      return m_size / m_stride;
    }

protected:
    u64 m_size{};
    u64 m_stride{};
    u64 m_unit_size{};
    bool m_is_dynamic{};
    bool m_is_frame_bound{};
    VkBuffer m_handle{};
    VulkanDevice &m_device_ref;
    VmaAllocation m_allocation{};
    VmaAllocationInfo m_alloc_info{};
  };
} // namespace ghi