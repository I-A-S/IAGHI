// IAGHI: IA Graphics Hardware Interface
//
// Copyright (C) 2026 I-A-S (ias@iasoft.dev)
// Copyright (C) 2026 IASoft (PVT) LTD (contact@iasoft.dev)
//
// This source code is licensed under the PolyForm Noncommercial License 1.0.0.
// A copy of this license is included in the LICENSE file at the root of this project,
// and is also available at <https://polyformproject.org/licenses/noncommercial/1.0.0>.

#include <auxid/auxid.hpp>
#include <auxid/containers/vec.hpp>

#include <iaghi/iaghi.hpp>
#include <iaghi/utils.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ghi
{
  const auto VERTEX_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vColor;

layout(set = 0, binding = 0) uniform GlobalDataUBO_T {
    mat4 proj_view;
} global_data;

layout(push_constant, std430) uniform PushConstants {
    mat4 model;
} pc;

void main()
{
    gl_Position = global_data.proj_view * pc.model * vec4(inPosition, 1.0);
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
    glm::vec3 pos;
    glm::vec3 color;
  };

  auto main() -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    InitInfo init_info{
        .app_name = "IAGHI Rotating Cube",
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
        {.location = 0, .binding = 0, .format = EFormat::R32G32B32Float, .offset = offsetof(Vertex, pos)},
        {.location = 1, .binding = 0, .format = EFormat::R32G32B32Float, .offset = offsetof(Vertex, color)},
    };

    BindingLayout global_data_binding_layout;
    AU_TRY_DISCARD(create_binding_layouts(device,
                                          {
                                              {
                                                  BindingLayoutEntry{
                                                      .binding = 0,
                                                      .count = 1,
                                                      .visibility = EShaderStage::Vertex,
                                                      .type = EDescriptorType::UniformBuffer,
                                                  },
                                              },
                                          },
                                          {&global_data_binding_layout}));

    GraphicsPipelineDesc pipeline_desc{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,

        .enable_depth_test = true,
        .cull_mode = ECullMode::Back,

        .binding_layouts = {global_data_binding_layout},
        .vertex_bindings = {vertex_input_binding},
        .vertex_attributes = {vertex_input_attributes},
        .push_constant_ranges =
            {
                PushConstantRange{0, sizeof(glm::mat4),
                                  (EShaderStage) ((u32) EShaderStage::Vertex | (u32) EShaderStage::Fragment)},
            },
    };
    AU_TRY_VAR(const pipeline, create_graphics_pipeline(device, pipeline_desc));

    Buffer ubo_global_data_buffer;
    AU_TRY_DISCARD(create_buffers(device,
                                  {
                                      BufferDesc{
                                          .size_bytes = sizeof(glm::mat4),
                                          .usage = EBufferUsage::FrameBoundUniform,
                                          .cpu_visible = true,
                                      },
                                  },
                                  {&ubo_global_data_buffer}));

    DescriptorTable global_data_descriptor_table;
    AU_TRY_DISCARD(create_descriptor_tables(device, true, global_data_binding_layout, {&global_data_descriptor_table}));

    DescriptorUpdate descriptor_updates[1] = {
        {
            .table = global_data_descriptor_table,
            .binding = 0,
            .array_element = 0,
            .buffer = ubo_global_data_buffer,
        },
    };
    update_descriptor_tables(device, descriptor_updates);

    destroy_shaders(device, {vertex_shader, fragment_shader});

    au::Vec<Vertex> vertices = {
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},   {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},  {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
    };
    au::Vec<u16> indices = {
        0, 2, 1, 2, 0, 3, // front
        1, 6, 5, 6, 1, 2, // right
        5, 7, 4, 7, 5, 6, // back
        4, 3, 0, 3, 4, 7, // left
        3, 6, 2, 6, 3, 7, // top
        4, 1, 5, 1, 4, 0, // bottom
    };

    AU_TRY_VAR(vertex_buffer,
               utils::create_device_local_buffer(device, EBufferUsage::Vertex, sizeof(Vertex) * vertices.size(),
                                                 vertices.data(), sizeof(Vertex) * vertices.size()));
    AU_TRY_VAR(index_buffer,
               utils::create_device_local_buffer(device, EBufferUsage::Index, sizeof(u16) * indices.size(),
                                                 indices.data(), sizeof(u16) * indices.size()));

    SDL_ShowWindow(window);

    logger.info("successfully initialized rotating-cube");

    bool running = true;
    f32 rotation = 0.0f;
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
          AU_TRY_DISCARD(resize_swapchain(device, event.window.data1, event.window.data2));
        }
      }

      u32 width = 800, height = 600;
      get_swapchain_extent(device, width, height);

      glm::mat4 proj = glm::perspective(glm::radians(45.0f), (f32) width / (f32) height, 0.1f, 100.0f);
      glm::mat4 view =
          glm::lookAt(glm::vec3(0.0f, 2.0f, 2.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

      // Vulkan specific adjustments
      proj[1][1] *= -1;

      glm::mat4 proj_view = proj * view;

      {
        const auto ptr = map_frame_bound_buffer(device, ubo_global_data_buffer);
        memcpy(ptr, &proj_view, sizeof(glm::mat4));
        unmap_buffer(device, ubo_global_data_buffer);
      }

      const auto cmd = begin_frame(device);

      cmd_begin_pipeline(cmd, pipeline);

      ColorAttachment color_attachment{
          .texture = nullptr,
          .clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
          .load_op_clear = true,
          .store_op_store = true,
      };

      DepthAttachment depth_attachment{
          .texture = nullptr,
          .clear_depth = 1.0f,
          .load_op_clear = true,
          .store_op_store = true,
      };

      RenderingInfo rendering_info{
          .width = width,
          .height = height,
          .color_attachments = {color_attachment},
          .depth_attachment = &depth_attachment,
      };

      cmd_begin_rendering(device, cmd, rendering_info);

      cmd_bind_frame_bound_descriptor_table(cmd, 0, pipeline, global_data_descriptor_table);

      cmd_set_viewport(cmd, 0, 0, (f32) width, (f32) height);
      cmd_set_scissor(cmd, 0, 0, width, height);

      cmd_bind_vertex_buffers(cmd, 0, {vertex_buffer}, {0});
      cmd_bind_index_buffer(cmd, index_buffer, 0, false);

      rotation += 0.01f;
      glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.5f, 1.0f, 0.0f));

      cmd_push_constants(cmd, pipeline, 0, sizeof(glm::mat4), &model);

      cmd_draw_indexed(cmd, 36, 1, 0, 0, 0);

      cmd_end_rendering(cmd);

      cmd_end_pipeline(cmd, pipeline);

      end_frame(device);
    }

    wait_idle(device);

    utils::shutdown(device);

    destroy_binding_layouts(device, {global_data_binding_layout});

    destroy_pipeline(device, pipeline);

    destroy_buffers(device, {vertex_buffer, index_buffer, ubo_global_data_buffer});

    destroy_device(device);

    logger.info("cleanly exited rotating-cube");

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
