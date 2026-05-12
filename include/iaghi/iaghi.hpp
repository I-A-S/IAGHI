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

#pragma once

#include <iaghi/definitions.hpp>

namespace ghi
{
  /*
  * @brief Returns whether the format is used as a depth or depth-stencil format.
  * @param format Format to query.
  * @return True if depth or depth-stencil.
  */
  auto is_depth_format(EFormat format) -> bool;
  /*
  * @brief Returns whether the format is block-compressed (BC).
  * @param format Format to query.
  * @return True if compressed.
  */
  auto is_compressed_format(EFormat format) -> bool;
  /*
  * @brief Returns the size in bytes of one uncompressed texel (non-compressed formats).
  * @param format Format to query.
  * @return Byte size per texel, or 0 if not applicable.
  */
  auto get_format_byte_size(EFormat format) -> u32;
  /*
  * @brief Returns the compressed block footprint (e.g. 4x4 for BC) for compressed formats.
  * @param format Format to query.
  * @return Block dimension in texels, or 0 if not compressed.
  */
  auto get_compressed_format_block_size(EFormat format) -> u32;

  /*
  * @brief Creates a graphics device and swapchain (or offscreen context).
  * @param init_info Application name, dimensions, validation, and optional surface callback.
  * @return Device handle on success.
  */
  auto create_device(const InitInfo &init_info) -> Result<Device>;
  /*
  * @brief Destroys a device and releases all associated resources.
  * @param device Device to destroy.
  */
  auto destroy_device(Device device) -> void;

  /*
  * @brief Creates one or more buffers from descriptions.
  * @param device Owning device.
  * @param descs Per-buffer creation parameters.
  * @param out_handles Output array receiving buffer handles (parallel to descs).
  * @return Success or error.
  */
  auto create_buffers(Device device, Span<const BufferDesc> descs, Span<Buffer *const> out_handles) -> Result<void>;
  /*
  * @brief Destroys buffer handles.
  * @param device Owning device.
  * @param handles Buffers to destroy.
  */
  auto destroy_buffers(Device device, Span<const Buffer> handles) -> void;
  /*
  * @brief Maps a frame-bound buffer for CPU write/read for the current frame.
  * @param device Owning device.
  * @param buffer Buffer to map.
  * @return Host pointer to mapped memory, or nullptr on failure.
  */
  auto map_frame_bound_buffer(Device device, Buffer buffer) -> void *;
  /*
  * @brief Maps a persistently mappable buffer for CPU access.
  * @param device Owning device.
  * @param buffer Buffer to map.
  * @return Host pointer to mapped memory, or nullptr on failure.
  */
  auto map_buffer(Device device, Buffer buffer) -> void *;
  /*
  * @brief Unmaps a previously mapped buffer.
  * @param device Owning device.
  * @param buffer Buffer to unmap.
  */
  auto unmap_buffer(Device device, Buffer buffer) -> void;

  /*
  * @brief Creates one or more images from descriptions.
  * @param device Owning device.
  * @param descs Per-image creation parameters.
  * @param out_handles Output array receiving image handles (parallel to descs).
  * @return Success or error.
  */
  auto create_images(Device device, Span<const ImageDesc> descs, Span<Image *const> out_handles) -> Result<void>;
  /*
  * @brief Destroys image handles.
  * @param device Owning device.
  * @param handles Images to destroy.
  */
  auto destroy_images(Device device, Span<const Image> handles) -> void;
  /*
  * @brief Uploads pixel data to images and optionally generates mipmaps.
  * @param device Owning device.
  * @param handles Images to update (parallel to image_data).
  * @param image_data Pointer per image to raw pixel data (layout per format).
  * @param generate_mip_maps Whether to build mip chain after upload.
  * @return Success or error.
  */
  auto upload_image_data(Device device, Span<const Image> handles, Span<const u8 *const> image_data, bool generate_mip_maps)
      -> Result<void>;

  /*
  * @brief Creates one or more samplers from descriptions.
  * @param device Owning device.
  * @param descs Per-sampler creation parameters.
  * @param out_handles Output array receiving sampler handles (parallel to descs).
  * @return Success or error.
  */
  auto create_samplers(Device device, Span<const SamplerDesc> descs, Span<Sampler *const> out_handles) -> Result<void>;
  /*
  * @brief Destroys sampler handles.
  * @param device Owning device.
  * @param handles Samplers to destroy.
  */
  auto destroy_samplers(Device device, Span<const Sampler> handles) -> void;

  /*
  * @brief Creates binding layouts from multiple entry sets (one layout per set).
  * @param device Owning device.
  * @param entry_sets For each layout, a span of BindingLayoutEntry defining bindings.
  * @param out_layouts Output handles (one per entry_sets element).
  * @return Success or error.
  */
  auto create_binding_layouts(Device device, Span<const Span<const BindingLayoutEntry>> entry_sets, Span<BindingLayout *const> out_layouts) -> Result<void>;
  /*
  * @brief Destroys binding layout handles.
  * @param device Owning device.
  * @param layouts Layouts to destroy.
  */
  auto destroy_binding_layouts(Device device, Span<const BindingLayout> layouts) -> void;

