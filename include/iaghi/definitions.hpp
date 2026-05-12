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

#include <auxid/auxid.hpp>

namespace ghi
{
  using namespace au;

  // -----------------------------------------------------------------------------
  // Types
  // -----------------------------------------------------------------------------

  struct Device_T;
  struct Buffer_T;
  struct Image_T;
  struct Sampler_T;
  struct Shader_T;
  struct Pipeline_T;
  struct BindingLayout_T;
  struct DescriptorTable_T;
  struct CommandBuffer_T;

  using Device = Device_T *;
  using Buffer = Buffer_T *;
  using Image = Image_T *;
  using Sampler = Sampler_T *;
  using Shader = Shader_T *;
  using Pipeline = Pipeline_T *;
  using BindingLayout = BindingLayout_T *;
  using DescriptorTable = DescriptorTable_T *;
  using CommandBuffer = CommandBuffer_T *;

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

    StaticUniform = 0x4,
    StaticStorage = 0x8,

    FrameBoundUniform = 0x10,
    FrameBoundStorage = 0x20,

    DynamicOffsetUniform = 0x40,
    DynamicOffsetStorage = 0x80,

    TransferSrc = 0x100,
    TransferDst = 0x200,

    Indirect = 0x400,
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

    DynamicUniformBuffer,
    DynamicStorageBuffer,

    SampledImage,
    StorageImage,
    CombinedImageSampler,
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

  /*
  * @brief Creates a native surface for presentation; used when rendering to a window.
  * @param instance_handle Backend-specific instance handle passed by the implementation.
  * @param user_data Opaque pointer supplied via InitInfo::surface_creation_callback_user_data.
  * @return Opaque surface handle for the backend, or nullptr on failure.
  */
  using SurfaceCreationCallback = void *(*)(void *instance_handle, void *user_data);

  // -----------------------------------------------------------------------------
  // Structs
  // -----------------------------------------------------------------------------

  /*
  * @brief Initialization information for the device.
  * @param app_name The name of the application.
  * @param validation_enabled Whether validation is enabled.
  * @param surface_width The width of the surface.
  * @param surface_height The height of the surface.
  * @param surface_creation_callback The callback to create a surface. if `nullptr`, then will be rendered off-screen.
  * @param surface_creation_callback_user_data The user data for the surface creation callback.
  */
  struct InitInfo
  {
    const char *app_name;
    u8 validation_enabled;
    u32 surface_width;
    u32 surface_height;
    SurfaceCreationCallback surface_creation_callback;
    void *surface_creation_callback_user_data;
  };

  /*
  * @brief Parameters for creating a buffer resource.
  * @param size_bytes Size of the buffer in bytes.
  * @param usage Intended usage (vertex, index, uniform, storage, transfer, etc.).
  * @param cpu_visible Non-zero if the buffer should be mappable for CPU access.
  * @param debug_name Optional debug label for tools and validation layers.
  */
  struct BufferDesc
  {
    u64 size_bytes;
    EBufferUsage usage;
    u8 cpu_visible;
    const char *debug_name;
  };

  /*
  * @brief Parameters for creating an image (texture) resource.
  * @param width Image width in texels.
  * @param height Image height in texels.
  * @param depth Depth for 3D textures; use 1 for 2D and array types.
  * @param mip_levels Number of mipmap levels.
  * @param format Texel format.
  * @param array_layers Number of array layers (for array and cube types).
  * @param type Dimensionality and layout (2D, 3D, cube, array).
  * @param debug_name Optional debug label for tools and validation layers.
  */
  struct ImageDesc
  {
    u32 width{1};
    u32 height{1};
    u32 depth{1};
    u32 mip_levels{1};
    EFormat format{};
    u32 array_layers{1};
    ETextureType type{ETextureType::_2D};
    const char *debug_name;
  };

  /*
  * @brief Parameters for creating a sampler state object.
  * @param linear_filter Non-zero for linear filtering; zero for nearest.
  * @param repeat_uv Non-zero to repeat UVs; zero for clamp-to-edge.
  * @param debug_name Optional debug label for tools and validation layers.
  */
  struct SamplerDesc
  {
    u8 linear_filter; // TRUE = Linear, FALSE = Nearest
    u8 repeat_uv;     // TRUE = Repeat, FALSE = Clamp to Edge
    const char *debug_name;
  };

  /*
  * @brief One binding slot in a descriptor binding layout.
  * @param binding Shader binding index.
  * @param count Number of descriptors at this binding (array size).
  * @param visibility Which shader stages may access this binding.
  * @param type Kind of descriptor (uniform buffer, sampled image, etc.).
  */
  struct BindingLayoutEntry
  {
    u32 binding;
    u32 count;
    EShaderStage visibility;
    EDescriptorType type;
  };

  /*
  * @brief Describes a single descriptor table update (bind resources to a table slot).
  * @param table Descriptor table to update.
  * @param binding Binding index within the layout.
  * @param array_element Array element index when the binding is an array.
  * @param buffer Buffer bound for buffer-backed descriptors; otherwise unused.
  * @param buffer_offset Byte offset into buffer for dynamic offsets or sub-range binding.
  * @param buffer_range Byte range for buffer views; 0 may mean whole buffer per implementation.
  * @param image Image bound for image-backed descriptors; otherwise unused.
  * @param sampler Sampler bound for combined image/sampler or sampler descriptors; otherwise unused.
  */
  struct DescriptorUpdate
  {
    DescriptorTable table;
    u32 binding;
    u32 array_element;

