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

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <volk.h>
#include <vk_mem_alloc.h>

#include <backends/backend.hpp>

#define VK_CALL(call, description)                                                                                     \
  {                                                                                                                    \
    const auto r = call;                                                                                               \
    if (r != VK_SUCCESS)                                                                                               \
      return fail("'%s' failed with code %llu", description, (u64) r);                                                 \
  }

#define VK_ENUM_CALL(call, result, ...)                                                                                \
  {                                                                                                                    \
    UINT32 __count;                                                                                                    \
    call(__VA_ARGS__, &__count, nullptr);                                                                              \
    result.resize(__count);                                                                                            \
    call(__VA_ARGS__, &__count, result.data());                                                                        \
  }

namespace ghi
{
  static constexpr u32 VULKAN_API_VERSION = VK_MAKE_VERSION(1, 3, 0);

  class VulkanBackend
  {
public:
    auto create_device(const InitInfo &init_info) -> Result<Device>;
    auto destroy_device(Device device) -> void;

    auto create_buffers(Device device, u32 count, const BufferDesc *descs, Buffer *out_handles) -> Result<void>;
    auto destroy_buffers(Device device, u32 count, const Buffer *handles) -> void;
    auto map_buffer(Device device, Buffer buffer) -> void *;
    auto unmap_buffer(Device device, Buffer buffer) -> void;

    auto create_images(Device device, u32 count, const ImageDesc *descs, Image *out_handles) -> Result<void>;
    auto destroy_images(Device device, u32 count, Image *handles) -> void;
    auto upload_image_data(Device device, u32 count, Image *handles, const u8 **image_data, bool generate_mip_maps)
        -> Result<void>;

    auto create_samplers(Device device, u32 count, const SamplerDesc *descs, Sampler *out_handles) -> Result<void>;
    auto destroy_samplers(Device device, u32 count, Sampler *handles) -> void;

    auto create_binding_layout(Device device, Span<const BindingLayoutEntry> entries) -> Result<BindingLayout>;
    auto destroy_binding_layout(Device device, BindingLayout layout) -> void;

    auto create_descriptor_tables(Device device, BindingLayout layout, u32 count, DescriptorTable *out_tables)
        -> Result<void>;
    auto update_descriptor_tables(Device device, u32 count, const DescriptorUpdate *updates) -> void;

    auto create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage) -> Result<Shader>;
    auto destroy_shader(Device device, Shader shader) -> void;

    auto create_graphics_pipeline(Device device, const GraphicsPipelineDesc *desc) -> Result<Pipeline>;
    auto destroy_pipeline(Device device, Pipeline pipeline) -> void;

    auto get_swapchain_format(Device device) -> EFormat;

    auto begin_frame(Device device) -> CommandBuffer;
    auto end_frame(Device device) -> void;

    auto wait_idle(Device device) -> void;
    auto set_clear_color(f32 r, f32 g, f32 b, f32 a) -> void;

    auto execute_single_time_commands(Device device, const std::function<void(CommandBuffer)> &commands_callback)
        -> Result<void>;

    auto cmd_copy_buffer(CommandBuffer cmd, Buffer src, Buffer dst, u64 size, u64 src_offset = 0, u64 dst_offset = 0) -> void;

    auto cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, u32 count, const Buffer *buffers,
                                 const u64 *offsets) -> void;
    auto cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices) -> void;

    auto cmd_bind_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void;

    auto cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table) -> void;

    auto cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void;
    auto cmd_set_scissor(CommandBuffer cmd, INT32 x, INT32 y, INT32 w, INT32 h) -> void;

    auto cmd_draw(CommandBuffer cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
        -> void;
    auto cmd_draw_indexed(CommandBuffer cmd, u32 index_count, u32 instance_count, u32 first_index, u32 first_vertex,
                          u32 first_instance) -> void;

    auto cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count, u32 stride)
        -> void;

    auto cmd_pipeline_barrier(CommandBuffer cmd, u32 buffer_barrier_count, const BufferBarrier *buffer_barriers,
                              u32 texture_barrier_count, const ImageBarrier *texture_barriers) -> void;

public:
    static auto is_vk_depth_format(VkFormat format) -> bool;
    static auto map_format_enum_to_vk(EFormat format) -> VkFormat;
    static auto map_vk_to_format_enum(VkFormat format) -> EFormat;
    static auto map_shader_stage_enum_to_vk(EShaderStage stage) -> VkShaderStageFlags;
    static auto map_buffer_usage_enum_to_vk(u32 usage_flags) -> VkBufferUsageFlags;
    static auto map_descriptor_type_enum_to_vk(EDescriptorType type) -> VkDescriptorType;
    static auto map_input_rate_enum_to_vk(EInputRate rate) -> VkVertexInputRate;
    static auto map_polygon_mode_to_vk(EPolygonMode mode) -> VkPolygonMode;
    static auto map_cull_mode_to_vk(ECullMode mode) -> VkCullModeFlags;
    static auto map_primitive_type_to_vk(EPrimitiveType type) -> VkPrimitiveTopology;
    static auto map_blend_mode_to_vk(EBlendMode mode) -> VkPipelineColorBlendAttachmentState;

private:
    f32 m_clear_color[4]{};
  };

  static_assert(IAGHI_BACKEND<VulkanBackend>, "Each backend must satisfy the IAGHI_BACKEND concept");
} // namespace ghi