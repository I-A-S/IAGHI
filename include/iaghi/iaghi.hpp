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

#include <auxid/auxid.hpp>

namespace ghi
{
  using namespace au;

  // -----------------------------------------------------------------------------
  // Types
  // -----------------------------------------------------------------------------

  typedef struct Device_T *Device;
  typedef struct Buffer_T *Buffer;
  typedef struct Image_T *Image;
  typedef struct Sampler_T *Sampler;
  typedef struct Shader_T *Shader;
  typedef struct Pipeline_T *Pipeline;
  typedef struct BindingLayout_T *BindingLayout;
  typedef struct DescriptorTable_T *DescriptorTable;
  typedef struct CommandBuffer_T *CommandBuffer;

  // -----------------------------------------------------------------------------
  // Enums
  // -----------------------------------------------------------------------------

  enum class EFormat
  {
    Undefined = 0,
    R8G8B8A8Unorm,
    R8G8B8A8Srgb,
    B8G8R8A8Srgb,
    B8G8R8A8Unorm,
    R32Uint,
    R32Float,
    R32G32Float,
    R32G32B32Float,
    R32G32B32A32Float,

    D16Unorm,
    D16UnormS8Uint,
    D24UnormS8Uint,
    D32Sfloat,
    D32SfloatS8Uint,

    Bc1RgbUnormBlock,
    Bc1RgbSrgbBlock,
    Bc1RgbaUnormBlock,
    Bc1RgbaSrgbBlock,
    Bc2UnormBlock,
    Bc2SrgbBlock,
    Bc3UnormBlock,
    Bc3SrgbBlock,
    Bc5UnormBlock,
    Bc5SnormBlock,
  };

  enum class ETextureType
  {
    _2D,
    _3D,
    Cube,
    _2DArray,
  };

  enum class EShaderStage
  {
    Vertex = 0x1,
    Fragment = 0x2,
    Compute = 0x4
  };

  enum class EBufferUsage
  {
    Vertex = 0x1,
    Index = 0x2,
    Uniform = 0x4,
    Storage = 0x8,
    Transfer = 0x10,
    Indirect = 0x20
  };

  enum class EResourceState
  {
    Undefined = 0,
    GeneralRead,
    GeneralWrite,
    ColorTarget,
    DepthTarget,
    Present,
  };

  enum class EDescriptorType
  {
    UniformBuffer,
    StorageBuffer,
    SampledImage,
    StorageImage,
  };

  enum class EInputRate
  {
    Vertex = 0,
    Instance = 1
  };

  enum class EPolygonMode
  {
    Fill,
    Line,
    Point,
  };

  enum class ECullMode
  {
    None,
    Back,
    Front,
  };

  enum class EBlendMode
  {
    Opaque,
    Alpha,
    Premultiplied,
    Additive,
    Multiply,
    Modulate
  };

  enum class EPrimitiveType
  {
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
  };

  // -----------------------------------------------------------------------------
  // Callbacks
  // -----------------------------------------------------------------------------

  typedef void *(*SurfaceCreationCallback)(void *instance_handle, void *user_data);

  // -----------------------------------------------------------------------------
  // Structs
  // -----------------------------------------------------------------------------

  typedef struct InitInfo
  {
    const char *app_name;
    u8 validation_enabled;
    u32 surface_width;
    u32 surface_height;
    SurfaceCreationCallback surface_creation_callback;
    void *surface_creation_callback_user_data;
  } InitInfo;

  typedef struct BufferDesc
  {
    u64 size_bytes;
    EBufferUsage usage;
    u8 cpu_visible;
    const char *debug_name;
  } BufferDesc;

  typedef struct ImageDesc
  {
    u32 width;
    u32 height;
    u32 depth;
    u32 mip_levels;
    EFormat format;
    u32 array_layers;
    ETextureType type;
    const char *debug_name;
  } ImageDesc;

  typedef struct SamplerDesc
  {
    u8 linear_filter; // TRUE = Linear, FALSE = Nearest
    u8 repeat_uv;     // TRUE = Repeat, FALSE = Clamp to Edge
    const char *debug_name;
  } SamplerDesc;

  typedef struct BindingLayoutEntry
  {
    u32 binding;
    u32 count;
    EShaderStage visibility;
    EDescriptorType type;
  } BindingLayoutEntry;

  typedef struct DescriptorUpdate
  {
    DescriptorTable table;
    u32 binding;
    u32 array_element;

    Buffer buffer;
    u64 buffer_offset;
    u64 buffer_range;

    Image image;
    Sampler sampler;
  } DescriptorUpdate;

  typedef struct VertexInputBinding
  {
    u32 binding;
    u32 stride;
    EInputRate input_rate;
  } VertexInputBinding;

  typedef struct VertexInputAttribute
  {
    u32 location;
    u32 binding;
    EFormat format;
    u32 offset;
  } VertexInputAttribute;

