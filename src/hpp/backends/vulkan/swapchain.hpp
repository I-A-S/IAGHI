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
    };

public:
    static auto create(VulkanDevice& device, u32 width, u32 height) -> Result<VulkanSwapchain>;
    auto destroy(VulkanDevice& device) -> void;

    auto recreate(VulkanDevice& device) -> Result<void>;
    auto recreate(VulkanDevice& device, u32 width, u32 height) -> Result<void>;

    auto advance_frame(VulkanDevice& device) -> bool;

    auto present(VulkanDevice& device) -> void;

  public:
    auto get_frame() -> Frame&
    {
      return m_frames[m_current_sync_frame_index];
    }

    auto get_extent() const -> VkExtent2D
    {
      return m_extent;
    }

    auto get_backbuffer_images(VkImage& color, VkImage& depth) const -> void
    {
      color = m_frames[m_current_frame_index].swapchain_image;
      depth = m_frames[m_current_frame_index].depth_image.get_handle();
    }

    auto get_backbuffer_views(VkImageView& color, VkImageView& depth) const -> void
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

    [[nodiscard]] auto get_clear_color() const -> VkClearColorValue
    {
      return {m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]};
    }

    auto set_clear_color(f32 r, f32 g, f32 b, f32 a = 1.0f) -> void
    {
      m_clear_color[0] = r;
      m_clear_color[1] = g;
      m_clear_color[2] = b;
      m_clear_color[3] = a;
    }

private:
    VkSwapchainKHR m_handle{};
    VkExtent2D m_extent{};
    VkFormat m_format{};
    VkColorSpaceKHR m_colorspace{};
    VkExtent2D m_min_extent{};
    VkExtent2D m_max_extent{};
    u32 m_buffer_count{};

    u32 m_current_frame_index{};
    u32 m_current_sync_frame_index{};
    Frame m_frames[NUM_FRAMES_BUFFERED]{};

    f32 m_clear_color[4]{};

    friend class VulkanDevice;
    friend class VulkanBackend;
  };
} // namespace ghi