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
  class VulkanDevice;

  struct VulkanShaderModule
  {
    VkShaderModule handle{};
    VkPipelineShaderStageCreateInfo stage_create_info{};

    static auto create(VulkanDevice& device, Span<const u32> spirv_code, VkShaderStageFlagBits stage)
        -> Result<VulkanShaderModule>;
    auto destroy(VulkanDevice& device) -> void;
  };

  struct VulkanBindingLayout
  {
    VkDescriptorSetLayout handle{};
    HashMap<u32, VkDescriptorType> binding_types;

    static auto create(VulkanDevice& device, Span<const BindingLayoutEntry> entries)
        -> Result<VulkanBindingLayout>;
    auto destroy(VulkanDevice& device) -> void;
  };

  struct VulkanPipelineLayout
  {
    VkPipelineLayout handle{};

    static auto create(VulkanDevice& device, Span<const VulkanBindingLayout*> bindings)
        -> Result<VulkanPipelineLayout>;
    auto destroy(VulkanDevice& device) -> void;
  };

  struct VulkanDescriptorTable
  {
    VkDescriptorSet handle{VK_NULL_HANDLE};
    VulkanBindingLayout *layout{nullptr};

    static auto create(VulkanDevice& device, VulkanBindingLayout* layout) -> Result<VulkanDescriptorTable>;
    auto destroy(VulkanDevice& device) -> void;
  };

  class VulkanGraphicsPipeline
  {
  public:
    static auto create(VulkanDevice& device, const GraphicsPipelineDesc& desc) -> Result<VulkanGraphicsPipeline>;
    auto destroy(VulkanDevice& device) -> void;

    auto get_handle() const -> VkPipeline
    {
      return m_handle;
    }

    auto get_layout() const -> VkPipelineLayout
    {
      return m_layout.handle;
    }

  private:
    VkPipeline m_handle{};
    VulkanPipelineLayout m_layout{};
  };
}