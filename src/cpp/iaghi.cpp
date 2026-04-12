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

#include <iaghi/iaghi.hpp>

#include <backends/vulkan/backend.hpp>

namespace ghi
{
  auto create_device(const InitInfo &init_info) -> Result<Device>
  {
    return VulkanBackend::create_device(init_info);
  }

  auto destroy_device(Device device) -> void
  {
    VulkanBackend::destroy_device(device);
  }

  auto create_buffers(Device device, Span<const BufferDesc> descs, Buffer *out_handles) -> Result<void>
  {
    return VulkanBackend::create_buffers(device, descs, out_handles);
  }

  auto destroy_buffers(Device device, Span<const Buffer> handles) -> void
  {
    VulkanBackend::destroy_buffers(device, handles);
  }

  auto map_buffer(Device device, Buffer buffer) -> void *
  {
    return VulkanBackend::map_buffer(device, buffer);
  }

  auto unmap_buffer(Device device, Buffer buffer) -> void
  {
    VulkanBackend::unmap_buffer(device, buffer);
  }

  auto create_images(Device device, Span<const ImageDesc> descs, Image *out_handles) -> Result<void>
  {
    return VulkanBackend::create_images(device, descs, out_handles);
  }

  auto destroy_images(Device device, Span<const Image> handles) -> void
  {
    VulkanBackend::destroy_images(device, handles);
  }

  auto upload_image_data(Device device, Span<const Image> handles, Span<const u8 *const> image_data,
                         bool generate_mip_maps) -> Result<void>
  {
    return VulkanBackend::upload_image_data(device, handles, image_data, generate_mip_maps);
  }

  auto create_samplers(Device device, Span<const SamplerDesc> descs, Sampler *out_handles) -> Result<void>
  {
    return VulkanBackend::create_samplers(device, descs, out_handles);
  }

  auto destroy_samplers(Device device, Span<const Sampler> handles) -> void
  {
    VulkanBackend::destroy_samplers(device, handles);
  }

  auto create_binding_layout(Device device, Span<const BindingLayoutEntry> entries) -> Result<BindingLayout>
  {
    return VulkanBackend::create_binding_layout(device, entries);
  }

  auto destroy_binding_layout(Device device, BindingLayout layout) -> void
  {
    VulkanBackend::destroy_binding_layout(device, layout);
  }

  auto create_descriptor_tables(Device device, BindingLayout layout, bool is_frame_bound, u32 count,
                                DescriptorTable *out_tables) -> Result<void>
  {
    return VulkanBackend::create_descriptor_tables(device, layout, is_frame_bound, count, out_tables);
  }

  auto update_descriptor_tables(Device device, Span<const DescriptorUpdate> updates) -> void
  {
    VulkanBackend::update_descriptor_tables(device, updates);
  }

  auto create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage) -> Result<Shader>
  {
    return VulkanBackend::create_shader(device, spirv_code, size, stage);
  }

  auto destroy_shader(Device device, Shader shader) -> void
  {
    VulkanBackend::destroy_shader(device, shader);
  }

  auto create_graphics_pipeline(Device device, const GraphicsPipelineDesc &desc) -> Result<Pipeline>
  {
    return VulkanBackend::create_graphics_pipeline(device, desc);
  }

  auto destroy_pipeline(Device device, Pipeline pipeline) -> void
  {
    VulkanBackend::destroy_pipeline(device, pipeline);
  }

  auto resize_swapchain(Device device, u32 width, u32 height) -> Result<void>
  {
    return VulkanBackend::resize_swapchain(device, width, height);
  }

  auto get_swapchain_format(Device device) -> EFormat
  {
    return VulkanBackend::get_swapchain_format(device);
  }

  auto get_swapchain_extent(Device device, u32 &width, u32 &height) -> void
  {
    VulkanBackend::get_swapchain_extent(device, width, height);
  }

  auto begin_frame(Device device) -> CommandBuffer
  {
    return VulkanBackend::begin_frame(device);
  }

  auto end_frame(Device device) -> void
  {
    VulkanBackend::end_frame(device);
  }

  auto wait_idle(Device device) -> void
  {
    VulkanBackend::wait_idle(device);
  }

  auto set_clear_color(Device device, f32 r, f32 g, f32 b, f32 a) -> void
  {
    VulkanBackend::set_clear_color(device, r, g, b, a);
  }

  auto execute_single_time_commands(Device device, const std::function<void(CommandBuffer)> &commands_callback)
      -> Result<void>
  {
    return VulkanBackend::execute_single_time_commands(device, commands_callback);
  }
} // namespace ghi

namespace ghi
{
  auto cmd_copy_buffer(CommandBuffer cmd, Buffer src, Buffer dst, u64 size, u64 src_offset, u64 dst_offset) -> void
  {
    VulkanBackend::cmd_copy_buffer(cmd, src, dst, size, src_offset, dst_offset);
  }

