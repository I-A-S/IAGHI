// IAGHI: IA Graphics Hardware Interface
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
// Copyright (C) 2026 IASoft (PVT) LTD (contact@iasoft.dev)
//
// This source code is licensed under the PolyForm Noncommercial License 1.0.0.
// A copy of this license is included in the LICENSE file at the root of this project,
// and is also available at <https://polyformproject.org/licenses/noncommercial/1.0.0>.

#pragma once

#include <backends/vulkan/image.hpp>

namespace ghi
{
  class VulkanDevice;

  class VulkanSwapchain
  {
public:
    struct Frame
    {
      VkFence in_use_fence;
      VkSemaphore image_available_semaphore;
      VkSemaphore render_finished_semaphore;
      VkCommandBuffer command_buffer;
      VkCommandPool command_pool;

      VkImage swapchain_image;
      VkImageView swapchain_image_view;

      VulkanImage depth_image;
      VulkanImage offscreen_color_image;
    };

public:
    static auto create(VulkanDevice &device, u32 width, u32 height) -> Result<VulkanSwapchain>;
    auto destroy(VulkanDevice &device) -> void;

    auto recreate(VulkanDevice &device) -> Result<void>;
    auto recreate(VulkanDevice &device, u32 width, u32 height) -> Result<void>;

    auto advance_frame(VkDevice device, VkSurfaceKHR surface) -> Result<bool>;

    auto present(VkQueue graphics_queue, VkSurfaceKHR surface) -> void;

public:
    auto get_frame() -> Frame &
    {
      return m_frames[m_current_sync_frame_index];
    }

    auto get_extent() const -> VkExtent2D
    {
      return m_extent;
    }

    auto get_backbuffer_images(VkImage &color, VkImage &depth) const -> void
    {
      color = m_frames[m_current_frame_index].swapchain_image;
      depth = m_frames[m_current_frame_index].depth_image.get_handle();
    }

    auto get_backbuffer_views(VkImageView &color, VkImageView &depth) const -> void
    {
      color = m_frames[m_current_frame_index].swapchain_image_view;
      depth = m_frames[m_current_frame_index].depth_image.get_view();
    }

    [[nodiscard]] auto get_backbuffer_image_count() const -> u32
    {
      return m_buffer_count;
    }

    [[nodiscard]] auto get_current_frame_index() const -> u32
    {
      return m_current_sync_frame_index;
    }

    [[nodiscard]] auto get_color_format() const -> VkFormat
    {
      return m_format;
    }

    [[nodiscard]] auto get_depth_format() const -> VkFormat
    {
      return m_depth_format;
    }

private:
    VkSwapchainKHR m_handle{};
    VkExtent2D m_extent{};
    VkFormat m_format{};
    VkFormat m_depth_format{};
    VkColorSpaceKHR m_colorspace{};
    VkExtent2D m_min_extent{};
    VkExtent2D m_max_extent{};
    u32 m_buffer_count{};

    u32 m_current_frame_index{};
    u32 m_current_sync_frame_index{};
    Frame m_frames[NUM_FRAMES_BUFFERED]{};

    friend class VulkanDevice;
    friend class VulkanBackend;
  };
} // namespace ghi