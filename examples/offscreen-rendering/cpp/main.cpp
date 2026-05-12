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

#include <auxid/auxid.hpp>
#include <auxid/containers/vec.hpp>

#include <iaghi/iaghi.hpp>
#include <iaghi/utils.hpp>

#include <glm/glm.hpp>
#include <fstream>
#include <vector>

namespace ghi
{
  const auto VERTEX_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vColor;

void main()
{
    gl_Position = vec4(inPosition, 0.0, 1.0);
    vColor = inColor;
}
)";

  const auto FRAGMENT_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(vColor, 1.0);
}
)";

  struct Vertex
  {
    glm::vec2 pos;
    glm::vec3 color;
  };

  auto main() -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    InitInfo init_info{
        .app_name = "IAGHI Offscreen Rendering",
        .validation_enabled = true,
        .surface_width = 800,
        .surface_height = 600,
        .surface_creation_callback = nullptr,
        .surface_creation_callback_user_data = nullptr,
    };

    AU_TRY_VAR(const device, create_device(init_info));

    AU_TRY_DISCARD(utils::initialize(device));

    AU_TRY_VAR(const vertex_shader, utils::create_shader_from_glsl(device, VERTEX_SHADER_SRC, EShaderStage::Vertex));
    AU_TRY_VAR(const fragment_shader,
               utils::create_shader_from_glsl(device, FRAGMENT_SHADER_SRC, EShaderStage::Fragment));

    VertexInputBinding vertex_input_binding{
        .binding = 0,
        .stride = sizeof(Vertex),
        .input_rate = EInputRate::Vertex,
    };

    VertexInputAttribute vertex_input_attributes[2] = {
        {.location = 0, .binding = 0, .format = EFormat::R32G32Float, .offset = offsetof(Vertex, pos)},
        {.location = 1, .binding = 0, .format = EFormat::R32G32B32Float, .offset = offsetof(Vertex, color)},
    };

    GraphicsPipelineDesc pipeline_desc{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,

        .cull_mode = ECullMode::None,

        .binding_layouts = {},
        .vertex_bindings = {vertex_input_binding},
        .vertex_attributes = {vertex_input_attributes},
        .push_constant_ranges = {},
    };
    AU_TRY_VAR(const pipeline, create_graphics_pipeline(device, pipeline_desc));

    destroy_shaders(device, {vertex_shader, fragment_shader});

    au::Vec<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    };

    AU_TRY_VAR(vertex_buffer,
               utils::create_device_local_buffer(device, EBufferUsage::Vertex, sizeof(Vertex) * vertices.size(),
                                                 vertices.data(), sizeof(Vertex) * vertices.size()));

    logger.info("successfully initialized offscreen-rendering");

    const auto cmd = begin_frame(device);

    cmd_begin_pipeline(cmd, pipeline);

    u32 width = 800, height = 600;
    get_swapchain_extent(device, width, height);

    ColorAttachment color_attachment{
        .texture = nullptr,
        .clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
        .load_op_clear = true,
        .store_op_store = true,
    };

    RenderingInfo rendering_info{
        .width = width,
        .height = height,
        .color_attachments = {color_attachment},
        .depth_attachment = nullptr,
    };

    cmd_begin_rendering(device, cmd, rendering_info);

    cmd_set_viewport(cmd, 0, 0, (f32) width, (f32) height);
    cmd_set_scissor(cmd, 0, 0, width, height);

    cmd_bind_vertex_buffers(cmd, 0, {vertex_buffer}, {0});

    cmd_draw(cmd, 3, 1, 0, 0);

    cmd_end_rendering(cmd);

    cmd_end_pipeline(cmd, pipeline);

    end_frame(device);
    wait_idle(device);

    // Read back the rendered image
    std::vector<u8> pixels;
    pixels.resize(width * height * 4);
    AU_TRY_DISCARD(copy_backbuffer_to_cpu(device, pixels));

    // Save to PPM
    std::ofstream ppm("output.ppm", std::ios::binary);
    ppm << "P6\n" << width << " " << height << "\n255\n";

    EFormat format = get_swapchain_format(device);
    bool is_bgra = (format == EFormat::B8G8R8A8Unorm || format == EFormat::B8G8R8A8Srgb);

    for (u32 y = 0; y < height; ++y)
    {
      for (u32 x = 0; x < width; ++x)
      {
        u32 idx = (y * width + x) * 4;
        u8 r, g, b;
        if (is_bgra)
        {
          b = pixels[idx + 0];
          g = pixels[idx + 1];
          r = pixels[idx + 2];
        }
        else
        {
          r = pixels[idx + 0];
          g = pixels[idx + 1];
          b = pixels[idx + 2];
        }
        ppm.write(reinterpret_cast<char *>(&r), 1);
        ppm.write(reinterpret_cast<char *>(&g), 1);
        ppm.write(reinterpret_cast<char *>(&b), 1);
      }
    }
    ppm.close();

    logger.info("Saved offscreen render to output.ppm");

    utils::shutdown(device);
    destroy_pipeline(device, pipeline);
    destroy_buffers(device, {vertex_buffer});
    destroy_device(device);

    logger.info("cleanly exited offscreen-rendering");

    return {};
  }
} // namespace ghi

int main()
{
  au::auxid::MainThreadGuard _thread_guard;

  if (const auto res = ghi::main(); !res)
  {
    au::auxid::get_thread_logger().error("%s", res.error().c_str());
    return -1;
  }

  return 0;
}
