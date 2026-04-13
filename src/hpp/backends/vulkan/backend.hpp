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
    static auto create_device(const InitInfo &init_info) -> Result<Device>;
    static auto destroy_device(Device device) -> void;

    static auto create_buffers(Device device, Span<const BufferDesc> descs, Buffer *out_handles) -> Result<void>;
    static auto destroy_buffers(Device device, Span<const Buffer> handles) -> void;
    static auto map_frame_bound_buffer(Device device, Buffer buffer) -> void *;
    static auto map_buffer(Device device, Buffer buffer) -> void *;
    static auto unmap_buffer(Device device, Buffer buffer) -> void;

    static auto create_images(Device device, Span<const ImageDesc> descs, Image *out_handles) -> Result<void>;
    static auto destroy_images(Device device, Span<const Image> handles) -> void;
    static auto upload_image_data(Device device, Span<const Image> handles, Span<const u8 *const> image_data,
                                  bool generate_mip_maps) -> Result<void>;

    static auto create_samplers(Device device, Span<const SamplerDesc> descs, Sampler *out_handles) -> Result<void>;
    static auto destroy_samplers(Device device, Span<const Sampler> handles) -> void;

    static auto create_binding_layouts(Device device, Span<const Span<const BindingLayoutEntry>> entry_sets,
                                       BindingLayout *out_layouts) -> Result<void>;
    static auto destroy_binding_layouts(Device device, Span<const BindingLayout> layouts) -> void;

    static auto create_descriptor_tables(Device device, bool is_frame_bound, BindingLayout layout, u32 count,
                                         DescriptorTable *out_tables) -> Result<void>;
    static auto update_descriptor_tables(Device device, Span<const DescriptorUpdate> updates) -> void;

    static auto create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage) -> Result<Shader>;
    static auto destroy_shader(Device device, Shader shader) -> void;

    static auto create_graphics_pipeline(Device device, const GraphicsPipelineDesc &desc) -> Result<Pipeline>;
    static auto destroy_pipeline(Device device, Pipeline pipeline) -> void;

    static auto resize_swapchain(Device device, u32 width, u32 height) -> Result<void>;
    static auto get_swapchain_format(Device device) -> EFormat;
    static auto get_swapchain_extent(Device device, u32 &width, u32 &height) -> void;
    static auto get_swapchain_image_count(Device device) -> u32;

    static auto begin_frame(Device device) -> CommandBuffer;
    static auto end_frame(Device device) -> void;
    static auto get_active_frame_index(Device device) -> u32;

    static auto wait_idle(Device device) -> void;
    static auto set_clear_color(Device device, f32 r, f32 g, f32 b, f32 a = 1.0f) -> void;

    static auto execute_single_time_commands(Device device, const std::function<void(CommandBuffer)> &commands_callback)
        -> Result<void>;

    static auto cmd_copy_buffer(CommandBuffer cmd, Buffer src, Buffer dst, u64 size, u64 src_offset = 0,
                                u64 dst_offset = 0) -> void;

    static auto cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, Span<const Buffer> buffers,
                                        Span<const u64> offsets) -> void;
    static auto cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices) -> void;

    static auto cmd_bind_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void;

    static auto cmd_push_constants(CommandBuffer cmd, Pipeline pipeline, u32 offset, u32 size, const void *data)
        -> void;

    static auto cmd_bind_frame_bound_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline,
                                                      DescriptorTable table) -> void;
    static auto cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table,
                                          Span<const u32> offsets) -> void;

    static auto cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void;
    static auto cmd_set_scissor(CommandBuffer cmd, i32 x, i32 y, i32 w, i32 h) -> void;

    static auto cmd_draw(CommandBuffer cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
        -> void;
    static auto cmd_draw_indexed(CommandBuffer cmd, u32 index_count, u32 instance_count, u32 first_index,
                                 u32 first_vertex, u32 first_instance) -> void;
    static auto cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count,
                                          u32 stride) -> void;

    static auto cmd_pipeline_barrier(CommandBuffer cmd, Span<const BufferBarrier> buffer_barriers,
                                     Span<const ImageBarrier> image_barriers) -> void;

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
  };

  static_assert(IAGHI_BACKEND<VulkanBackend>, "Each backend must satisfy the IAGHI_BACKEND concept");
} // namespace ghi