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

#define VMA_IMPLEMENTATION

#include <backends/vulkan/pipeline.hpp>
#include <backends/vulkan/backend.hpp>
#include <backends/vulkan/device.hpp>
#include <backends/vulkan/buffer.hpp>
#include <backends/vulkan/image.hpp>

namespace ghi
{
  auto VulkanBackend::is_vk_depth_format(VkFormat format) -> bool
  {
    return (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D16_UNORM_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT ||
            format == VK_FORMAT_D32_SFLOAT_S8_UINT);
  }

  auto VulkanBackend::map_format_enum_to_vk(EFormat format) -> VkFormat
  {
    switch (format)
    {
    case EFormat::Undefined:
      return VK_FORMAT_UNDEFINED;

    case EFormat::R8G8B8A8Unorm:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case EFormat::R8G8B8A8Srgb:
      return VK_FORMAT_R8G8B8A8_SRGB;
    case EFormat::B8G8R8A8Srgb:
      return VK_FORMAT_B8G8R8A8_SRGB;
    case EFormat::B8G8R8A8Unorm:
      return VK_FORMAT_B8G8R8A8_UNORM;

    case EFormat::R32Uint:
      return VK_FORMAT_R32_UINT;
    case EFormat::R32Float:
      return VK_FORMAT_R32_SFLOAT;
    case EFormat::R32G32Float:
      return VK_FORMAT_R32G32_SFLOAT;
    case EFormat::R32G32B32Float:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case EFormat::R32G32B32A32Float:
      return VK_FORMAT_R32G32B32A32_SFLOAT;

    case EFormat::D16Unorm:
      return VK_FORMAT_D16_UNORM;
    case EFormat::D16UnormS8Uint:
      return VK_FORMAT_D16_UNORM_S8_UINT;
    case EFormat::D24UnormS8Uint:
      return VK_FORMAT_D24_UNORM_S8_UINT;
    case EFormat::D32Sfloat:
      return VK_FORMAT_D32_SFLOAT;
    case EFormat::D32SfloatS8Uint:
      return VK_FORMAT_D32_SFLOAT_S8_UINT;

    case EFormat::Bc1RgbUnormBlock:
      return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case EFormat::Bc1RgbSrgbBlock:
      return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case EFormat::Bc1RgbaUnormBlock:
      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case EFormat::Bc1RgbaSrgbBlock:
      return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case EFormat::Bc2UnormBlock:
      return VK_FORMAT_BC2_UNORM_BLOCK;
    case EFormat::Bc2SrgbBlock:
      return VK_FORMAT_BC2_SRGB_BLOCK;
    case EFormat::Bc3UnormBlock:
      return VK_FORMAT_BC3_UNORM_BLOCK;
    case EFormat::Bc3SrgbBlock:
      return VK_FORMAT_BC3_SRGB_BLOCK;
    case EFormat::Bc5UnormBlock:
      return VK_FORMAT_BC5_UNORM_BLOCK;
    case EFormat::Bc5SnormBlock:
      return VK_FORMAT_BC5_SNORM_BLOCK;

    default:
      return VK_FORMAT_UNDEFINED;
    }
  }

