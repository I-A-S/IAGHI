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

#include <backends/vulkan/pipeline.hpp>
#include <backends/vulkan/backend.hpp>
#include <backends/vulkan/device.hpp>

namespace ghi
{
  auto map_vertex_bindings_vk(const VertexInputBinding *bindings, u32 count) -> Vec<VkVertexInputBindingDescription>
  {
    if (!bindings || count == 0)
      return Vec<VkVertexInputBindingDescription>();

    Vec<VkVertexInputBindingDescription> result(count);
    for (u32 i = 0; i < count; ++i)
    {
      result[i].binding = bindings[i].binding;
      result[i].stride = bindings[i].stride;
      result[i].inputRate = VulkanBackend::map_input_rate_enum_to_vk(bindings[i].input_rate);
    }
    return result;
  }

  auto map_vertex_attributes_vk(const VertexInputAttribute *attributes, u32 count)
      -> Vec<VkVertexInputAttributeDescription>
  {
    if (!attributes || count == 0)
      return Vec<VkVertexInputAttributeDescription>();

    Vec<VkVertexInputAttributeDescription> result(count);
    for (u32 i = 0; i < count; ++i)
    {
      result[i].location = attributes[i].location;
      result[i].binding = attributes[i].binding;
      result[i].format = VulkanBackend::map_format_enum_to_vk(attributes[i].format);
      result[i].offset = attributes[i].offset;
    }
    return result;
  }

  auto VulkanShaderModule::create(VulkanDevice &device, Span<const u32> spirv_code, VkShaderStageFlagBits stage)
      -> Result<VulkanShaderModule>
  {
    VulkanShaderModule result{};

    VkShaderModuleCreateInfo module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv_code.size() * sizeof(u32),
        .pCode = spirv_code.data(),
    };

    VK_CALL(vkCreateShaderModule(device.get_handle(), &module_create_info, nullptr, &result.handle),
            "Creating shader module from SPIRV");

