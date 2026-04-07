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

layout(set = 0, binding = 0) uniform sampler2D tex;

void main()
{
    outColor = texture(tex, vUV);
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

    AU_TRY_DISCARD(ghi::utils::initialize(device));

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

    const auto binding_layout =
        AU_TRY(ghi::create_binding_layout(device, {
                                                      BindingLayoutEntry{
                                                          .binding = 0,
                                                          .count = 1,
                                                          .visibility = EShaderStage::Fragment,
                                                          .type = EDescriptorType::CombinedImageSampler,
                                                      },
                                                  }));

    auto color_format = ghi::get_swapchain_format(device);
    ghi::GraphicsPipelineDesc pipeline_desc{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,

        .binding_layouts = &binding_layout,
        .binding_layout_count = 1,

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

    Sampler sampler;
    SamplerDesc sampler_desc{
        .linear_filter = true,
        .repeat_uv = false,
    };
    AU_TRY_DISCARD(ghi::create_samplers(device, 1, &sampler_desc, &sampler));

    auto image = AU_TRY(ghi::utils::create_image_from_file(device, "sandbox/res/6_diffuseOriginal.png"));

    DescriptorTable descriptor_table;
    AU_TRY_DISCARD(ghi::create_descriptor_tables(device, binding_layout, 1, &descriptor_table));

    DescriptorUpdate descriptor_update{
        .table = descriptor_table,
        .binding = 0,
        .array_element = 0,

        .image = image,
        .sampler = sampler,
    };
    update_descriptor_tables(device, 1, &descriptor_update);

    ghi::destroy_shader(device, vertex_shader);
    ghi::destroy_shader(device, fragment_shader);

    Vec<glm::vec4> vertices = {
        {-0.5f, 0.5f, 0.0f, 0.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {0.5f, -0.5f, 1.0f, 1.0f},
        {0.5f, 0.5f, 1.0f, 0.0f},
    };
    Vec<i32> indices = {0, 1, 2, 2, 3, 0};

    Buffer vertex_buffer =
        AU_TRY(ghi::utils::create_device_local_buffer(device, EBufferUsage::Vertex, sizeof(glm::vec4) * vertices.size(),
                                                      vertices.data(), sizeof(glm::vec4) * vertices.size()));
    Buffer index_buffer = AU_TRY(ghi::utils::create_device_local_buffer(
        device, EBufferUsage::Index, sizeof(i32) * indices.size(), indices.data(), sizeof(i32) * indices.size()));

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

      ghi::cmd_bind_descriptor_table(cmd, 0, pipeline, descriptor_table);

      ghi::cmd_set_viewport(cmd, 0, 0, 800, 600);
      ghi::cmd_set_scissor(cmd, 0, 0, 800, 600);

      u64 off = 0;
      ghi::cmd_bind_vertex_buffers(cmd, 0, 1, &vertex_buffer, &off);
      ghi::cmd_bind_index_buffer(cmd, index_buffer, 0, true);

      ghi::cmd_draw_indexed(cmd, 6, 1, 0, 0, 0);

      ghi::end_frame(device);
    }

    ghi::wait_idle(device);

    ghi::utils::shutdown(device);

    ghi::destroy_images(device, 1, &image);
    ghi::destroy_samplers(device, 1, &sampler);

    ghi::destroy_binding_layout(device, binding_layout);

    ghi::destroy_pipeline(device, pipeline);

    ghi::destroy_buffers(device, 1, &vertex_buffer);
    ghi::destroy_buffers(device, 1, &index_buffer);

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