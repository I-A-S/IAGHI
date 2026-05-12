// IAGHI: IA Graphics Hardware Interface
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
// Copyright (C) 2026 IASoft PVT LTD (contact@iasoft.dev)
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
#include <backends/vulkan/buffer.hpp>

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

    Vec<VkDescriptorBindingFlags> binding_flags;
    binding_flags.reserve(entries.size());
    bool has_bindless = false;

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

      VkDescriptorBindingFlags flags = 0;
      if (entry.is_bindless)
      {
        flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        has_bindless = true;
      }
      binding_flags.push_back(flags);

      result.binding_types[entry.binding] = b.descriptorType;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_info{};
    flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flags_info.bindingCount = static_cast<u32>(binding_flags.size());
    flags_info.pBindingFlags = binding_flags.data();

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<u32>(bindings.size());
    layout_info.pBindings = bindings.data();
    if (has_bindless)
    {
      layout_info.pNext = &flags_info;
      layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    }

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

  auto VulkanPipelineLayout::create(VulkanDevice &device, Span<const VulkanBindingLayout *> bindings,
                                    Span<const VkPushConstantRange> push_constants) -> Result<VulkanPipelineLayout>
  {
    VulkanPipelineLayout result{};

    Vec<VkDescriptorSetLayout> descriptor_set_layouts;
    for (const auto &binding : bindings)
    {
      descriptor_set_layouts.push_back(binding->handle);
    }

    const VkPipelineLayoutCreateInfo layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<u32>(descriptor_set_layouts.size()),
        .pSetLayouts = descriptor_set_layouts.data(),
        .pushConstantRangeCount = static_cast<u32>(push_constants.size()),
        .pPushConstantRanges = push_constants.data(),
    };

    VkShaderStageFlags pc_stages = 0;
    for (const auto& pc : push_constants)
    {
      pc_stages |= pc.stageFlags;
    }
    result.push_constant_stages = pc_stages;

    VK_CALL(vkCreatePipelineLayout(device.get_handle(), &layout_create_info, nullptr, &result.handle),
            "Creating pipeline layout");

    return result;
  }

  auto VulkanPipelineLayout::destroy(VulkanDevice &device) -> void
  {
    vkDestroyPipelineLayout(device.get_handle(), handle, nullptr);
  }

  auto VulkanDescriptorTable::create(VulkanDevice &device, bool is_frame_bound, VulkanBindingLayout *layout)
      -> Result<VulkanDescriptorTable>
  {
    VulkanDescriptorTable result;

    result.handle_count = is_frame_bound ? device.get_swapchain().get_backbuffer_image_count() : 1;

    VkDescriptorSetLayout layouts[NUM_FRAMES_BUFFERED] = {};
    for (u32 i = 0; i < NUM_FRAMES_BUFFERED; ++i)
      layouts[i] = layout->handle;

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = device.get_descriptor_pool();
    alloc_info.descriptorSetCount = result.handle_count;
    alloc_info.pSetLayouts = layouts;

    VK_CALL(vkAllocateDescriptorSets(device.get_handle(), &alloc_info, result.handles), "allocating descriptor set");

    result.layout = layout;

    return result;
  }

  auto VulkanDescriptorTable::destroy(VulkanDevice &device) -> void
  {
    vkFreeDescriptorSets(device.get_handle(), device.get_descriptor_pool(), handle_count, handles);
  }

  auto VulkanGraphicsPipeline::create(VulkanDevice &device, const GraphicsPipelineDesc &desc)
      -> Result<VulkanGraphicsPipeline*>
  {
    auto result = new VulkanGraphicsPipeline();

    result->m_device = &device;

    Vec<VkPushConstantRange> push_constants;
    for (const auto &pc : desc.push_constant_ranges)
      push_constants.push_back({
          .stageFlags = VulkanBackend::map_shader_stage_enum_to_vk(pc.stages),
          .offset = pc.offset,
          .size = pc.size,
      });

    Vec<const VulkanBindingLayout *> bindings_layouts;
    for (u32 i = 0; i < desc.binding_layouts.size(); ++i)
    {
      bindings_layouts.push_back(reinterpret_cast<VulkanBindingLayout *>(desc.binding_layouts[i]));
    }
    AU_TRY_VAR(layout, VulkanPipelineLayout::create(device, bindings_layouts, push_constants));
    result->m_layout=std::move(layout);

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

    auto vertex_bindings = map_vertex_bindings_vk(desc.vertex_bindings.data(), desc.vertex_bindings.size());
    auto vertex_attributes = map_vertex_attributes_vk(desc.vertex_attributes.data(), desc.vertex_attributes.size());

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

    Vec<VkFormat> color_attachment_formats;
    VkFormat depth_attachment_format = VK_FORMAT_UNDEFINED;

    if (desc.color_target_formats.empty())
    {
      color_attachment_formats.push_back(device.get_swapchain().get_color_format());
      depth_attachment_format =
          desc.enable_depth_test ? device.get_swapchain().get_depth_format() : VK_FORMAT_UNDEFINED;
    }
    else
    {
      for (auto format : desc.color_target_formats)
      {
        color_attachment_formats.push_back(VulkanBackend::map_format_enum_to_vk(format));
      }
      depth_attachment_format = desc.enable_depth_test
                                    ? VulkanBackend::map_format_enum_to_vk(desc.depth_target_format)
                                    : VK_FORMAT_UNDEFINED;
    }

    VkPipelineRenderingCreateInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_info.colorAttachmentCount = color_attachment_formats.size();
    rendering_info.pColorAttachmentFormats = color_attachment_formats.data();
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
        .stageCount = static_cast<u32>(std::size(shader_stages)),
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = result->m_layout.handle,
    };

    VK_CALL(vkCreateGraphicsPipelines(device.get_handle(), nullptr, 1, &create_info, nullptr, &result->m_handle),
            "Creating graphics pipeline");

    return result;
  }

  auto VulkanGraphicsPipeline::destroy(VulkanDevice &device) -> void
  {
    vkDestroyPipeline(device.get_handle(), m_handle, nullptr);
    m_layout.destroy(device);
  }

  auto VulkanGraphicsPipeline::begin(VkCommandBuffer cmd) -> void
  {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_handle);
  }

  auto VulkanGraphicsPipeline::end(VkCommandBuffer cmd) -> void
  {
  }

  auto VulkanComputePipeline::create(VulkanDevice &device, const ComputePipelineDesc &desc)
      -> Result<VulkanComputePipeline*>
  {
    auto result = new VulkanComputePipeline();

    result->m_device = &device;

    Vec<VkPushConstantRange> push_constants;
    for (const auto &pc : desc.push_constant_ranges)
      push_constants.push_back({
          .stageFlags = VulkanBackend::map_shader_stage_enum_to_vk(pc.stages),
          .offset = pc.offset,
          .size = pc.size,
      });

    Vec<const VulkanBindingLayout *> bindings_layouts;
    for (u32 i = 0; i < desc.binding_layouts.size(); ++i)
    {
      bindings_layouts.push_back(reinterpret_cast<VulkanBindingLayout *>(desc.binding_layouts[i]));
    }
    AU_TRY_VAR(layout, VulkanPipelineLayout::create(device, bindings_layouts, push_constants));
    result->m_layout=std::move(layout);

    const auto compute_shader_module = reinterpret_cast<const VulkanShaderModule *>(desc.compute_shader);

    VkComputePipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = compute_shader_module->stage_create_info,
        .layout = result->m_layout.handle,
    };

    VK_CALL(vkCreateComputePipelines(device.get_handle(), nullptr, 1, &create_info, nullptr, &result->m_handle),
            "Creating compute pipeline");

    return result;
  }

  auto VulkanComputePipeline::destroy(VulkanDevice &device) -> void
  {
    vkDestroyPipeline(device.get_handle(), m_handle, nullptr);
    m_layout.destroy(device);
  }

  auto VulkanComputePipeline::begin(VkCommandBuffer cmd) -> void
  {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_handle);
  }

  auto VulkanComputePipeline::end(VkCommandBuffer cmd) -> void
  {
  }

  auto VulkanBackend::create_binding_layouts(Device device, Span<const Span<const BindingLayoutEntry>> entry_sets,
                                             Span<BindingLayout *const> out_layouts) -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    u32 i{0};
    for (const auto &entries : entry_sets)
    {
      auto layout_res = VulkanBindingLayout::create(*dev, entries);
      if (!layout_res.has_value())
      {
        for (u32 j = 0; j < i; ++j)
        {
          const auto layout_impl = reinterpret_cast<VulkanBindingLayout *>(*out_layouts[j]);
          layout_impl->destroy(*dev);
          delete layout_impl;
          *out_layouts[j] = nullptr;
        }
        return layout_res.error();
      }
      *out_layouts[i++] = reinterpret_cast<BindingLayout>(new VulkanBindingLayout(std::move(layout_res.value())));
    }

    return {};
  }

  auto VulkanBackend::destroy_binding_layouts(Device device, Span<const BindingLayout> layouts) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (auto layout : layouts)
    {
      const auto layout_impl = reinterpret_cast<VulkanBindingLayout *>(layout);
      layout_impl->destroy(*dev);
      delete layout_impl;
    }
  }

  auto VulkanBackend::create_descriptor_tables(Device device, bool is_frame_bound, BindingLayout layout,
                                               Span<DescriptorTable *const> out_tables) -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < out_tables.size(); i++)
    {
      const auto layout_impl = reinterpret_cast<VulkanBindingLayout *>(layout);
      auto table_res = VulkanDescriptorTable::create(*dev, is_frame_bound, layout_impl);
      if (!table_res.has_value())
      {
        for (u32 j = 0; j < i; ++j)
        {
          const auto table_impl = reinterpret_cast<VulkanDescriptorTable *>(*out_tables[j]);
          table_impl->destroy(*dev);
          delete table_impl;
          *out_tables[j] = nullptr;
        }
        return table_res.error();
      }
      *out_tables[i] = reinterpret_cast<DescriptorTable>(new VulkanDescriptorTable(std::move(table_res.value())));
    }

    return {};
  }

  auto VulkanBackend::update_descriptor_tables(Device device, Span<const DescriptorUpdate> updates) -> void
  {
    auto &logger = auxid::get_thread_logger();

    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (const auto &update : updates)
    {
      const auto table = reinterpret_cast<VulkanDescriptorTable *>(update.table);

      const auto target_type = table->layout->binding_types.find(update.binding);
      if (!target_type)
      {
        logger.error("UpdateDescriptorTables: Binding %d not found in layout", update.binding);
        continue;
      }

      VkWriteDescriptorSet write{};
      write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write.dstBinding = update.binding;
      write.dstArrayElement = update.array_element;
      write.descriptorCount = 1;
      write.descriptorType = *target_type;

      VkDescriptorBufferInfo buffer_info{};
      VkDescriptorImageInfo image_info{};

      if (update.buffer)
      {
        write.pBufferInfo = &buffer_info;

        const auto buffer_impl = reinterpret_cast<VulkanBuffer *>(update.buffer);

        assert(!buffer_impl->is_frame_bound() || (update.buffer_offset == 0 && update.buffer_range == 0));

        buffer_info.buffer = buffer_impl->get_handle();

        if (buffer_impl->is_frame_bound())
        {
          assert(table->handle_count == buffer_impl->get_unit_count());

          for (u32 f = 0; f < table->handle_count; f++)
          {
            buffer_info.offset = buffer_impl->get_stride() * f;
            buffer_info.range = buffer_impl->get_unit_size();
            write.dstSet = table->handles[f];
            vkUpdateDescriptorSets(dev->get_handle(), 1, &write, 0, nullptr);
          }

          continue;
        }

        buffer_info.offset = update.buffer_offset;
        buffer_info.range = (update.buffer_range == 0) ? VK_WHOLE_SIZE : update.buffer_range;
      }
      else if (update.image)
      {
        write.pImageInfo = &image_info;

        const auto image_impl = reinterpret_cast<VulkanImage *>(update.image);

        VkSampler sampler = VK_NULL_HANDLE;
        if (update.sampler)
          sampler = reinterpret_cast<VkSampler>(update.sampler);

        image_info.imageView = image_impl->get_view();
        image_info.sampler = sampler;

        if (*target_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
            *target_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
        {
          image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else if (*target_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        {
          image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
      }

      write.dstSet = table->handles[0];
      vkUpdateDescriptorSets(dev->get_handle(), 1, &write, 0, nullptr);
    }
  }

  auto VulkanBackend::create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage)
      -> Result<Shader>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    AU_TRY_VAR(shader, VulkanShaderModule::create(*dev, Span<const u32>(static_cast<const u32 *>(spirv_code), size >> 2),
                                          static_cast<VkShaderStageFlagBits>(map_shader_stage_enum_to_vk(stage))));
    return reinterpret_cast<Shader>(new VulkanShaderModule(std::move(shader)));
  }

  auto VulkanBackend::destroy_shaders(Device device, Span<const Shader> shaders) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    for (const auto &shader : shaders)
    {
      const auto shader_impl = reinterpret_cast<VulkanShaderModule *>(shader);
      shader_impl->destroy(*dev);
      delete shader_impl;
    }
  }

  auto VulkanBackend::create_graphics_pipeline(Device device, const GraphicsPipelineDesc &desc) -> Result<Pipeline>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    AU_TRY_VAR(pipeline, VulkanGraphicsPipeline::create(*dev, desc));
    return reinterpret_cast<Pipeline>(pipeline);
  }

  auto VulkanBackend::create_compute_pipeline(Device device, const ComputePipelineDesc &desc) -> Result<Pipeline>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    AU_TRY_VAR(pipeline, VulkanComputePipeline::create(*dev, desc));
    return reinterpret_cast<Pipeline>(pipeline);
  }

  auto VulkanBackend::destroy_pipeline(Device device, Pipeline pipeline) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto pipeline_impl = reinterpret_cast<VulkanPipelineBase *>(pipeline);
    if (pipeline_impl->get_bind_point() == VK_PIPELINE_BIND_POINT_GRAPHICS)
    {
      const auto gfx = static_cast<VulkanGraphicsPipeline*>(pipeline_impl);
      gfx->destroy(*dev);
      delete gfx;
    }
    else if (pipeline_impl->get_bind_point() == VK_PIPELINE_BIND_POINT_COMPUTE)
    {
      const auto comp = static_cast<VulkanComputePipeline*>(pipeline_impl);
      comp->destroy(*dev);
      delete comp;
    }
  }
} // namespace ghi