  auto cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, Span<const Buffer> buffers,
                               Span<const u64> offsets) -> void
  {
    VulkanBackend::cmd_bind_vertex_buffers(cmd, first_binding, buffers, offsets);
  }

  auto cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices) -> void
  {
    VulkanBackend::cmd_bind_index_buffer(cmd, buffer, offset, use_32_bit_indices);
  }

  auto cmd_bind_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void
  {
    VulkanBackend::cmd_bind_pipeline(cmd, pipeline);
  }

  auto cmd_push_constants(CommandBuffer cmd, Pipeline pipeline, u32 offset, u32 size, const void *data) -> void
  {
    VulkanBackend::cmd_push_constants(cmd, pipeline, offset, size, data);
  }

  auto cmd_bind_frame_bound_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table)
      -> void
  {
    VulkanBackend::cmd_bind_frame_bound_descriptor_table(cmd, set_index, pipeline, table);
  }

  auto cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table,
                                 Span<const u32> offsets) -> void
  {
    VulkanBackend::cmd_bind_descriptor_table(cmd, set_index, pipeline, table, offsets);
  }

  auto cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void
  {
    VulkanBackend::cmd_set_viewport(cmd, x, y, w, h);
  }

  auto cmd_set_scissor(CommandBuffer cmd, i32 x, i32 y, i32 w, i32 h) -> void
  {
    VulkanBackend::cmd_set_scissor(cmd, x, y, w, h);
  }

  auto cmd_draw(CommandBuffer cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) -> void
  {
    VulkanBackend::cmd_draw(cmd, vertex_count, instance_count, first_vertex, first_instance);
  }

  auto cmd_draw_indexed(CommandBuffer cmd, u32 index_count, u32 instance_count, u32 first_index, u32 first_vertex,
                        u32 first_instance) -> void
  {
    VulkanBackend::cmd_draw_indexed(cmd, index_count, instance_count, first_index, first_vertex, first_instance);
  }

  auto cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count, u32 stride)
      -> void
  {
    VulkanBackend::cmd_draw_indexed_indirect(cmd, indirect_buffer, offset, draw_count, stride);
  }

  auto cmd_pipeline_barrier(CommandBuffer cmd, Span<const BufferBarrier> buffer_barriers,
                            Span<const ImageBarrier> image_barriers) -> void
  {
    VulkanBackend::cmd_pipeline_barrier(cmd, buffer_barriers, image_barriers);
  }
} // namespace ghi

namespace ghi
{
  auto is_depth_format(EFormat format) -> bool
  {
    return (format == EFormat::D16Unorm || format == EFormat::D16UnormS8Uint || format == EFormat::D24UnormS8Uint ||
            format == EFormat::D32Sfloat || format == EFormat::D32SfloatS8Uint);
  }

  auto is_compressed_format(EFormat format) -> bool
  {
    return format >= EFormat::Bc1RgbUnormBlock;
  }

  auto get_format_byte_size(EFormat format) -> u32
  {
    switch (format)
    {
    case EFormat::R8G8B8A8Unorm:
    case EFormat::R8G8B8A8Srgb:
    case EFormat::B8G8R8A8Srgb:
    case EFormat::B8G8R8A8Unorm:
      return 4;

    case EFormat::R32Uint:
    case EFormat::R32Float:
      return 4;
    case EFormat::R32G32Float:
      return 8;
    case EFormat::R32G32B32Float:
      return 12;
    case EFormat::R32G32B32A32Float:
      return 16;

    case EFormat::D16Unorm:
      return 2;
    case EFormat::D16UnormS8Uint:
      return 3;
    case EFormat::D24UnormS8Uint:
      return 4;
    case EFormat::D32Sfloat:
      return 4;
    case EFormat::D32SfloatS8Uint:
      return 5;

    case EFormat::Bc1RgbUnormBlock:
    case EFormat::Bc1RgbSrgbBlock:
    case EFormat::Bc1RgbaUnormBlock:
    case EFormat::Bc1RgbaSrgbBlock:
    case EFormat::Bc2UnormBlock:
    case EFormat::Bc2SrgbBlock:
    case EFormat::Bc3UnormBlock:
    case EFormat::Bc3SrgbBlock:
    case EFormat::Bc5UnormBlock:
    case EFormat::Bc5SnormBlock:
      return 0;

    case EFormat::Undefined:
    default:
      return 0;
    }
  }

  auto get_compressed_format_block_size(EFormat format) -> u32
  {
    if (!is_compressed_format(format))
      return 0;

    switch (format)
    {
    case EFormat::Bc1RgbUnormBlock:
    case EFormat::Bc1RgbaUnormBlock:
    case EFormat::Bc1RgbSrgbBlock:
    case EFormat::Bc1RgbaSrgbBlock:
      return 8;

    default:
      return 16;
    }
  }
} // namespace ghi