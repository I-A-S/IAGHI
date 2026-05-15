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

namespace ghi
{
  const auto COMPUTE_SHADER_SRC = R"(
    #version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, set = 0) buffer InputBuffer {
    float data[];
} inputs[];

layout(binding = 1, set = 0) buffer OutputBuffer {
    float data[];
} outputs[];

layout(push_constant) uniform Constants {
    uint num_elements;
} push_constants;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index >= push_constants.num_elements) {
        return;
    }

    // Multiply by 2.0 to show some compute work happening
    outputs[0].data[index] = inputs[0].data[index] * 2.0;
}
    )";

  auto main() -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    InitInfo init_info{
        .app_name = "hello-compute",
        .validation_enabled = true,
        .surface_width = 1,
        .surface_height = 1,
        .surface_creation_callback = nullptr,
        .surface_creation_callback_user_data = nullptr,
    };

    AU_TRY_VAR(device, create_device(init_info));

    const u32 num_elements = 1024;
    const u32 buffer_size = num_elements * sizeof(f32);

    Vec<f32> initial_data(num_elements);
    for (u32 i = 0; i < num_elements; ++i)
    {
      initial_data[i] = static_cast<f32>(i);
    }

    BufferDesc input_buffer_desc{
        .size_bytes = buffer_size,
        .usage = EBufferUsage::StaticStorage | EBufferUsage::TransferDst,
        .cpu_visible = true,
        .debug_name = "input_buffer",
    };

    BufferDesc output_buffer_desc{
        .size_bytes = buffer_size,
        .usage = EBufferUsage::StaticStorage | EBufferUsage::TransferSrc,
        .cpu_visible = true,
        .debug_name = "output_buffer",
    };

    Buffer input_buffer{};
    Buffer output_buffer{};

    if (!create_buffers(device, {input_buffer_desc, output_buffer_desc}, {&input_buffer, &output_buffer}))
      return fail("Failed to create buffers");

    // Upload initial data to input buffer
    void *input_mapped = map_buffer(device, input_buffer);
    memcpy(input_mapped, initial_data.data(), buffer_size);
    unmap_buffer(device, input_buffer);

    void *early_output_mapped = map_buffer(device, output_buffer);
    memset(early_output_mapped, 0, buffer_size);
    f32 test_read = static_cast<f32 *>(early_output_mapped)[0];
    unmap_buffer(device, output_buffer);

    BindingLayoutEntry layout_entries[] = {
        {
            .binding = 0,
            .count = 1,
            .visibility = EShaderStage::Compute,
            .type = EDescriptorType::StorageBuffer,
            .is_bindless = true,
        },
        {
            .binding = 1,
            .count = 1,
            .visibility = EShaderStage::Compute,
            .type = EDescriptorType::StorageBuffer,
            .is_bindless = true,
        },
    };

    BindingLayout binding_layout{};
    if (!create_binding_layouts(device, {{layout_entries}}, {&binding_layout}))
    {
      return fail("Failed to create binding layout");
    }

    DescriptorTable descriptor_table{};
    if (!create_descriptor_tables(device, false, binding_layout, {&descriptor_table}))
    {
      return fail("Failed to create descriptor table");
    }

    DescriptorUpdate descriptor_updates[] = {
        {
            .table = descriptor_table,
            .binding = 0,
            .array_element = 0,
            .buffer = input_buffer,
        },
        {
            .table = descriptor_table,
            .binding = 1,
            .array_element = 0,
            .buffer = output_buffer,
        },
    };

    update_descriptor_tables(device, descriptor_updates);

    AU_TRY_VAR(compute_shader, utils::create_shader_from_glsl(device, COMPUTE_SHADER_SRC, EShaderStage::Compute));

    ComputePipelineDesc pipeline_desc{
        .compute_shader = compute_shader,
        .binding_layouts = {binding_layout},
        .push_constant_ranges = {{
            .offset = 0,
            .size = sizeof(u32),
            .stages = EShaderStage::Compute,
        }},
    };

    AU_TRY_VAR(pipeline, create_compute_pipeline(device, pipeline_desc));

    auto cmd = begin_frame(device);

    cmd_begin_pipeline(cmd, pipeline);

    cmd_bind_frame_bound_descriptor_table(cmd, 0, pipeline, descriptor_table);

    cmd_push_constants(cmd, pipeline, 0, sizeof(u32), &num_elements);

    u32 group_count_x = (num_elements + 255) / 256;
    cmd_dispatch(cmd, group_count_x, 1, 1);

    cmd_end_pipeline(cmd, pipeline);

    end_frame(device);

    wait_idle(device);

    void *output_mapped = map_buffer(device, output_buffer);
    void *input_mapped_again = map_buffer(device, input_buffer);
    if (!output_mapped)
    {
      return fail("Failed to map output buffer");
    }
    f32 *output_data = static_cast<f32 *>(output_mapped);
    f32 *input_data = static_cast<f32 *>(input_mapped_again);
    f32 dummy = input_data[0];

    bool success = true;
    for (u32 i = 0; i < num_elements; ++i)
    {
      if (output_data[i] != initial_data[i] * 2.0f)
      {
        logger.error("Data mismatch at index %u: expected %f, got %f", i, initial_data[i] * 2.0f, output_data[i]);
        success = false;
      }
    }

    if (success)
    {
      logger.info("Compute shader executed successfully, data was verified!");
    }

    unmap_buffer(device, output_buffer);

    destroy_pipeline(device, pipeline);
    destroy_shaders(device, {compute_shader});
    destroy_binding_layouts(device, {binding_layout});
    destroy_buffers(device, {input_buffer, output_buffer});

    destroy_device(device);

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