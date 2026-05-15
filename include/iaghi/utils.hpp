// IAGHI: IA Graphics Hardware Interface
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
// Copyright (C) 2026 IASoft (PVT) LTD (contact@iasoft.dev)
//
// This source code is licensed under the PolyForm Noncommercial License 1.0.0.
// A copy of this license is included in the LICENSE file at the root of this project,
// and is also available at <https://polyformproject.org/licenses/noncommercial/1.0.0>.

#pragma once

#include <iaghi/iaghi.hpp>

namespace ghi::utils
{
  /*
   * @brief Prepares the utils layer: initializes glslang, creates an internal staging buffer, a default sampler,
   *        and a small default checkerboard image used by get_default_image.
   * @param device Device that will use helper routines until shutdown.
   * @return Success or error if initialization fails.
   */
  auto initialize(Device device) -> Result<void>;
  /*
   * @brief Releases default image/sampler and the staging buffer, then finalizes glslang.
   * @param device Device previously passed to initialize.
   */
  auto shutdown(Device device) -> void;

  /*
   * @brief Creates a device-local buffer (not CPU-mappable) and optionally uploads initial bytes via the utils staging
   * buffer.
   * @param device Owning device (initialize must have created a large enough staging buffer, which may be grown here).
   * @param usage Buffer usage flags for the device-local buffer.
   * @param size Total size in bytes.
   * @param initial_data Optional data copied in with a one-shot transfer; nullptr leaves contents undefined.
   * @param initial_data_size Number of bytes copied when initial_data is non-null (typically <= size).
   * @return New buffer handle on success.
   */
  auto create_device_local_buffer(Device device, EBufferUsage usage, usize size, const void *initial_data,
                                  usize initial_data_size) -> Result<Buffer>;

  /*
   * @brief Loads an image via stb_image (decoded as RGBA8), then creates and uploads a 2D image (single mip level).
   * @param device Owning device.
   * @param filepath Path to the image file (formats supported by stb_image).
   * @param format Target texel format for the created image (default sRGBA8).
   * @param generate_mipmaps Whether to automatically generate mipmaps.
   * @return New image handle on success.
   */
  auto create_image_from_file(Device device, const char *filepath, EFormat format = EFormat::R8G8B8A8Srgb,
                              bool generate_mipmaps = true) -> Result<Image>;
  /*
   * @brief Creates a 2D image from raw RGBA8 row-major pixels and uploads it (single mip level; no mip generation).
   * @param device Owning device.
   * @param width Image width in pixels.
   * @param height Image height in pixels.
   * @param rgba_data Pointer to width * height RGBA8 texels (4 bytes per pixel).
   * @param format Target texel format for the created image (default sRGBA8).
   * @param generate_mipmaps Whether to automatically generate mipmaps.
   * @return New image handle on success.
   */
  auto create_image_from_rgba(Device device, u32 width, u32 height, const u8 *rgba_data,
                              EFormat format = EFormat::R8G8B8A8Srgb, bool generate_mipmaps = true) -> Result<Image>;

  /*
   * @brief Returns the default 32×32 checkerboard image created in initialize (violet / white tiles).
   * @return Cached default image handle (null if initialize was not called successfully).
   */
  auto get_default_image() -> Image;
  /*
   * @brief Returns the default sampler from initialize (nearest filtering, repeat UV).
   * @return Cached default sampler handle.
   */
  auto get_default_sampler() -> Sampler;

  /*
   * @brief Compiles GLSL source to SPIR-V and creates a shader module for the given stage.
   * @param device Owning device.
   * @param glsl Null-terminated GLSL source for the requested stage.
   * @param stage Shader stage (vertex, fragment, compute, …).
   * @param entry_point Name of the entry function (default “main”).
   * @return Shader handle on success.
   */
  auto create_shader_from_glsl(Device device, const char *glsl, EShaderStage stage, const char *entry_point = "main")
      -> Result<Shader>;
} // namespace ghi::utils