    Buffer buffer;
    u64 buffer_offset{0};
    u64 buffer_range{0};

    Image image;
    Sampler sampler;
  };

  /*
  * @brief Vertex input binding: stride and rate for a vertex buffer slot.
  * @param binding Vertex buffer binding index matching shader inputs.
  * @param stride Byte stride between consecutive vertex elements.
  * @param input_rate Per-vertex or per-instance advancement.
  */
  struct VertexInputBinding
  {
    u32 binding;
    u32 stride;
    EInputRate input_rate;
  };

  /*
  * @brief Vertex attribute description for the graphics pipeline.
  * @param location Shader attribute location.
  * @param binding Vertex buffer binding index supplying this attribute.
  * @param format Attribute component format.
  * @param offset Byte offset from the start of the vertex structure.
  */
  struct VertexInputAttribute
  {
    u32 location;
    u32 binding;
    EFormat format;
    u32 offset;
  };

  /*
  * @brief Push constant range visible to selected shader stages.
  * @param offset Byte offset into the push constant block.
  * @param size Size in bytes of this range.
  * @param stages Mask of shader stages that read this range.
  */
  struct PushConstantRange
  {
    u32 offset;
    u32 size;
    EShaderStage stages;
  };

  /*
  * @brief Full description of a graphics pipeline state object.
  * @param vertex_shader Vertex stage SPIR-V module handle.
  * @param fragment_shader Fragment stage SPIR-V module handle.
  * @param depth_target Depth attachment image for depth testing; optional per usage.
  * @param color_targets Color attachments for fragment outputs.
  * @param enable_depth_test Whether depth testing is enabled.
  * @param cull_mode Triangle face culling mode.
  * @param blend_mode Fragment color blending mode.
  * @param polygon_mode Rasterization fill mode (fill, line, point).
  * @param primitive_type Assembly primitive topology.
  * @param binding_layouts Descriptor set layouts used by the pipeline.
  * @param vertex_bindings Vertex buffer binding descriptions.
  * @param vertex_attributes Vertex attribute layout.
  * @param push_constant_ranges Push constant ranges declared by the pipeline layout.
  */
  struct GraphicsPipelineDesc
  {
    Shader vertex_shader;
    Shader fragment_shader;

    Image depth_target{};
    Span<const Image> color_targets{};

    bool enable_depth_test{true};
    ECullMode cull_mode{ECullMode::Back};
    EBlendMode blend_mode{EBlendMode::Alpha};
    EPolygonMode polygon_mode{EPolygonMode::Fill};
    EPrimitiveType primitive_type{EPrimitiveType::TriangleList};

    Span<const BindingLayout> binding_layouts;
    Span<const VertexInputBinding> vertex_bindings;
    Span<const VertexInputAttribute> vertex_attributes;
    Span<const PushConstantRange> push_constant_ranges;
  };

  /*
  * @brief Description of a compute pipeline state object.
  * @param compute_shader Compute stage SPIR-V module handle.
  * @param binding_layouts Descriptor set layouts used by the pipeline.
  */
  struct ComputePipelineDesc
  {
    Shader compute_shader;

    Span<const BindingLayout> binding_layouts;
  };

  /*
  * @brief Color attachment configuration for a render pass slice or clear/load/store ops.
  * @param texture Color attachment image.
  * @param resolve_target Multisample resolve destination; unused if not resolving.
  * @param clear_color RGBA clear values when clearing.
  * @param load_op_clear Non-zero to clear on load; zero to load existing contents.
  * @param store_op_store Non-zero to store results; zero for dont-care.
  */
  struct ColorAttachment
  {
    Image texture;
    Image resolve_target;
    f32 clear_color[4];
    u8 load_op_clear;  // TRUE = CLEAR, FALSE = LOAD
    u8 store_op_store; // TRUE = STORE, FALSE = DONT_CARE
  };

  /*
  * @brief Depth attachment configuration for a render pass slice.
  * @param texture Depth (or depth-stencil) attachment image.
  * @param clear_depth Depth clear value when clearing.
  * @param load_op_clear Non-zero to clear on load; zero to load existing contents.
  * @param store_op_store Non-zero to store results; zero for dont-care.
  */
  struct DepthAttachment
  {
    Image texture;
    f32 clear_depth;
    u8 load_op_clear;
    u8 store_op_store;
  };

  /*
  * @brief Image layout / access transition for pipeline barriers.
  * @param image Image whose state is transitioning.
  * @param old_state Resource state before the barrier.
  * @param new_state Resource state after the barrier.
  */
  struct ImageBarrier
  {
    Image image;
    EResourceState old_state;
    EResourceState new_state;
  };

  /*
  * @brief Buffer layout / access transition for pipeline barriers.
  * @param buffer Buffer whose state is transitioning.
  * @param old_state Resource state before the barrier.
  * @param new_state Resource state after the barrier.
  */
  struct BufferBarrier
  {
    Buffer buffer;
    EResourceState old_state;
    EResourceState new_state;
  };
}