    result.stage_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = result.handle,
        .pName = "main",
    };

    return result;
  }

  auto VulkanShaderModule::destroy(VulkanDevice &device) -> void
  {
    vkDestroyShaderModule(device.get_handle(), handle, nullptr);
  }

  auto VulkanBindingLayout::create(VulkanDevice &device, Span<const BindingLayoutEntry> entries)
      -> Result<VulkanBindingLayout>
  {
    Vec<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(entries.size());

    VulkanBindingLayout result;

    for (const auto &entry : entries)
    {
      VkDescriptorSetLayoutBinding b{};
      b.binding = entry.binding;
      b.descriptorType = VulkanBackend::map_descriptor_type_enum_to_vk(entry.type);
      b.descriptorCount = entry.count;
      b.stageFlags = VulkanBackend::map_shader_stage_enum_to_vk(entry.visibility);
      b.pImmutableSamplers = nullptr; // [IATODO] Support immutable samplers
      bindings.push_back(b);

      result.binding_types[entry.binding] = b.descriptorType;
    }

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<u32>(bindings.size());
    layout_info.pBindings = bindings.data();

    VkDescriptorSetLayout layout_handle;
    VK_CALL(vkCreateDescriptorSetLayout(device.get_handle(), &layout_info, nullptr, &layout_handle),
            "creating descriptor set layout");

    result.handle = layout_handle;

    return result;
  }

  auto VulkanBindingLayout::destroy(VulkanDevice &device) -> void
  {
    vkDestroyDescriptorSetLayout(device.get_handle(), handle, nullptr);
  }

  auto VulkanPipelineLayout::create(VulkanDevice &device, Span<const VulkanBindingLayout *> bindings)
      -> Result<VulkanPipelineLayout>
  {
    VulkanPipelineLayout result{};

    Vec<VkDescriptorSetLayout> descriptor_set_layouts;
    for (const auto &binding : bindings)
    {
      descriptor_set_layouts.push_back(binding->handle);
    }

    VkPipelineLayoutCreateInfo layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<u32>(descriptor_set_layouts.size()),
        .pSetLayouts = descriptor_set_layouts.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    VK_CALL(vkCreatePipelineLayout(device.get_handle(), &layout_create_info, nullptr, &result.handle),
            "Creating pipeline layout");

    return result;
  }

  auto VulkanPipelineLayout::destroy(VulkanDevice &device) -> void
  {
    vkDestroyPipelineLayout(device.get_handle(), handle, nullptr);
  }

  auto VulkanDescriptorTable::create(VulkanDevice &device, VulkanBindingLayout *layout) -> Result<VulkanDescriptorTable>
  {
    VulkanDescriptorTable result;

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = device.get_descriptor_pool();
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &layout->handle;

    VK_CALL(vkAllocateDescriptorSets(device.get_handle(), &alloc_info, &result.handle), "allocating descriptor set");

    result.layout = layout;

    return result;
  }

  auto VulkanDescriptorTable::destroy(VulkanDevice &device) -> void
  {
    vkFreeDescriptorSets(device.get_handle(), device.get_descriptor_pool(), 1, &handle);
  }

  auto VulkanGraphicsPipeline::create(VulkanDevice &device, const GraphicsPipelineDesc &desc)
      -> Result<VulkanGraphicsPipeline>
  {
    VulkanGraphicsPipeline result{};

    Vec<const VulkanBindingLayout *> bindings_layouts;
    for (u32 i = 0; i < desc.binding_layout_count; ++i)
    {
      bindings_layouts.push_back(reinterpret_cast<VulkanBindingLayout *>(desc.binding_layouts[i]));
    }
    result.m_layout = AU_TRY(VulkanPipelineLayout::create(device, bindings_layouts));

    const auto vertex_shader_module = reinterpret_cast<const VulkanShaderModule *>(desc.vertex_shader);
    const auto fragment_shader_module = reinterpret_cast<const VulkanShaderModule *>(desc.fragment_shader);

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vertex_shader_module->stage_create_info,
        fragment_shader_module->stage_create_info,
    };

    Vec<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<u32>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    auto vertex_bindings = map_vertex_bindings_vk(desc.vertex_bindings, desc.vertex_binding_count);
    auto vertex_attributes = map_vertex_attributes_vk(desc.vertex_attributes, desc.vertex_attribute_count);

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = static_cast<u32>(vertex_bindings.size());
    vertex_input_info.pVertexBindingDescriptions = vertex_bindings.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<u32>(vertex_attributes.size());
    vertex_input_info.pVertexAttributeDescriptions = vertex_attributes.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    input_assembly.topology = VulkanBackend::map_primitive_type_to_vk(desc.primitive_type);

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VulkanBackend::map_polygon_mode_to_vk(desc.polygon_mode);
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VulkanBackend::map_cull_mode_to_vk(desc.cull_mode);
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    const auto color_blend_attachment = VulkanBackend::map_blend_mode_to_vk(desc.blend_mode);
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    Vec<VkFormat> color_attachments;
    for (u32 i = 0; i < desc.color_attachment_count; ++i)
      color_attachments.push_back(VulkanBackend::map_format_enum_to_vk(desc.color_formats[i]));

    VkFormat depth_attachment_format = VulkanBackend::map_format_enum_to_vk(desc.depth_format);

    VkPipelineRenderingCreateInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_info.colorAttachmentCount = color_attachments.size();
    rendering_info.pColorAttachmentFormats = color_attachments.data();
    rendering_info.depthAttachmentFormat = depth_attachment_format;

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering_info,
        .stageCount = _countof(shader_stages),
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = result.m_layout.handle,
    };

    VK_CALL(vkCreateGraphicsPipelines(device.get_handle(), nullptr, 1, &create_info, nullptr, &result.m_handle),
            "Creating graphics pipeline");

    return result;
  }

  auto VulkanGraphicsPipeline::destroy(VulkanDevice &device) -> void
  {
    vkDestroyPipeline(device.get_handle(), m_handle, nullptr);
    m_layout.destroy(device);
  }
} // namespace ghi