  auto VulkanBackend::map_vk_to_format_enum(VkFormat format) -> EFormat
  {
    switch (format)
    {
    case VK_FORMAT_UNDEFINED:
      return EFormat::Undefined;

    case VK_FORMAT_R8G8B8A8_UNORM:
      return EFormat::R8G8B8A8Unorm;
    case VK_FORMAT_R8G8B8A8_SRGB:
      return EFormat::R8G8B8A8Srgb;
    case VK_FORMAT_B8G8R8A8_SRGB:
      return EFormat::B8G8R8A8Srgb;
    case VK_FORMAT_B8G8R8A8_UNORM:
      return EFormat::B8G8R8A8Unorm;

    case VK_FORMAT_R32_UINT:
      return EFormat::R32Uint;
    case VK_FORMAT_R32_SFLOAT:
      return EFormat::R32Float;
    case VK_FORMAT_R32G32_SFLOAT:
      return EFormat::R32G32Float;
    case VK_FORMAT_R32G32B32_SFLOAT:
      return EFormat::R32G32B32Float;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
      return EFormat::R32G32B32A32Float;

    case VK_FORMAT_D16_UNORM:
      return EFormat::D16Unorm;
    case VK_FORMAT_D16_UNORM_S8_UINT:
      return EFormat::D16UnormS8Uint;
    case VK_FORMAT_D24_UNORM_S8_UINT:
      return EFormat::D24UnormS8Uint;
    case VK_FORMAT_D32_SFLOAT:
      return EFormat::D32Sfloat;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return EFormat::D32SfloatS8Uint;

    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
      return EFormat::Bc1RgbUnormBlock;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
      return EFormat::Bc1RgbSrgbBlock;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
      return EFormat::Bc1RgbaUnormBlock;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      return EFormat::Bc1RgbaSrgbBlock;
    case VK_FORMAT_BC2_UNORM_BLOCK:
      return EFormat::Bc2UnormBlock;
    case VK_FORMAT_BC2_SRGB_BLOCK:
      return EFormat::Bc2SrgbBlock;
    case VK_FORMAT_BC3_UNORM_BLOCK:
      return EFormat::Bc3UnormBlock;
    case VK_FORMAT_BC3_SRGB_BLOCK:
      return EFormat::Bc3SrgbBlock;
    case VK_FORMAT_BC5_UNORM_BLOCK:
      return EFormat::Bc5UnormBlock;
    case VK_FORMAT_BC5_SNORM_BLOCK:
      return EFormat::Bc5SnormBlock;

    default:
      return EFormat::Undefined;
    }
  }

