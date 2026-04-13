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

layout(set = 0, binding = 0) uniform GlobalDataUBO_T {
    mat4 projection;
} global_data;

layout(set = 1, binding = 0) uniform PerFrameDataUBO_T {
    mat4 view;
} per_frame_data;

layout(push_constant, std430) uniform PushConstants {
    mat4 model;
} pc;

void main()
{
    gl_Position = global_data.projection * per_frame_data.view * pc.model * vec4(inPosition, 0.0, 1.0);
    vUV = inTexCoord;
}
)";

  const auto FRAGMENT_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D tex;

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

    ghi::set_clear_color(device, 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);

    const auto vertex_shader = AU_TRY(utils::create_shader_from_glsl(device, VERTEX_SHADER_SRC, EShaderStage::Vertex));
    const auto fragment_shader =
        AU_TRY(utils::create_shader_from_glsl(device, FRAGMENT_SHADER_SRC, EShaderStage::Fragment));

    VertexInputBinding vertex_input_binding{
        .binding = 0,
        .stride = sizeof(glm::vec4),
        .input_rate = EInputRate::Vertex,
    };

    VertexInputAttribute vertex_input_attributes[2] = {
        {.location = 0, .binding = 0, .format = EFormat::R32G32Float, .offset = 0},
        {.location = 1, .binding = 0, .format = EFormat::R32G32Float, .offset = sizeof(glm::vec2)},
    };

    BindingLayout _layout_handles[3] = {};
    AU_TRY_DISCARD(ghi::create_binding_layouts(device,
                                               {
                                                   {
                                                       BindingLayoutEntry{
                                                           .binding = 0,
                                                           .count = 1,
                                                           .visibility = EShaderStage::Fragment,
                                                           .type = EDescriptorType::CombinedImageSampler,
                                                       },
                                                   },
                                                   {
                                                       BindingLayoutEntry{
                                                           .binding = 0,
                                                           .count = 1,
                                                           .visibility = EShaderStage::Vertex,
                                                           .type = EDescriptorType::UniformBuffer,
                                                       },
                                                   },
                                                   {
                                                       BindingLayoutEntry{
                                                           .binding = 0,
                                                           .count = 1,
                                                           .visibility = EShaderStage::Vertex,
                                                           .type = EDescriptorType::UniformBuffer,
                                                       },
                                                   },
                                               },
                                               _layout_handles));

    BindingLayout texture_binding_layout, global_data_binding_layout, per_frame_data_binding_layout;
    texture_binding_layout = _layout_handles[0];
    global_data_binding_layout = _layout_handles[1];
    per_frame_data_binding_layout = _layout_handles[2];

    auto color_format = ghi::get_swapchain_format(device);
    ghi::GraphicsPipelineDesc pipeline_desc{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,

        .color_formats = &color_format,
        .color_attachment_count = 1,
        .depth_format = EFormat::D32Sfloat,
        .cull_mode = ECullMode::None,

        .binding_layouts = {global_data_binding_layout, per_frame_data_binding_layout, texture_binding_layout},
        .vertex_bindings = {vertex_input_binding},
        .vertex_attributes = {vertex_input_attributes},
        .push_constant_ranges =
            {
                PushConstantRange{0, sizeof(glm::mat4),
                                  (EShaderStage) ((u32) EShaderStage::Vertex | (u32) EShaderStage::Fragment)},
            },
    };
    const auto pipeline = AU_TRY(ghi::create_graphics_pipeline(device, pipeline_desc));

    ghi::Buffer buffers[2];
    AU_TRY_DISCARD(ghi::create_buffers(device,
                                       {
                                           ghi::BufferDesc{
                                               .size_bytes = sizeof(glm::mat4),
                                               .usage = EBufferUsage::StaticUniform,
                                               .cpu_visible = true,
                                           },
                                           ghi::BufferDesc{
                                               .size_bytes = sizeof(glm::mat4),
                                               .usage = EBufferUsage::FrameBoundUniform,
                                               .cpu_visible = true,
                                           },
                                       },
                                       buffers));

    DescriptorTable texture_descriptor_table, global_data_descriptor_table, per_frame_data_descriptor_table;
    AU_TRY_DISCARD(ghi::create_descriptor_tables(device, false, texture_binding_layout, 1, &texture_descriptor_table));
    AU_TRY_DISCARD(
        ghi::create_descriptor_tables(device, false, global_data_binding_layout, 1, &global_data_descriptor_table));
    AU_TRY_DISCARD(ghi::create_descriptor_tables(device, true, per_frame_data_binding_layout, 1,
                                                 &per_frame_data_descriptor_table));

    DescriptorUpdate descriptor_updates[3] = {
        {
            .table = global_data_descriptor_table,
            .binding = 0,
            .array_element = 0,

            .buffer = buffers[0],
        },
        {
            .table = per_frame_data_descriptor_table,
            .binding = 0,
            .array_element = 0,

            .buffer = buffers[1],
        },
        {
            .table = texture_descriptor_table,
            .binding = 0,
            .array_element = 0,

            .image = ghi::utils::get_default_image(),
            .sampler = ghi::utils::get_default_sampler(),
        },
    };
    update_descriptor_tables(device, descriptor_updates);

    glm::mat4 m{1.0f};
    {
      const auto ptr = ghi::map_buffer(device, buffers[0]);
      memcpy(ptr, &m, sizeof(glm::mat4));
      ghi::unmap_buffer(device, buffers[0]);
    }

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
        else if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
          AU_TRY_DISCARD(ghi::resize_swapchain(device, event.window.data1, event.window.data2));
        }
      }

      const auto current_frame = static_cast<f32>(SDL_GetTicks()) / 1000.0f;
      delta_time = current_frame - last_frame;
      last_frame = current_frame;

      {
        const auto ptr = ghi::map_frame_bound_buffer(device, buffers[1]);
        memcpy(ptr, &m, sizeof(glm::mat4));
        ghi::unmap_buffer(device, buffers[1]);
      }

      const auto cmd = ghi::begin_frame(device);

      ghi::cmd_bind_pipeline(cmd, pipeline);

      ghi::cmd_bind_descriptor_table(cmd, 0, pipeline, global_data_descriptor_table, {});
      ghi::cmd_bind_descriptor_table(cmd, 2, pipeline, texture_descriptor_table, {});

      ghi::cmd_bind_frame_bound_descriptor_table(cmd, 1, pipeline, per_frame_data_descriptor_table);

      ghi::cmd_set_viewport(cmd, 0, 0, 800, 600);
      ghi::cmd_set_scissor(cmd, 0, 0, 800, 600);

      ghi::cmd_bind_vertex_buffers(cmd, 0, {vertex_buffer}, {0});
      ghi::cmd_bind_index_buffer(cmd, index_buffer, 0, true);

      ghi::cmd_push_constants(cmd, pipeline, 0, sizeof(glm::mat4), &m);

      ghi::cmd_draw_indexed(cmd, 6, 1, 0, 0, 0);

      ghi::end_frame(device);
    }

    ghi::wait_idle(device);

    ghi::utils::shutdown(device);

    ghi::destroy_binding_layouts(device,
                                 {texture_binding_layout, global_data_binding_layout, per_frame_data_binding_layout});

    ghi::destroy_pipeline(device, pipeline);

    ghi::destroy_buffers(device, {vertex_buffer, index_buffer, buffers[0], buffers[1]});

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