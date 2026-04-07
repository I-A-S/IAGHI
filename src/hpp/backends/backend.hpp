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

#include <base.hpp>

namespace ghi
{
  template<typename BackendT>
  concept IAGHI_BACKEND = requires(BackendT backend) {
    { backend.create_device(std::declval<const InitInfo &>()) } -> std::same_as<Result<Device>>;
    { backend.destroy_device(std::declval<Device>()) } -> std::same_as<void>;

    {
      backend.create_buffers(std::declval<Device>(), std::declval<u32>(), std::declval<const BufferDesc *>(),
                             std::declval<Buffer *>())
    } -> std::same_as<Result<void>>;
    {
      backend.destroy_buffers(std::declval<Device>(), std::declval<u32>(), std::declval<const Buffer *>())
    } -> std::same_as<void>;
    { backend.map_buffer(std::declval<Device>(), std::declval<Buffer>()) } -> std::same_as<void *>;
    { backend.unmap_buffer(std::declval<Device>(), std::declval<Buffer>()) } -> std::same_as<void>;

    {
      backend.create_images(std::declval<Device>(), std::declval<u32>(), std::declval<const ImageDesc *>(),
                            std::declval<Image *>())
    } -> std::same_as<Result<void>>;
    {
      backend.destroy_images(std::declval<Device>(), std::declval<u32>(), std::declval<Image *>())
    } -> std::same_as<void>;
    {
      backend.upload_image_data(std::declval<Device>(), std::declval<u32>(), std::declval<Image *>(),
                                std::declval<const u8 **>(), std::declval<bool>())
    } -> std::same_as<Result<void>>;

    {
      backend.create_samplers(std::declval<Device>(), std::declval<u32>(), std::declval<const SamplerDesc *>(),
                              std::declval<Sampler *>())
    } -> std::same_as<Result<void>>;
    {
      backend.destroy_samplers(std::declval<Device>(), std::declval<u32>(), std::declval<Sampler *>())
    } -> std::same_as<void>;

    {
      backend.set_clear_color(std::declval<f32>(), std::declval<f32>(), std::declval<f32>(), std::declval<f32>())
    } -> std::same_as<void>;

    {
      backend.create_binding_layout(std::declval<Device>(), std::declval<Span<const BindingLayoutEntry>>())
    } -> std::same_as<Result<BindingLayout>>;
    { backend.destroy_binding_layout(std::declval<Device>(), std::declval<BindingLayout>()) } -> std::same_as<void>;

    {
      backend.create_descriptor_tables(std::declval<Device>(), std::declval<BindingLayout>(), std::declval<u32>(),
                                       std::declval<DescriptorTable *>())
    } -> std::same_as<Result<void>>;
    {
      backend.update_descriptor_tables(std::declval<Device>(), std::declval<u32>(),
                                       std::declval<const DescriptorUpdate *>())
    } -> std::same_as<void>;

    {
      backend.create_shader(std::declval<Device>(), std::declval<const void *>(), std::declval<usize>(),
                            std::declval<EShaderStage>())
    } -> std::same_as<Result<Shader>>;
    { backend.destroy_shader(std::declval<Device>(), std::declval<Shader>()) } -> std::same_as<void>;

    { backend.get_swapchain_format(std::declval<Device>()) } -> std::same_as<EFormat>;

    // { [IATODO]
    //   backend.create_compute_pipeline(std::declval<Device>(), std::declval<const ComputePipelineDesc *>())
    // } -> std::same_as<Pipeline>;

    {
      backend.create_graphics_pipeline(std::declval<Device>(), std::declval<const GraphicsPipelineDesc *>())
    } -> std::same_as<Result<Pipeline>>;

    { backend.destroy_pipeline(std::declval<Device>(), std::declval<Pipeline>()) } -> std::same_as<void>;

    { backend.begin_frame(std::declval<Device>()) } -> std::same_as<CommandBuffer>;
    { backend.end_frame(std::declval<Device>()) } -> std::same_as<void>;

    { backend.wait_idle(std::declval<Device>()) } -> std::same_as<void>;

    {
      backend.cmd_bind_vertex_buffers(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<u32>(),
                                      std::declval<const Buffer *>(), std::declval<const u64 *>())
    } -> std::same_as<void>;
    {
      backend.cmd_bind_index_buffer(std::declval<CommandBuffer>(), std::declval<Buffer>(), std::declval<u64>(),
                                    std::declval<bool>())
    } -> std::same_as<void>;

    { backend.cmd_bind_pipeline(std::declval<CommandBuffer>(), std::declval<Pipeline>()) } -> std::same_as<void>;

    {
      backend.cmd_bind_descriptor_table(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<Pipeline>(),
                                        std::declval<DescriptorTable>())
    } -> std::same_as<void>;

    {
      backend.cmd_set_viewport(std::declval<CommandBuffer>(), std::declval<f32>(), std::declval<f32>(),
                               std::declval<f32>(), std::declval<f32>())
    } -> std::same_as<void>;
    {
      backend.cmd_set_scissor(std::declval<CommandBuffer>(), std::declval<INT32>(), std::declval<INT32>(),
                              std::declval<INT32>(), std::declval<INT32>())
    } -> std::same_as<void>;

    {
      backend.cmd_draw(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<u32>(), std::declval<u32>(),
                       std::declval<u32>())
    } -> std::same_as<void>;
    {
      backend.cmd_draw_indexed(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<u32>(),
                               std::declval<u32>(), std::declval<u32>(), std::declval<u32>())
    } -> std::same_as<void>;

    {
      backend.cmd_draw_indexed_indirect(std::declval<CommandBuffer>(), std::declval<Buffer>(), std::declval<u64>(),
                                        std::declval<u32>(), std::declval<u32>())
    } -> std::same_as<void>;

    //{ [IATODO]
    //  backend.cmd_dispatch(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<u32>(),
    //  std::declval<u32>())
    //} -> std::same_as<void>;

    {
      backend.cmd_pipeline_barrier(std::declval<CommandBuffer>(), std::declval<u32>(),
                                   std::declval<const BufferBarrier *>(), std::declval<u32>(),
                                   std::declval<const ImageBarrier *>())
    } -> std::same_as<void>;
  };
} // namespace ghi