  auto VulkanBackend::map_shader_stage_enum_to_vk(EShaderStage stage) -> VkShaderStageFlags
  {
    VkShaderStageFlags vk_flags = 0;
    const u32 stage_bits = static_cast<u32>(stage);

    if (stage_bits & static_cast<u32>(EShaderStage::Vertex))
    {
      vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (stage_bits & static_cast<u32>(EShaderStage::Fragment))
    {
      vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (stage_bits & static_cast<u32>(EShaderStage::Compute))
    {
      vk_flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    return vk_flags;
  }

  auto VulkanBackend::map_buffer_usage_enum_to_vk(u32 usage_flags) -> VkBufferUsageFlags
  {
    VkBufferUsageFlags result = 0;

    if (usage_flags & static_cast<u32>(EBufferUsage::Vertex))
    {
      result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if (usage_flags & static_cast<u32>(EBufferUsage::Index))
    {
      result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if ((usage_flags & static_cast<u32>(EBufferUsage::StaticUniform)) ||
        (usage_flags & static_cast<u32>(EBufferUsage::FrameBoundUniform)) ||
        (usage_flags & static_cast<u32>(EBufferUsage::DynamicOffsetUniform)))
    {
      result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if ((usage_flags & static_cast<u32>(EBufferUsage::StaticStorage)) ||
        (usage_flags & static_cast<u32>(EBufferUsage::FrameBoundStorage)) ||
        (usage_flags & static_cast<u32>(EBufferUsage::DynamicOffsetStorage)))
    {
      result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if (usage_flags & static_cast<u32>(EBufferUsage::TransferSrc))
    {
      result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    if (usage_flags & static_cast<u32>(EBufferUsage::TransferDst))
    {
      result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    if (usage_flags & static_cast<u32>(EBufferUsage::Indirect))
    {
      result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }

    return result;
  }

  auto VulkanBackend::map_descriptor_type_enum_to_vk(EDescriptorType type) -> VkDescriptorType
  {
    switch (type)
    {
    case EDescriptorType::UniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case EDescriptorType::StorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    case EDescriptorType::DynamicUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case EDescriptorType::DynamicStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

    case EDescriptorType::SampledImage:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case EDescriptorType::StorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case EDescriptorType::CombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    default:
      return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
  }

  auto VulkanBackend::map_input_rate_enum_to_vk(EInputRate rate) -> VkVertexInputRate
  {
    switch (rate)
    {
    case EInputRate::Vertex:
      return VK_VERTEX_INPUT_RATE_VERTEX;
    case EInputRate::Instance:
      return VK_VERTEX_INPUT_RATE_INSTANCE;
    default:
      return VK_VERTEX_INPUT_RATE_VERTEX;
    }
  }

  auto VulkanBackend::map_polygon_mode_to_vk(EPolygonMode mode) -> VkPolygonMode
  {
    switch (mode)
    {
    case EPolygonMode::Fill:
      return VK_POLYGON_MODE_FILL;
    case EPolygonMode::Line:
      return VK_POLYGON_MODE_LINE;
    case EPolygonMode::Point:
      return VK_POLYGON_MODE_POINT;
    default:
      return VK_POLYGON_MODE_FILL;
    }
  }

  auto VulkanBackend::map_cull_mode_to_vk(ECullMode mode) -> VkCullModeFlags
  {
    switch (mode)
    {
    case ECullMode::None:
      return VK_CULL_MODE_NONE;
    case ECullMode::Back:
      return VK_CULL_MODE_BACK_BIT;
    case ECullMode::Front:
      return VK_CULL_MODE_FRONT_BIT;
    default:
      return VK_CULL_MODE_NONE;
    }
  }

  auto VulkanBackend::map_primitive_type_to_vk(EPrimitiveType type) -> VkPrimitiveTopology
  {
    switch (type)
    {
    case EPrimitiveType::PointList:
      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case EPrimitiveType::LineList:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case EPrimitiveType::LineStrip:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case EPrimitiveType::TriangleList:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case EPrimitiveType::TriangleStrip:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    default:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
  }

  auto VulkanBackend::map_blend_mode_to_vk(EBlendMode mode) -> VkPipelineColorBlendAttachmentState
  {
    VkPipelineColorBlendAttachmentState state{};
    state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    switch (mode)
    {
    case EBlendMode::Opaque:
      state.blendEnable = VK_FALSE;
      break;

    case EBlendMode::Alpha:
      state.blendEnable = VK_TRUE;
      state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      state.colorBlendOp = VK_BLEND_OP_ADD;
      state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      state.alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    case EBlendMode::Premultiplied:
      state.blendEnable = VK_TRUE;
      state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      state.colorBlendOp = VK_BLEND_OP_ADD;
      state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      state.alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    case EBlendMode::Additive:
      state.blendEnable = VK_TRUE;
      state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
      state.colorBlendOp = VK_BLEND_OP_ADD;
      state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      state.alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    case EBlendMode::Multiply:
      state.blendEnable = VK_TRUE;
      state.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
      state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
      state.colorBlendOp = VK_BLEND_OP_ADD;
      state.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
      state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      state.alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    case EBlendMode::Modulate:
      state.blendEnable = VK_TRUE;
      state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
      state.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
      state.colorBlendOp = VK_BLEND_OP_ADD;
      state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      state.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      state.alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    default:
      state.blendEnable = VK_FALSE;
      break;
    }

    return state;
  }
} // namespace ghi

namespace ghi
{
  auto VulkanBackend::create_device(const InitInfo &init_info) -> Result<Device>
  {
    const auto device = AU_TRY(VulkanDevice::create(init_info));
    return reinterpret_cast<Device>(new VulkanDevice(std::move(device)));
  }

  auto VulkanBackend::destroy_device(Device device) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    dev->destroy();
    delete dev;
  }

  auto VulkanBackend::resize_swapchain(Device device, u32 width, u32 height) -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    AU_TRY_DISCARD(dev->m_swapchain.recreate(*dev, width, height));
    return {};
  }

  auto VulkanBackend::get_swapchain_format(Device device) -> EFormat
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    return map_vk_to_format_enum(dev->m_swapchain.m_format);
  }

  auto VulkanBackend::get_swapchain_extent(Device device, u32 &width, u32 &height) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto extent = dev->m_swapchain.get_extent();
    width = extent.width;
    height = extent.height;
  }

  auto VulkanBackend::get_swapchain_image_count(Device device) -> u32
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    return dev->m_swapchain.get_backbuffer_image_count();
  }

  auto VulkanBackend::begin_frame(Device device) -> CommandBuffer
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    if (!dev->m_swapchain.advance_frame(*dev)) // [IATODO] change the dev->x(*dev), decouple these after dinner tonight
      return VK_NULL_HANDLE;

    const auto &frame = dev->m_swapchain.get_frame();

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(frame.command_buffer, &begin_info);

    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = dev->m_swapchain.m_frames[dev->m_swapchain.m_current_frame_index].swapchain_image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    const VkDependencyInfo dep_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };
    vkCmdPipelineBarrier2(frame.command_buffer, &dep_info);

    VkImageView swapchain_backbuffer_color_view{VK_NULL_HANDLE};
    VkImageView swapchain_backbuffer_depth_view{VK_NULL_HANDLE};
    dev->m_swapchain.get_backbuffer_views(swapchain_backbuffer_color_view, swapchain_backbuffer_depth_view);
    VkRenderingAttachmentInfo color_attachment_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain_backbuffer_color_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue =
            {
                .color = dev->m_swapchain.get_clear_color(),
            },
    };
    VkRenderingAttachmentInfo depth_attachment_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain_backbuffer_depth_view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.depthStencil = {1.0f, 0}},
    };
    const VkRenderingInfo rendering_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
            {
                .extent = dev->m_swapchain.get_extent(),
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };
    vkCmdBeginRendering(frame.command_buffer, &rendering_info);

    return reinterpret_cast<CommandBuffer>(frame.command_buffer);
  }

  auto VulkanBackend::end_frame(Device device) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    const auto &frame = dev->m_swapchain.get_frame();

    vkCmdEndRendering(frame.command_buffer);

    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = dev->m_swapchain.m_frames[dev->m_swapchain.m_current_frame_index].swapchain_image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    const VkDependencyInfo dep_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };
    vkCmdPipelineBarrier2(frame.command_buffer, &dep_info);

