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

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <glm/glm.hpp>

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

  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };

  auto main() -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    ghi::InitInfo init_info{
        .app_name = "IAGHI Hello Triangle",
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
    AU_TRY_VAR(const device, ghi::create_device(init_info));

    AU_TRY_DISCARD(ghi::utils::initialize(device));

    ghi::set_clear_color(device, 0.1f, 0.1f, 0.1f, 1.0f);

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

    ghi::GraphicsPipelineDesc pipeline_desc{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,

        .cull_mode = ECullMode::None,

        .binding_layouts = {},
        .vertex_bindings = {vertex_input_binding},
        .vertex_attributes = {vertex_input_attributes},
        .push_constant_ranges = {},
    };
    AU_TRY_VAR(const pipeline, ghi::create_graphics_pipeline(device, pipeline_desc));

    ghi::destroy_shaders(device, {vertex_shader, fragment_shader});

    au::Vec<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    };

    AU_TRY_VAR(vertex_buffer, ghi::utils::create_device_local_buffer(device, EBufferUsage::Vertex, sizeof(Vertex) * vertices.size(),
                                                                     vertices.data(), sizeof(Vertex) * vertices.size()));

    SDL_ShowWindow(window);

    logger.info("successfully initialized hello-triangle");

    bool running = true;
    while (running)
    {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
        if ((event.type == SDL_EVENT_QUIT) ||
            ((event.type == SDL_EVENT_KEY_DOWN) && (event.key.scancode == SDL_SCANCODE_ESCAPE)))
          running = false;
        else if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
          AU_TRY_DISCARD(ghi::resize_swapchain(device, event.window.data1, event.window.data2));
        }
      }

      const auto cmd = ghi::begin_frame(device);

      ghi::cmd_begin_pipeline(cmd, pipeline);

      u32 width = 800, height = 600;
      ghi::get_swapchain_extent(device, width, height);

      ghi::cmd_set_viewport(cmd, 0, 0, (f32)width, (f32)height);
      ghi::cmd_set_scissor(cmd, 0, 0, width, height);

      ghi::cmd_bind_vertex_buffers(cmd, 0, {vertex_buffer}, {0});

      ghi::cmd_draw(cmd, 3, 1, 0, 0);

      ghi::cmd_end_pipeline(cmd, pipeline);

      ghi::end_frame(device);
    }

    ghi::wait_idle(device);

    ghi::utils::shutdown(device);

    ghi::destroy_pipeline(device, pipeline);

    ghi::destroy_buffers(device, {vertex_buffer});

    ghi::destroy_device(device);

    logger.info("cleanly exited hello-triangle");

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
