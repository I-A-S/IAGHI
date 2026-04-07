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

#define _CRT_SECURE_NO_WARNINGS

#include <auxid/auxid.hpp>
#include <auxid/containers/vec.hpp>

#include <iaghi/iaghi.hpp>
#include <iaghi/utils.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <glm/glm.hpp>

namespace ghi
{
  const auto VERTEX_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 vUV;

void main()
{
    gl_Position = vec4(inPosition, 0.0, 1.0);
    vUV = inTexCoord;
}
)";

  const auto FRAGMENT_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(vUV, 0.5, 1.0);
}
)";

  auto main() -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    ghi::InitInfo init_info{
        .app_name = "IAGHI Sandbox",
        .validation_enabled = true,
        .surface_width = 800,
        .surface_height = 600,
        .surface_creation_callback = [](void *instance_handle, void *user_data) -> void * {
          VkSurfaceKHR surface;
          const auto window = static_cast<SDL_Window *>(user_data);
          SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance_handle), nullptr, &surface);
          return surface;
        },
        .surface_creation_callback_user_data = nullptr,
    };

    SDL_Window *window{};

    if (!SDL_Init(SDL_INIT_VIDEO))
      return fail("failed to initialize SDL '%s'", SDL_GetError());

    window = SDL_CreateWindow(init_info.app_name, init_info.surface_width, init_info.surface_height,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if (!window)
      return fail("failed to create SDL window '%s'", SDL_GetError());

    init_info.surface_creation_callback_user_data = window;
    const auto device = AU_TRY(ghi::create_device(init_info));

    ghi::set_clear_color(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);

    const auto vertex_shader = AU_TRY(utils::compile_glsl(device, VERTEX_SHADER_SRC, EShaderStage::Vertex));
    const auto fragment_shader = AU_TRY(utils::compile_glsl(device, FRAGMENT_SHADER_SRC, EShaderStage::Fragment));

    VertexInputBinding vertex_input_binding{
        .binding = 0,
        .stride = sizeof(glm::vec4),
        .input_rate = EInputRate::Vertex,
    };

    VertexInputAttribute vertex_input_attributes[2] = {
        {.location = 0, .binding = 0, .format = EFormat::R32G32Float, .offset = 0},
        {.location = 1, .binding = 0, .format = EFormat::R32G32Float, .offset = sizeof(glm::vec2)},
    };

    auto color_format = ghi::get_swapchain_format(device);
    ghi::GraphicsPipelineDesc pipeline_desc{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .binding_layout_count = 0,
        .color_formats = &color_format,
        .color_attachment_count = 1,
        .depth_format = EFormat::D32Sfloat,
        .cull_mode = ECullMode::None,
        .vertex_bindings = &vertex_input_binding,
        .vertex_binding_count = 1,
        .vertex_attributes = vertex_input_attributes,
        .vertex_attribute_count = 2,
    };
    const auto pipeline = AU_TRY(ghi::create_graphics_pipeline(device, &pipeline_desc));

    ghi::destroy_shader(device, vertex_shader);
    ghi::destroy_shader(device, fragment_shader);

    Vec<glm::vec4> vertices = {
        {-0.5f, 0.5f, 0.0f, 0.0f},
        {0.0f, -0.5f, 0.0f, 0.0f},
        {0.5f, 0.5f, 0.0f, 0.0f},
    };

    Buffer vertex_buffer{};
    BufferDesc vertex_buffer_desc{
        .size_bytes = sizeof(glm::vec4) * 3,
        .usage = EBufferUsage::Vertex,
        .cpu_visible = true,
    };
    AU_TRY_DISCARD(ghi::create_buffers(device, 1, &vertex_buffer_desc, &vertex_buffer));

    const auto ptr = ghi::map_buffer(device, vertex_buffer);
    if (!ptr)
      return fail("failed to map vertex buffer");
    memcpy(ptr, vertices.data(), sizeof(glm::vec4) * 3);
    ghi::unmap_buffer(device, vertex_buffer);

    SDL_ShowWindow(window);

    logger.info("successfully initialized the engine");

    bool running = true;
    f32 delta_time = 0.0f;
    f32 last_frame = 0.0f;
    while (running)
    {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
        if ((event.type == SDL_EVENT_QUIT) ||
            ((event.type == SDL_EVENT_KEY_DOWN) && (event.key.scancode == SDL_SCANCODE_ESCAPE)))
          running = false;
      }

      const auto current_frame = static_cast<f32>(SDL_GetTicks()) / 1000.0f;
      delta_time = current_frame - last_frame;
      last_frame = current_frame;

      const auto cmd = ghi::begin_frame(device);

      ghi::cmd_bind_pipeline(cmd, pipeline);

      ghi::cmd_set_viewport(cmd, 0, 0, 800, 600);
      ghi::cmd_set_scissor(cmd, 0, 0, 800, 600);

      u64 off = 0;
      ghi::cmd_bind_vertex_buffers(cmd, 0, 1, &vertex_buffer, &off);

      ghi::cmd_draw(cmd, 3, 1, 0, 0);

      ghi::end_frame(device);
    }

    ghi::wait_idle(device);

    ghi::destroy_pipeline(device, pipeline);
    ghi::destroy_buffers(device, 1, &vertex_buffer);

    ghi::destroy_device(device);

    logger.info("cleanly exited the engine");

    SDL_DestroyWindow(window);
    SDL_Quit();

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