    vkEndCommandBuffer(frame.command_buffer);

    dev->submit_and_present(frame.command_buffer, frame.in_use_fence);
  }

  auto VulkanBackend::get_active_frame_index(Device device) -> u32
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    return dev->get_swapchain().get_current_frame_index();
  }

  auto VulkanBackend::wait_idle(Device device) -> void
  {
    reinterpret_cast<VulkanDevice *>(device)->wait_idle();
  }

  auto VulkanBackend::set_clear_color(Device device, f32 r, f32 g, f32 b, f32 a) -> void
  {
    reinterpret_cast<VulkanDevice *>(device)->get_swapchain().set_clear_color(r, g, b, a);
  }

  auto VulkanBackend::execute_single_time_commands(Device device,
                                                   const std::function<void(CommandBuffer)> &commands_callback)
      -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    return dev->execute_single_time_commands(
        [=](VkCommandBuffer cmd) { commands_callback(reinterpret_cast<CommandBuffer>(cmd)); });
  }
} // namespace ghi

namespace ghi
{
  auto VulkanBackend::cmd_copy_buffer(CommandBuffer cmd, Buffer src, Buffer dst, u64 size, u64 src_offset,
                                      u64 dst_offset) -> void
  {
    const auto src_buf = reinterpret_cast<VulkanBuffer *>(src);
    const auto dst_buf = reinterpret_cast<VulkanBuffer *>(dst);

    const VkBufferCopy copy_region{
        .srcOffset = src_offset,
        .dstOffset = dst_offset,
        .size = size,
    };

    vkCmdCopyBuffer(reinterpret_cast<VkCommandBuffer>(cmd), src_buf->get_handle(), dst_buf->get_handle(), 1,
                    &copy_region);
  }

