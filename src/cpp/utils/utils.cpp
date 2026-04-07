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

#include <iaghi/utils.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

namespace ghi::utils
{
  auto initialize() -> Result<void>
  {
    glslang::InitializeProcess();
  }

  auto shutdown() -> void
  {
    glslang::FinalizeProcess();
  }

  auto compile_glsl(Device device, const char *glsl, EShaderStage stage, const char *entry_point) -> Result<Shader>
  {
    const auto map_stage_to_glslang = [](EShaderStage stage) {
        switch (stage) {
            case EShaderStage::Vertex:   return EShLangVertex;
            case EShaderStage::Fragment: return EShLangFragment;
            case EShaderStage::Compute:  return EShLangCompute;
            default:                     return EShLangVertex;
        }
    };

    EShLanguage glslang_stage = map_stage_to_glslang(stage);
    glslang::TShader shader(glslang_stage);

    shader.setStrings(&glsl, 1);
    shader.setEntryPoint(entry_point);

    shader.setEnvInput(glslang::EShSourceGlsl, glslang_stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    const TBuiltInResource* resources = GetDefaultResources();
    EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

    if (!shader.parse(resources, 100, false, messages)) {
        return fail("shader parsing failed:\n%s\n%s", shader.getInfoLog(), shader.getInfoDebugLog());
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages)) {
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