  /*
  * @brief Allocates descriptor tables for a given layout.
  * @param device Owning device.
  * @param is_frame_bound If true, tables are ring-buffered per frame; otherwise long-lived.
  * @param layout Binding layout the tables are compatible with.
  * @param out_handles Output table handles.
  * @return Success or error.
  */
  auto create_descriptor_tables(Device device, bool is_frame_bound, BindingLayout layout, Span<DescriptorTable *const> out_tables)
      -> Result<void>;
  /*
  * @brief Updates descriptor tables with buffer, image, or sampler bindings.
  * @param device Owning device.
  * @param updates List of DescriptorUpdate records.
  */
  auto update_descriptor_tables(Device device, Span<const DescriptorUpdate> updates) -> void;

  /*
  * @brief Creates a shader module from SPIR-V bytecode.
  * @param device Owning device.
  * @param spirv_code Pointer to SPIR-V words.
  * @param size Size of SPIR-V bytecode in bytes.
  * @param stage Shader stage this module is used with.
  * @return Shader handle on success.
  */
  auto create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage) -> Result<Shader>;
  /*
  * @brief Destroys shader modules.
  * @param device Owning device.
  * @param shaders Shaders to destroy.
  */
  auto destroy_shaders(Device device, Span<const Shader> shaders) -> void;

  /*
  * @brief Creates a graphics pipeline from shaders, vertex layout, and fixed-function state.
  * @param device Owning device.
  * @param desc Full pipeline description.
  * @return Pipeline handle on success.
  */
  auto create_graphics_pipeline(Device device, const GraphicsPipelineDesc &desc) -> Result<Pipeline>;
  /*
  * @brief Destroys a pipeline object.
  * @param device Owning device.
  * @param pipeline Pipeline to destroy.
  */
  auto destroy_pipeline(Device device, Pipeline pipeline) -> void;

  /*
  * @brief Recreates or resizes the swapchain to match the given surface extent.
  * @param device Owning device.
  * @param width New width in pixels.
  * @param height New height in pixels.
  * @return Success or error.
  */
  auto resize_swapchain(Device device, u32 width, u32 height) -> Result<void>;
  /*
  * @brief Returns the swapchain image format (e.g. for pipeline compatibility).
  * @param device Owning device.
  * @return Surface/swapchain color format.
  */
  auto get_swapchain_format(Device device) -> EFormat;
  /*
  * @brief Writes the current swapchain extent to out parameters.
  * @param device Owning device.
  * @param width Out: swapchain width in pixels.
  * @param height Out: swapchain height in pixels.
  */
  auto get_swapchain_extent(Device device, u32 &width, u32 &height) -> void;
  /*
  * @brief Number of swapchain images in the presentation ring.
  * @param device Owning device.
  * @return Image count (typically 2–3).
  */
  auto get_swapchain_image_count(Device device) -> u32;

  /*
  * @brief Begins a new frame and returns a command buffer for recording work.
  * @param device Owning device.
  * @return Command buffer for this frame.
  */
  auto begin_frame(Device device) -> CommandBuffer;
  /*
  * @brief Submits the current frame and presents (if applicable).
  * @param device Owning device.
  */
  auto end_frame(Device device) -> void;
  /*
  * @brief Index of the frame currently being recorded (for ring-buffered resources).
  * @param device Owning device.
  * @return Frame index in [0, swapchain_image_count).
  */
  auto get_active_frame_index(Device device) -> u32;

  /*
  * @brief Copies the current swapchain/backbuffer image to a CPU buffer (e.g. screenshots).
  * @param device Owning device.
  * @param out_data Destination span (layout matches swapchain format and extent).
  * @return Success or error.
  */
  auto copy_backbuffer_to_cpu(Device device, Span<u8> out_data) -> Result<void>;

  /*
  * @brief Blocks until the device is idle (all submitted work finished).
  * @param device Owning device.
  */
  auto wait_idle(Device device) -> void;
  /*
  * @brief Sets the default clear color for render passes that clear color targets.
  * @param device Owning device.
  * @param r Red component.
  * @param g Green component.
  * @param b Blue component.
  * @param a Alpha component (default 1).
  */
  auto set_clear_color(Device device, f32 r, f32 g, f32 b, f32 a = 1.0f) -> void;

  /*
  * @brief Allocates a one-shot command buffer, runs the callback, then submits and waits.
  * @param device Owning device.
  * @param commands_callback Recording lambda receiving a CommandBuffer.
  * @return Success or error.
  */
  auto execute_single_time_commands(Device device, const std::function<void(CommandBuffer)> &commands_callback)
      -> Result<void>;

  /*
  * @brief Records a buffer-to-buffer copy on the given command buffer.
  * @param cmd Command buffer.
  * @param src Source buffer.
  * @param dst Destination buffer.
  * @param size Number of bytes to copy.
  * @param src_offset Offset in source buffer.
  * @param dst_offset Offset in destination buffer.
  */
  auto cmd_copy_buffer(CommandBuffer cmd, Buffer src, Buffer dst, u64 size, u64 src_offset = 0, u64 dst_offset = 0)
      -> void;

  /*
  * @brief Binds vertex buffers and per-buffer offsets.
  * @param cmd Command buffer.
  * @param first_binding First binding slot index.
  * @param buffers Vertex buffers to bind.
  * @param offsets Byte offsets for each binding (parallel to buffers).
  */
  auto cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, Span<const Buffer> buffers, Span<const u64> offsets) -> void;
  /*
  * @brief Binds the index buffer for indexed draws.
  * @param cmd Command buffer.
  * @param buffer Index buffer.
  * @param offset Byte offset into the index buffer.
  * @param use_32_bit_indices True for u32 indices, false for u16.
  */
  auto cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices) -> void;

  /*
  * @brief Binds pipeline state for subsequent draws until cmd_end_pipeline or another begin.
  * @param cmd Command buffer.
  * @param pipeline Graphics or compute pipeline to bind.
  */
  auto cmd_begin_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void;
  /*
  * @brief Ends the active pipeline binding scope (if required by the backend).
  * @param cmd Command buffer.
  * @param pipeline Pipeline that was begun with cmd_begin_pipeline.
  */
  auto cmd_end_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void;

  /*
  * @brief Updates push constants for the bound pipeline layout.
  * @param cmd Command buffer.
  * @param pipeline Pipeline whose layout defines push constant ranges.
  * @param offset Byte offset into the push constant block.
  * @param size Size in bytes of the data pointed to by data.
  * @param data Pointer to push constant values (size bytes).
  */
  auto cmd_push_constants(CommandBuffer cmd, Pipeline pipeline, u32 offset, u32 size, const void *data) -> void;

  /*
  * @brief Binds a frame-bound descriptor table (no dynamic offsets).
  * @param cmd Command buffer.
  * @param set_index Descriptor set index in the pipeline layout.
  * @param pipeline Bound pipeline defining the layout.
  * @param table Descriptor table to bind.
  */
  auto cmd_bind_frame_bound_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table) -> void;
  /*
  * @brief Binds a descriptor table with optional dynamic uniform/storage offsets.
  * @param cmd Command buffer.
  * @param set_index Descriptor set index in the pipeline layout.
  * @param pipeline Bound pipeline defining the layout.
  * @param table Descriptor table to bind.
  * @param offsets Dynamic offsets for bindings that use dynamic offset descriptors (parallel order per layout rules).
  */
  auto cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table, Span<const u32> offsets) -> void;

  /*
  * @brief Sets the viewport rectangle for rasterization.
  * @param cmd Command buffer.
  * @param x Viewport left (pixels).
  * @param y Viewport top (pixels).
  * @param w Viewport width.
  * @param h Viewport height.
  */
  auto cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void;
  /*
  * @brief Sets the scissor rectangle for clipping fragments.
  * @param cmd Command buffer.
  * @param x Scissor left.
  * @param y Scissor top.
  * @param w Scissor width.
  * @param h Scissor height.
  */
  auto cmd_set_scissor(CommandBuffer cmd, i32 x, i32 y, i32 w, i32 h) -> void;

  /*
  * @brief Draws non-indexed geometry.
  * @param cmd Command buffer.
  * @param vertex_count Vertices per instance.
  * @param instance_count Number of instances.
  * @param first_vertex First vertex index in the bound vertex buffers.
  * @param first_instance First instance index.
  */
  auto cmd_draw(CommandBuffer cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) -> void;
  /*
  * @brief Draws indexed geometry using the bound index buffer.
  * @param cmd Command buffer.
  * @param index_count Indices per instance.
  * @param instance_count Number of instances.
  * @param first_index First index relative to the index buffer offset.
  * @param first_vertex Base vertex added to each index (for multi-draw vertex buffer addressing).
  * @param first_instance First instance index.
  */
  auto cmd_draw_indexed(CommandBuffer cmd, u32 index_count, u32 instance_count, u32 first_index, u32 first_vertex,
                        u32 first_instance) -> void;
  /*
  * @brief Dispatches draws from an indirect buffer (GPU-generated draw parameters).
  * @param cmd Command buffer.
  * @param indirect_buffer Buffer containing draw command structures.
  * @param offset Byte offset to the first indirect command.
  * @param draw_count Number of indirect draws to process.
  * @param stride Byte stride between consecutive indirect commands.
  */
  auto cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count, u32 stride)
      -> void;

  /*
  * @brief Inserts memory barriers for buffers and images (layout and visibility).
  * @param cmd Command buffer.
  * @param buffer_barriers Buffer transitions.
  * @param image_barriers Image transitions.
  */
  auto cmd_pipeline_barrier(CommandBuffer cmd, Span<const BufferBarrier> buffer_barriers, Span<const ImageBarrier> image_barriers) -> void;
} // namespace ghi
