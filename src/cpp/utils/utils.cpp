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

#define STB_IMAGE_IMPLEMENTATION

#include <iaghi/utils.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

#include <stb_image.h>

namespace ghi::utils
{
  usize g_staging_buffer_size{64 * 1024};
  Buffer g_staging_buffer{NULL};

  auto ensure_min_staging_buffer_size(Device device, usize size) -> Result<void>
  {
    if (g_staging_buffer_size >= size)
      return {};

    if (g_staging_buffer != NULL)
    {
      ghi::destroy_buffers(device, 1, &g_staging_buffer);
      g_staging_buffer = NULL;
    }

    const BufferDesc buffer_desc{
        .size_bytes = g_staging_buffer_size = size,
        .usage = EBufferUsage::Transfer,
        .cpu_visible = true,
    };
    AU_TRY_DISCARD(ghi::create_buffers(device, 1, &buffer_desc, &g_staging_buffer));

    return {};
  }

  auto initialize(Device device) -> Result<void>
  {
    glslang::InitializeProcess();

    BufferDesc buffer_desc{
        .size_bytes = g_staging_buffer_size,
        .usage = EBufferUsage::Transfer,
        .cpu_visible = true,
    };
    AU_TRY_DISCARD(ghi::create_buffers(device, 1, &buffer_desc, &g_staging_buffer));

    return {};
  }

  auto shutdown(Device device) -> void
  {
    ghi::destroy_buffers(device, 1, &g_staging_buffer);
    g_staging_buffer = NULL;
    g_staging_buffer_size = 0;

    glslang::FinalizeProcess();
  }

  auto create_device_local_buffer(Device device, EBufferUsage usage, usize size, const void *initial_data,
                                  usize initial_data_size) -> Result<Buffer>
  {
    AU_TRY_DISCARD(ensure_min_staging_buffer_size(device, initial_data_size));

    Buffer buffer;
    BufferDesc buffer_desc{
        .size_bytes = size,
        .usage = usage,
        .cpu_visible = false,
    };
    AU_TRY_DISCARD(ghi::create_buffers(device, 1, &buffer_desc, &buffer));

    if (!initial_data)
      return buffer;

    const auto ptr = ghi::map_buffer(device, g_staging_buffer);
    if (!ptr)
      return fail("failed to map staging buffer");
    memcpy(ptr, initial_data, initial_data_size);
    ghi::unmap_buffer(device, g_staging_buffer);

    AU_TRY_DISCARD(ghi::execute_single_time_commands(device, [&](CommandBuffer cmd) {
      ghi::cmd_copy_buffer(cmd, g_staging_buffer, buffer, initial_data_size, 0, 0);
    }));

    return buffer;
  }

  auto create_image_from_file(Device device, const char *filepath, EFormat format) -> Result<Image>
  {
    i32 w, h, nr;
    const auto data = stbi_load(filepath, &w, &h, &nr, STBI_rgb_alpha);
    if (!data)
      return fail("failed to load image");
    const auto result = create_image_from_rgba(device, w, h, data, format);
    delete[] data;
    return result;
  }

  auto create_image_from_rgba(Device device, u32 width, u32 height, const u8 *rgba_data, EFormat format)
      -> Result<Image>
  {
    Image image;
    const ImageDesc desc{
        .width = width,
        .height = height,
        .mip_levels = 1, // [IATODO]
        .format = format,
        .array_layers = 1,         // [IATODO]
        .type = ETextureType::_2D, // [IATODO]
    };
    AU_TRY_DISCARD(ghi::create_images(device, 1, &desc, &image));

    AU_TRY_DISCARD(ghi::upload_image_data(device, 1, &image, &rgba_data, false));

    return image;
  }

  auto create_shader_from_glsl(Device device, const char *glsl, EShaderStage stage, const char *entry_point) -> Result<Shader>
  {
    const auto map_stage_to_glslang = [](EShaderStage stage) {
      switch (stage)
      {
      case EShaderStage::Vertex:
        return EShLangVertex;
      case EShaderStage::Fragment:
        return EShLangFragment;
      case EShaderStage::Compute:
        return EShLangCompute;
      default:
        return EShLangVertex;
      }
    };

    EShLanguage glslang_stage = map_stage_to_glslang(stage);
    glslang::TShader shader(glslang_stage);

    shader.setStrings(&glsl, 1);
    shader.setEntryPoint(entry_point);

    shader.setEnvInput(glslang::EShSourceGlsl, glslang_stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    const TBuiltInResource *resources = GetDefaultResources();
    EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

    if (!shader.parse(resources, 100, false, messages))
    {
      return fail("shader parsing failed:\n%s\n%s", shader.getInfoLog(), shader.getInfoDebugLog());
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages))
    {
      return fail("shader linking failed:\n%s\n%s", program.getInfoLog(), program.getInfoDebugLog());
    }

    std::vector<uint32_t> spirv;
    glslang::SpvOptions spv_options;
    spv_options.generateDebugInfo = false;
    spv_options.disableOptimizer = false;
    spv_options.optimizeSize = false;

    glslang::GlslangToSpv(*program.getIntermediate(glslang_stage), spirv, &spv_options);

    const auto size_bytes = spirv.size() * sizeof(uint32_t);
    const auto shader_obj = create_shader(device, spirv.data(), size_bytes, stage);

    if (!shader_obj)
      return fail("failed to create shader");

    return *shader_obj;
  }
} // namespace ghi::utils
