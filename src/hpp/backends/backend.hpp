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
  concept IAGHI_BACKEND = requires() {
    { BackendT::create_device(std::declval<const InitInfo &>()) } -> std::same_as<Result<Device>>;
    { BackendT::destroy_device(std::declval<Device>()) } -> std::same_as<void>;

    {
      BackendT::create_buffers(std::declval<Device>(), std::declval<Span<const BufferDesc>>(), std::declval<Buffer *>())
    } -> std::same_as<Result<void>>;
    { BackendT::destroy_buffers(std::declval<Device>(), std::declval<Span<const Buffer>>()) } -> std::same_as<void>;
    { BackendT::map_buffer(std::declval<Device>(), std::declval<Buffer>()) } -> std::same_as<void *>;
    { BackendT::unmap_buffer(std::declval<Device>(), std::declval<Buffer>()) } -> std::same_as<void>;

    {
      BackendT::create_images(std::declval<Device>(), std::declval<Span<const ImageDesc>>(), std::declval<Image *>())
    } -> std::same_as<Result<void>>;
    { BackendT::destroy_images(std::declval<Device>(), std::declval<Span<const Image>>()) } -> std::same_as<void>;
    {
      BackendT::upload_image_data(std::declval<Device>(), std::declval<Span<const Image>>(),
                                std::declval<Span<const u8 *const>>(), std::declval<bool>())
    } -> std::same_as<Result<void>>;

    {
      BackendT::create_samplers(std::declval<Device>(), std::declval<Span<const SamplerDesc>>(),
                              std::declval<Sampler *>())
    } -> std::same_as<Result<void>>;
    { BackendT::destroy_samplers(std::declval<Device>(), std::declval<Span<const Sampler>>()) } -> std::same_as<void>;

    {
      BackendT::create_binding_layout(std::declval<Device>(), std::declval<Span<const BindingLayoutEntry>>())
    } -> std::same_as<Result<BindingLayout>>;
    { BackendT::destroy_binding_layout(std::declval<Device>(), std::declval<BindingLayout>()) } -> std::same_as<void>;

    {
      BackendT::create_descriptor_tables(std::declval<Device>(), std::declval<BindingLayout>(), std::declval<bool>(),
                                       std::declval<u32>(), std::declval<DescriptorTable *>())
    } -> std::same_as<Result<void>>;
    {
      BackendT::update_descriptor_tables(std::declval<Device>(), std::declval<Span<const DescriptorUpdate>>())
    } -> std::same_as<void>;

    {
      BackendT::create_shader(std::declval<Device>(), std::declval<const void *>(), std::declval<usize>(),
                            std::declval<EShaderStage>())
    } -> std::same_as<Result<Shader>>;
    { BackendT::destroy_shader(std::declval<Device>(), std::declval<Shader>()) } -> std::same_as<void>;

    {
      BackendT::create_graphics_pipeline(std::declval<Device>(), std::declval<const GraphicsPipelineDesc &>())
    } -> std::same_as<Result<Pipeline>>;
    { BackendT::destroy_pipeline(std::declval<Device>(), std::declval<Pipeline>()) } -> std::same_as<void>;

    {
      BackendT::resize_swapchain(std::declval<Device>(), std::declval<u32>(), std::declval<u32>())
    } -> std::same_as<Result<void>>;
    { BackendT::get_swapchain_format(std::declval<Device>()) } -> std::same_as<EFormat>;
    {
      BackendT::get_swapchain_extent(std::declval<Device>(), std::declval<u32 &>(), std::declval<u32 &>())
    } -> std::same_as<void>;

    { BackendT::begin_frame(std::declval<Device>()) } -> std::same_as<CommandBuffer>;
    { BackendT::end_frame(std::declval<Device>()) } -> std::same_as<void>;

    { BackendT::wait_idle(std::declval<Device>()) } -> std::same_as<void>;
    {
      BackendT::set_clear_color(std::declval<Device>(), std::declval<f32>(), std::declval<f32>(), std::declval<f32>(), std::declval<f32>())
    } -> std::same_as<void>;

    {
      BackendT::execute_single_time_commands(std::declval<Device>(),
                                           std::declval<const std::function<void(CommandBuffer)> &>())
    } -> std::same_as<Result<void>>;

    {
      BackendT::cmd_copy_buffer(std::declval<CommandBuffer>(), std::declval<Buffer>(), std::declval<Buffer>(),
                              std::declval<u64>(), std::declval<u64>(), std::declval<u64>())
    } -> std::same_as<void>;

    {
      BackendT::cmd_bind_vertex_buffers(std::declval<CommandBuffer>(), std::declval<u32>(),
                                      std::declval<Span<const Buffer>>(), std::declval<Span<const u64>>())
    } -> std::same_as<void>;
    {
      BackendT::cmd_bind_index_buffer(std::declval<CommandBuffer>(), std::declval<Buffer>(), std::declval<u64>(),
                                    std::declval<bool>())
    } -> std::same_as<void>;

    { BackendT::cmd_bind_pipeline(std::declval<CommandBuffer>(), std::declval<Pipeline>()) } -> std::same_as<void>;

    {
      BackendT::cmd_push_constants(std::declval<CommandBuffer>(), std::declval<Pipeline>(), std::declval<u32>(),
                                 std::declval<u32>(), std::declval<const void *>())
    } -> std::same_as<void>;

    {
      BackendT::cmd_bind_frame_bound_descriptor_table(std::declval<CommandBuffer>(), std::declval<u32>(),
                                                    std::declval<Pipeline>(), std::declval<DescriptorTable>())
    } -> std::same_as<void>;
    {
      BackendT::cmd_bind_descriptor_table(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<Pipeline>(),
                                        std::declval<DescriptorTable>(), std::declval<Span<const u32>>())
    } -> std::same_as<void>;

    {
      BackendT::cmd_set_viewport(std::declval<CommandBuffer>(), std::declval<f32>(), std::declval<f32>(),
                               std::declval<f32>(), std::declval<f32>())
    } -> std::same_as<void>;
    {
      BackendT::cmd_set_scissor(std::declval<CommandBuffer>(), std::declval<i32>(), std::declval<i32>(),
                              std::declval<i32>(), std::declval<i32>())
    } -> std::same_as<void>;

    {
      BackendT::cmd_draw(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<u32>(), std::declval<u32>(),
                       std::declval<u32>())
    } -> std::same_as<void>;
    {
      BackendT::cmd_draw_indexed(std::declval<CommandBuffer>(), std::declval<u32>(), std::declval<u32>(),
                               std::declval<u32>(), std::declval<u32>(), std::declval<u32>())
    } -> std::same_as<void>;

    {
      BackendT::cmd_draw_indexed_indirect(std::declval<CommandBuffer>(), std::declval<Buffer>(), std::declval<u64>(),
                                        std::declval<u32>(), std::declval<u32>())
    } -> std::same_as<void>;

    {
      BackendT::cmd_pipeline_barrier(std::declval<CommandBuffer>(), std::declval<Span<const BufferBarrier>>(),
                                   std::declval<Span<const ImageBarrier>>())
    } -> std::same_as<void>;
  };
} // namespace ghi
