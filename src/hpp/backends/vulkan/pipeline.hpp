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
#include <backends/vulkan/image.hpp>

namespace ghi
{
  class VulkanDevice;

  struct VulkanShaderModule
  {
    VkShaderModule handle{};
    VkPipelineShaderStageCreateInfo stage_create_info{};

    static auto create(VulkanDevice &device, Span<const u32> spirv_code, VkShaderStageFlagBits stage)
        -> Result<VulkanShaderModule>;
    auto destroy(VulkanDevice &device) -> void;
  };

  struct VulkanBindingLayout
  {
    VkDescriptorSetLayout handle{};
    HashMap<u32, VkDescriptorType> binding_types;

    static auto create(VulkanDevice &device, Span<const BindingLayoutEntry> entries) -> Result<VulkanBindingLayout>;
    auto destroy(VulkanDevice &device) -> void;
  };

  struct VulkanPipelineLayout
  {
    VkPipelineLayout handle{};
    VkShaderStageFlags push_constant_stages{};

    static auto create(VulkanDevice &device, Span<const VulkanBindingLayout *> bindings,
                       Span<const VkPushConstantRange> push_constants) -> Result<VulkanPipelineLayout>;
    auto destroy(VulkanDevice &device) -> void;
  };

  struct VulkanDescriptorTable
  {
    u32 handle_count{0};
    VkDescriptorSet handles[NUM_FRAMES_BUFFERED]{};
    VulkanBindingLayout *layout{nullptr};

    static auto create(VulkanDevice &device, bool is_frame_bound, VulkanBindingLayout *layout)
        -> Result<VulkanDescriptorTable>;
    auto destroy(VulkanDevice &device) -> void;
  };

  class VulkanPipelineBase
  {
public:
    virtual ~VulkanPipelineBase() = default;
    virtual auto begin(VkCommandBuffer cmd) -> void = 0;
    virtual auto end(VkCommandBuffer cmd) -> void = 0;

    virtual auto get_handle() const -> VkPipeline = 0;
    virtual auto get_layout() const -> VkPipelineLayout = 0;
    virtual auto get_push_constant_stages() const -> VkShaderStageFlags = 0;
    virtual auto get_device() const -> VulkanDevice * = 0;
    virtual auto get_bind_point() const -> VkPipelineBindPoint = 0;
  };

  class VulkanGraphicsPipeline : public VulkanPipelineBase
  {
public:
    static auto create(VulkanDevice &device, const GraphicsPipelineDesc &desc) -> Result<VulkanGraphicsPipeline *>;
    auto destroy(VulkanDevice &device) -> void;

    auto begin(VkCommandBuffer cmd) -> void override;
    auto end(VkCommandBuffer cmd) -> void override;

    auto get_handle() const -> VkPipeline override
    {
      return m_handle;
    }

    auto get_layout() const -> VkPipelineLayout override
    {
      return m_layout.handle;
    }

    auto get_push_constant_stages() const -> VkShaderStageFlags override
    {
      return m_layout.push_constant_stages;
    }

    auto get_device() const -> VulkanDevice * override
    {
      return m_device;
    }

    auto get_bind_point() const -> VkPipelineBindPoint override
    {
      return VK_PIPELINE_BIND_POINT_GRAPHICS;
    }

private:
    VulkanDevice *m_device{};
    VkPipeline m_handle{};
    VulkanPipelineLayout m_layout{};
  };

  class VulkanComputePipeline : public VulkanPipelineBase
  {
public:
    static auto create(VulkanDevice &device, const ComputePipelineDesc &desc) -> Result<VulkanComputePipeline *>;
    auto destroy(VulkanDevice &device) -> void;

    auto begin(VkCommandBuffer cmd) -> void override;
    auto end(VkCommandBuffer cmd) -> void override;

    auto get_handle() const -> VkPipeline override
    {
      return m_handle;
    }

    auto get_layout() const -> VkPipelineLayout override
    {
      return m_layout.handle;
    }

    auto get_push_constant_stages() const -> VkShaderStageFlags override
    {
      return m_layout.push_constant_stages;
    }

    auto get_device() const -> VulkanDevice * override
    {
      return m_device;
    }

    auto get_bind_point() const -> VkPipelineBindPoint override
    {
      return VK_PIPELINE_BIND_POINT_COMPUTE;
    }

private:
    VulkanDevice *m_device{};
    VkPipeline m_handle{};
    VulkanPipelineLayout m_layout{};
  };
} // namespace ghi