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
  VulkanBackend backend{};

  auto create_device(const InitInfo &init_info) -> Result<Device>
  {
    return backend.create_device(init_info);
  }

  auto destroy_device(Device device) -> void
  {
    backend.destroy_device(device);
  }

  auto begin_frame(Device device) -> CommandBuffer
  {
    return backend.begin_frame(device);
  }

  auto end_frame(Device device) -> void
  {
    backend.end_frame(device);
  }
  
  auto set_clear_color(f32 r, f32 g, f32 b, f32 a) -> void
  {
    backend.set_clear_color(r, g, b, a);
  }

  auto wait_idle(Device device) -> void
  {
    backend.wait_idle(device);
  }

  auto create_buffers(Device device, u32 count, const BufferDesc *descs, Buffer *out_handles) -> Result<void>
  {
    return backend.create_buffers(device, count, descs, out_handles);
  }

  auto destroy_buffers(Device device, u32 count, const Buffer *handles) -> void
  {
    backend.destroy_buffers(device, count, handles);
  }

  auto map_buffer(Device device, Buffer buffer) -> void *
  {
    return backend.map_buffer(device, buffer);
  }

  auto unmap_buffer(Device device, Buffer buffer) -> void
  {
    backend.unmap_buffer(device, buffer);
  }

  auto create_images(Device device, u32 count, const ImageDesc *descs, Image *out_handles) -> Result<void>
  {
    return backend.create_images(device, count, descs, out_handles);
  }

  auto destroy_images(Device device, u32 count, Image *handles) -> void
  {
    backend.destroy_images(device, count, handles);
  }

  auto upload_image_data(Device device, u32 count, Image *handles, const u8 **image_data,
                                 bool generate_mip_maps) -> Result<void>
  {
    return backend.upload_image_data(device, count, handles, image_data, generate_mip_maps);
  }

  auto create_samplers(Device device, u32 count, const SamplerDesc *descs, Sampler *out_handles) -> Result<void>
  {
    return backend.create_samplers(device, count, descs, out_handles);
  }

  auto destroy_samplers(Device device, u32 count, Sampler *handles) -> void
  {
    backend.destroy_samplers(device, count, handles);
  }

  auto create_binding_layout(Device device, Span<const BindingLayoutEntry> entries) -> Result<BindingLayout>
  {
    return backend.create_binding_layout(device, entries);
  }

  auto destroy_binding_layout(Device device, BindingLayout layout) -> void
  {
    backend.destroy_binding_layout(device, layout);
  }

  auto create_descriptor_tables(Device device, BindingLayout layout, u32 count, DescriptorTable *out_tables) -> Result<void>
  {
    return backend.create_descriptor_tables(device, layout, count, out_tables);
  }

  auto update_descriptor_tables(Device device, u32 count, const DescriptorUpdate *updates) -> void
  {
    backend.update_descriptor_tables(device, count, updates);
  }

  auto create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage) -> Result<Shader>
  {
    return backend.create_shader(device, spirv_code, size, stage);
  }

  auto destroy_shader(Device device, Shader shader) -> void
  {
    backend.destroy_shader(device, shader);
  }

  // auto create_compute_pipeline(Device device, const ComputePipelineDesc *desc) -> Pipeline
  // {
  //   return backend.create_compute_pipeline(device, desc);
  // }

  auto create_graphics_pipeline(Device device, const GraphicsPipelineDesc *desc) -> Result<Pipeline>
  {
    return backend.create_graphics_pipeline(device, desc);
  }

  auto destroy_pipeline(Device device, Pipeline pipeline) -> void
  {
    backend.destroy_pipeline(device, pipeline);
  }

  auto get_swapchain_format(Device device) -> EFormat
  {
    return backend.get_swapchain_format(device);
  }
} // namespace ghi

namespace ghi
{
  auto cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, u32 count, const Buffer *buffers,
                               const u64 *offsets) -> void
  {
    backend.cmd_bind_vertex_buffers(cmd, first_binding, count, buffers, offsets);
  }

  auto cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices) -> void
  {
    backend.cmd_bind_index_buffer(cmd, buffer, offset, use_32_bit_indices);
  }

  auto cmd_bind_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void
  {
    backend.cmd_bind_pipeline(cmd, pipeline);
  }

  auto cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table) -> void
  {
    backend.cmd_bind_descriptor_table(cmd, set_index, pipeline, table);
  }

  auto cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void
  {
    backend.cmd_set_viewport(cmd, x, y, w, h);
  }

  auto cmd_set_scissor(CommandBuffer cmd, INT32 x, INT32 y, INT32 w, INT32 h) -> void
  {
    backend.cmd_set_scissor(cmd, x, y, w, h);
  }

  auto cmd_draw(CommandBuffer cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) -> void
  {
    backend.cmd_draw(cmd, vertex_count, instance_count, first_vertex, first_instance);
  }

  auto cmd_draw_indexed(CommandBuffer cmd, u32 index_count, u32 instance_count, u32 first_index, u32 first_vertex,
                        u32 first_instance) -> void
  {
    backend.cmd_draw_indexed(cmd, index_count, instance_count, first_index, first_vertex, first_instance);
  }

  auto cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count, u32 stride)
      -> void
  {
    backend.cmd_draw_indexed_indirect(cmd, indirect_buffer, offset, draw_count, stride);
  }

  // auto cmd_dispatch(CommandBuffer cmd, u32 x, u32 y, u32 z) -> void
  // {
  //   backend.cmd_dispatch(cmd, x, y, z);
  // }

  auto cmd_pipeline_barrier(CommandBuffer cmd, u32 buffer_barrier_count, const BufferBarrier *buffer_barriers,
                            u32 texture_barrier_count, const ImageBarrier *texture_barriers) -> void
  {
    backend.cmd_pipeline_barrier(cmd, buffer_barrier_count, buffer_barriers, texture_barrier_count, texture_barriers);
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