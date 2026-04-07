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

#include <iaghi/iaghi.hpp>

namespace ghi::utils
{
  auto initialize(Device device) -> Result<void>;
  auto shutdown(Device device) -> void;

  auto create_device_local_buffer(Device device, EBufferUsage usage, usize size, const void* initial_data, usize initial_data_size) -> Result<Buffer>;

  auto create_image_from_file(Device device, const char* filepath, EFormat format = EFormat::R8G8B8A8Srgb) -> Result<Image>;
  auto create_image_from_rgba(Device device, u32 width, u32 height, const u8* rgba_data, EFormat format = EFormat::R8G8B8A8Srgb) -> Result<Image>;

  auto compile_glsl(Device device, const char* glsl, EShaderStage stage, const char* entry_point = "main") -> Result<Shader>;
}