  typedef struct GraphicsPipelineDesc
  {
    Shader vertex_shader;
    Shader fragment_shader;
    BindingLayout *binding_layouts;
    u32 binding_layout_count;

    EFormat *color_formats;
    u32 color_attachment_count;
    EFormat depth_format;

    ECullMode cull_mode{ECullMode::Back};
    EBlendMode blend_mode{EBlendMode::Alpha};
    EPolygonMode polygon_mode{EPolygonMode::Fill};
    EPrimitiveType primitive_type{EPrimitiveType::TriangleList};

    VertexInputBinding* vertex_bindings;
    u32 vertex_binding_count;
    VertexInputAttribute* vertex_attributes;
    u32 vertex_attribute_count;
  } GraphicsPipelineDesc;

  typedef struct ComputePipelineDesc
  {
    Shader compute_shader;
    BindingLayout *layouts;
    u32 layout_count;
  } ComputePipelineDesc;

  typedef struct ColorAttachment
  {
    Image texture;
    Image resolve_target;
    f32 clear_color[4];
    u8 load_op_clear;  // TRUE = CLEAR, FALSE = LOAD
    u8 store_op_store; // TRUE = STORE, FALSE = DONT_CARE
  } ColorAttachment;

  typedef struct DepthAttachment
  {
    Image texture;
    f32 clear_depth;
    u8 load_op_clear;
    u8 store_op_store;
  } DepthAttachment;

  typedef struct ImageBarrier
  {
    Image image;
    EResourceState old_state;
    EResourceState new_state;
  } ImageBarrier;

  typedef struct BufferBarrier
  {
    Buffer buffer;
    EResourceState old_state;
    EResourceState new_state;
  } BufferBarrier;

  // -----------------------------------------------------------------------------
  // API
  // -----------------------------------------------------------------------------

  auto create_device(const InitInfo &init_info) -> Result<Device>;
  auto destroy_device(Device device) -> void;

  auto create_buffers(Device device, u32 count, const BufferDesc *descs, Buffer *out_handles) -> Result<void>;
  auto destroy_buffers(Device device, u32 count, const Buffer *handles) -> void;
  auto map_buffer(Device device, Buffer buffer) -> void *;
  auto unmap_buffer(Device device, Buffer buffer) -> void;

  auto create_images(Device device, u32 count, const ImageDesc *descs, Image *out_handles) -> Result<void>;
  auto destroy_images(Device device, u32 count, Image *handles) -> void;
  auto upload_image_data(Device device, u32 count, Image *handles, const u8 **image_data,
                                 bool generate_mip_maps) -> Result<void>;

  auto is_depth_format(EFormat format) -> bool;
  auto is_compressed_format(EFormat format) -> bool;
  auto get_compressed_format_block_size(EFormat format) -> u32;

  auto create_samplers(Device device, u32 count, const SamplerDesc *descs, Sampler *out_handles) -> Result<void>;
  auto destroy_samplers(Device device, u32 count, Sampler *handles) -> void;

  auto create_binding_layout(Device device, Span<const BindingLayoutEntry> entries) -> Result<BindingLayout>;
  auto destroy_binding_layout(Device device, BindingLayout layout) -> void;

  auto create_descriptor_tables(Device device, BindingLayout layout, u32 count, DescriptorTable *out_tables) -> Result<void>;
  auto update_descriptor_tables(Device device, u32 count, const DescriptorUpdate *updates) -> void;

  auto create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage) -> Result<Shader>;
  auto destroy_shader(Device device, Shader shader) -> void;

  // [IATODO] auto create_compute_pipeline(Device device, const ComputePipelineDesc *desc) -> Pipeline;
  auto create_graphics_pipeline(Device device, const GraphicsPipelineDesc *desc) -> Result<Pipeline>;
  auto destroy_pipeline(Device device, Pipeline pipeline) -> void;

  auto get_swapchain_format(Device device) -> EFormat;

  auto begin_frame(Device device) -> CommandBuffer;
  auto end_frame(Device device) -> void;

  auto wait_idle(Device device) -> void;
  auto set_clear_color(f32 r, f32 g, f32 b, f32 a = 1.0f) -> void;

  auto cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, u32 count, const Buffer *buffers,
                               const u64 *offsets) -> void;
  auto cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices) -> void;

  auto cmd_bind_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void;

  auto cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline, DescriptorTable table) -> void;

  auto cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void;
  auto cmd_set_scissor(CommandBuffer cmd, INT32 x, INT32 y, INT32 w, INT32 h) -> void;

  auto cmd_draw(CommandBuffer cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) -> void;
  auto cmd_draw_indexed(CommandBuffer cmd, u32 index_count, u32 instance_count, u32 first_index, u32 first_vertex,
                        u32 first_instance) -> void;

  auto cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count, u32 stride)
      -> void;

  // [IATODO] auto cmd_dispatch(CommandBuffer cmd, u32 x, u32 y, u32 z) -> void;

  auto cmd_pipeline_barrier(CommandBuffer cmd, u32 buffer_barrier_count, const BufferBarrier *buffer_barriers,
                            u32 texture_barrier_count, const ImageBarrier *texture_barriers) -> void;
} // namespace ghi