  auto VulkanBackend::cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, Span<const Buffer> buffers,
                                              Span<const u64> offsets) -> void
  {
    Vec<VkBuffer> vk_buffers(buffers.size());
    Vec<VkDeviceSize> vk_offsets(buffers.size());

    for (u32 i = 0; i < buffers.size(); ++i)
    {
      const auto *impl = reinterpret_cast<VulkanBuffer *>(buffers[i]);
      vk_buffers[i] = impl->get_handle();
      vk_offsets[i] = offsets[i];
    }

    vkCmdBindVertexBuffers(reinterpret_cast<VkCommandBuffer>(cmd), first_binding, buffers.size(), vk_buffers.data(),
                           vk_offsets.data());
  }

  auto VulkanBackend::cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices)
      -> void
  {
    const auto *impl = reinterpret_cast<VulkanBuffer *>(buffer);
    const VkIndexType type = use_32_bit_indices ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    vkCmdBindIndexBuffer(reinterpret_cast<VkCommandBuffer>(cmd), impl->get_handle(), offset, type);
  }

  auto VulkanBackend::cmd_bind_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void
  {
    const auto p = reinterpret_cast<VulkanGraphicsPipeline *>(pipeline);
    vkCmdBindPipeline(reinterpret_cast<VkCommandBuffer>(cmd), VK_PIPELINE_BIND_POINT_GRAPHICS, p->get_handle());
  }

  auto VulkanBackend::cmd_push_constants(CommandBuffer cmd, Pipeline pipeline, u32 offset, u32 size, const void *data)
      -> void
  {
    const auto p = reinterpret_cast<VulkanGraphicsPipeline *>(pipeline);
    vkCmdPushConstants(reinterpret_cast<VkCommandBuffer>(cmd), p->get_layout(),
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, offset, size, data);
  }

  auto VulkanBackend::cmd_bind_frame_bound_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline,
                                                            DescriptorTable table) -> void
  {
    const auto p = reinterpret_cast<VulkanGraphicsPipeline *>(pipeline);
    const auto *impl = reinterpret_cast<VulkanDescriptorTable *>(table);
    vkCmdBindDescriptorSets(reinterpret_cast<VkCommandBuffer>(cmd), VK_PIPELINE_BIND_POINT_GRAPHICS, p->get_layout(),
                            set_index, 1, &impl->handles[p->get_device()->get_swapchain().get_current_frame_index()], 0,
                            nullptr);
  }

  auto VulkanBackend::cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, Pipeline pipeline,
                                                DescriptorTable table, Span<const u32> offsets) -> void
  {
    const auto p = reinterpret_cast<VulkanGraphicsPipeline *>(pipeline);
    const auto *impl = reinterpret_cast<VulkanDescriptorTable *>(table);
    vkCmdBindDescriptorSets(reinterpret_cast<VkCommandBuffer>(cmd), VK_PIPELINE_BIND_POINT_GRAPHICS, p->get_layout(),
                            set_index, 1, &impl->handles[0], offsets.size(), offsets.data());
  }

  auto VulkanBackend::cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void
  {
    const VkViewport viewport{
        .x = x,
        .y = y,
        .width = w,
        .height = h,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(reinterpret_cast<VkCommandBuffer>(cmd), 0, 1, &viewport);
  }

  auto VulkanBackend::cmd_set_scissor(CommandBuffer cmd, i32 x, i32 y, i32 w, i32 h) -> void
  {
    const VkRect2D scissor{
        .offset = {x, y},
        .extent = {static_cast<u32>(std::max(0, w)), static_cast<u32>(std::max(0, h))},
    };
    vkCmdSetScissor(reinterpret_cast<VkCommandBuffer>(cmd), 0, 1, &scissor);
  }

  auto VulkanBackend::cmd_draw(CommandBuffer cmd, u32 vertex_count, u32 instance_count, u32 first_vertex,
                               u32 first_instance) -> void
  {
    vkCmdDraw(reinterpret_cast<VkCommandBuffer>(cmd), vertex_count, instance_count, first_vertex, first_instance);
  }

  auto VulkanBackend::cmd_draw_indexed(CommandBuffer cmd, u32 index_count, u32 instance_count, u32 first_index,
                                       u32 first_vertex, u32 first_instance) -> void
  {
    vkCmdDrawIndexed(reinterpret_cast<VkCommandBuffer>(cmd), index_count, instance_count, first_index,
                     static_cast<i32>(first_vertex), first_instance);
  }

  auto VulkanBackend::cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count,
                                                u32 stride) -> void
  {
    if (!indirect_buffer)
      return;

    const auto *impl = reinterpret_cast<VulkanBuffer *>(indirect_buffer);
    vkCmdDrawIndexedIndirect(reinterpret_cast<VkCommandBuffer>(cmd), impl->get_handle(), offset, draw_count, stride);
  }

  auto VulkanBackend::cmd_pipeline_barrier(CommandBuffer cmd, Span<const BufferBarrier> buffer_barriers,
                                           Span<const ImageBarrier> image_barriers) -> void
  {
    struct VulkanResourceStateInfo
    {
      VkPipelineStageFlags2 stage;
      VkAccessFlags2 access;
      VkImageLayout layout;
    };

    auto GetStateInfo = [](EResourceState state) -> VulkanResourceStateInfo {
      switch (state)
      {
      case EResourceState::Undefined:
        return {VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED};

      case EResourceState::GeneralRead:
        return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

      case EResourceState::GeneralWrite:
        return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};

      case EResourceState::ColorTarget:
        return {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

      case EResourceState::DepthTarget:
        return {VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

      case EResourceState::Present:
        return {VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

      default:
        return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};
      }
    };

    Vec<VkImageMemoryBarrier2> images;
    Vec<VkBufferMemoryBarrier2> buffers;
    images.reserve(image_barriers.size());
    buffers.reserve(buffer_barriers.size());

    for (const auto& desc : buffer_barriers)
    {
      auto *impl = reinterpret_cast<VulkanBuffer *>(desc.buffer);

      const auto src_info = GetStateInfo(desc.old_state);
      const auto dst_info = GetStateInfo(desc.new_state);

      VkBufferMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = src_info.stage,
        .srcAccessMask = src_info.access,
        .dstStageMask = dst_info.stage,
        .dstAccessMask = dst_info.access,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = impl->get_handle(),
        .offset = 0,
        .size = impl->get_size(),
    };

      buffers.push_back(barrier);
    }

    for (const auto& desc : image_barriers)
    {
      const auto *impl = reinterpret_cast<VulkanImage *>(desc.image);

      const auto src_info = GetStateInfo(desc.old_state);
      const auto dst_info = GetStateInfo(desc.new_state);

      VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
      if (is_vk_depth_format(impl->get_format()))
        aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;

      VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = src_info.stage,
        .srcAccessMask = src_info.access,
        .dstStageMask = dst_info.stage,
        .dstAccessMask = dst_info.access,
        .oldLayout = src_info.layout,
        .newLayout = dst_info.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = impl->get_handle(),
        .subresourceRange =
            {
              .aspectMask = aspect_mask,
              .baseMipLevel = 0,
              .levelCount = VK_REMAINING_MIP_LEVELS,
              .baseArrayLayer = 0,
              .layerCount = VK_REMAINING_ARRAY_LAYERS,
          },
  };

      images.push_back(barrier);
    }

    const VkDependencyInfo dep_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .bufferMemoryBarrierCount = static_cast<u32>(buffers.size()),
        .pBufferMemoryBarriers = buffers.data(),
        .imageMemoryBarrierCount = static_cast<u32>(images.size()),
        .pImageMemoryBarriers = images.data(),
    };

    vkCmdPipelineBarrier2(reinterpret_cast<VkCommandBuffer>(cmd), &dep_info);
  }
} // namespace ghi