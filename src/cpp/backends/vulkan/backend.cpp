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

  auto VulkanBackend::map_shader_stage_enum_to_vk(EShaderStage stage) -> VkShaderStageFlags
  {
    VkShaderStageFlags vkFlags = 0;
    const u32 stage_bits = static_cast<u32>(stage);

    if (stage_bits & static_cast<u32>(EShaderStage::Vertex))
    {
      vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (stage_bits & static_cast<u32>(EShaderStage::Fragment))
    {
      vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (stage_bits & static_cast<u32>(EShaderStage::Compute))
    {
      vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    return vkFlags;
  }

  auto VulkanBackend::map_buffer_usage_enum_to_vk(u32 usage_flags) -> VkBufferUsageFlags
  {
    VkBufferUsageFlags result = 0;

    if (usage_flags & static_cast<uint32_t>(EBufferUsage::Vertex))
    {
      result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if (usage_flags & static_cast<uint32_t>(EBufferUsage::Index))
    {
      result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if (usage_flags & static_cast<uint32_t>(EBufferUsage::Uniform))
    {
      result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (usage_flags & static_cast<uint32_t>(EBufferUsage::Storage))
    {
      result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if (usage_flags & static_cast<uint32_t>(EBufferUsage::Transfer))
    {
      result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    if (usage_flags & static_cast<uint32_t>(EBufferUsage::Indirect))
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
    case EDescriptorType::SampledImage:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case EDescriptorType::StorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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
    delete reinterpret_cast<VulkanDevice *>(device);
  }

  auto VulkanBackend::create_buffers(Device device, u32 count, const BufferDesc *descs, Buffer *out_handles)
      -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < count; i++)
    {
      const auto &desc = descs[i];
      const auto buffer =
          AU_TRY(VulkanBuffer::create(dev->m_allocator, desc.size_bytes, map_buffer_usage_enum_to_vk((u32) desc.usage),
                                      desc.cpu_visible, desc.debug_name));
      out_handles[i] = reinterpret_cast<Buffer>(new VulkanBuffer(std::move(buffer)));
    }

    return {};
  }

  auto VulkanBackend::destroy_buffers(Device device, u32 count, const Buffer *handles) -> void
  {
    AU_UNUSED(device);

    for (u32 i = 0; i < count; i++)
    {
      auto buffer = reinterpret_cast<VulkanBuffer *>(handles[i]);
      buffer->destroy();
      delete buffer;
    }
  }

  auto VulkanBackend::map_buffer(Device device, Buffer buffer) -> void *
  {
    const auto buf = reinterpret_cast<VulkanBuffer *>(buffer);

    if (buf->alloc_info.pMappedData)
      return buf->alloc_info.pMappedData;

    void *data;
    vmaMapMemory(buf->allocator_ref, buf->allocation, &data);

    return data;
  }

  auto VulkanBackend::unmap_buffer(Device device, Buffer buffer) -> void
  {
    const auto buf = reinterpret_cast<VulkanBuffer *>(buffer);

    if (!buf->alloc_info.pMappedData)
      vmaUnmapMemory(buf->allocator_ref, buf->allocation);
  }

  auto VulkanBackend::create_images(Device device, u32 count, const ImageDesc *descs, Image *out_handles)
      -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < count; i++)
    {
      const auto &desc = descs[i];
      const auto image = AU_TRY(VulkanImage::create(
          dev->m_handle, dev->m_allocator, map_format_enum_to_vk(desc.format), {desc.width, desc.height, desc.depth},
          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT, desc.array_layers,
          desc.mip_levels)); // [IATODO]: Use ETextureType
      out_handles[i] = reinterpret_cast<Image>(new VulkanImage(std::move(image)));
    }

    return {};
  }

  auto VulkanBackend::destroy_images(Device device, u32 count, Image *handles) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < count; i++)
    {
      auto image = reinterpret_cast<VulkanImage *>(handles[i]);
      image->destroy(dev->m_handle, dev->m_allocator);
      delete image;
    }
  }

  auto VulkanBackend::upload_image_data(Device device, u32 count, Image *handles, const u8 **image_data,
                                        bool generate_mip_maps) -> Result<void>
  {
    const auto insert_image_barrier = [](VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
                                         VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout,
                                         VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                         VkImageSubresourceRange subresourceRange) {
      VkImageMemoryBarrier barrier = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .srcAccessMask = srcAccessMask,
          .dstAccessMask = dstAccessMask,
          .oldLayout = oldLayout,
          .newLayout = newLayout,
          .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .image = image,
          .subresourceRange = subresourceRange,
      };
      vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    };

    if (count == 0)
      return {};

    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    VkDeviceSize totalSize = 0;
    Vec<VkDeviceSize> offsets(count);
    Vec<VkDeviceSize> textureByteSizes(count);

    for (u32 i = 0; i < count; i++)
    {
      auto impl = (VulkanImage *) handles[i];
      if (!impl || !image_data[i])
        continue;

      offsets[i] = totalSize;
      VkDeviceSize currentTextureSize = 0;

      // [IATODO]: Add compressed format support
      currentTextureSize =
          impl->get_extent().width * impl->get_extent().height * impl->get_extent().depth * 4 * impl->get_layer_count();

      textureByteSizes[i] = currentTextureSize;
      totalSize += currentTextureSize;
    }

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo;

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = totalSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VK_CALL(
        vmaCreateBuffer(dev->m_allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, &stagingAllocInfo),
        "create staging buffer for texture upload");

    const auto dataDst = static_cast<u8 *>(stagingAllocInfo.pMappedData);
    for (u32 i = 0; i < count; i++)
    {
      auto impl = reinterpret_cast<VulkanImage *>(handles[i]);
      if (!impl || !image_data[i])
        continue;

      memcpy(dataDst + offsets[i], image_data[i], textureByteSizes[i]);
    }
    vmaFlushAllocation(dev->m_allocator, stagingAlloc, 0, VK_WHOLE_SIZE);

    AU_TRY_DISCARD(dev->execute_single_time_commands([&](VkCommandBuffer cmd) {
      for (u32 i = 0; i < count; i++)
      {
        auto impl = (VulkanImage *) handles[i];
        if (!impl || !image_data[i])
          continue;

        // [IATODO]: Add support for compressed data
        {
          VkImageSubresourceRange range = {};
          range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
          range.baseMipLevel = 0;
          range.levelCount = impl->get_mip_level_count();
          range.baseArrayLayer = 0;
          range.layerCount = impl->get_layer_count();

          insert_image_barrier(cmd, impl->get_handle(), 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, range);

          VkBufferImageCopy region = {};
          region.bufferOffset = offsets[i];
          region.bufferRowLength = 0;
          region.bufferImageHeight = 0;
          region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
          region.imageSubresource.mipLevel = 0;
          region.imageSubresource.baseArrayLayer = 0;
          region.imageSubresource.layerCount = impl->get_layer_count();
          region.imageOffset = {0, 0, 0};
          region.imageExtent = impl->get_extent();

          vkCmdCopyBufferToImage(cmd, stagingBuffer, impl->get_handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                 &region);

          if (generate_mip_maps && impl->get_mip_level_count() > 1)
          {
            int32_t mipWidth = impl->get_extent().width;
            int32_t mipHeight = impl->get_extent().height;

            for (uint32_t j = 1; j < impl->get_mip_level_count(); j++)
            {
              VkImageSubresourceRange mipSubRange = {};
              mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              mipSubRange.baseMipLevel = j - 1;
              mipSubRange.levelCount = 1;
              mipSubRange.baseArrayLayer = 0;
              mipSubRange.layerCount = impl->get_layer_count();

              insert_image_barrier(cmd, impl->get_handle(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange);

              VkImageBlit blit = {};
              blit.srcOffsets[0] = {0, 0, 0};
              blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
              blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              blit.srcSubresource.mipLevel = j - 1;
              blit.srcSubresource.baseArrayLayer = 0;
              blit.srcSubresource.layerCount = impl->get_layer_count();

              blit.dstOffsets[0] = {0, 0, 0};
              blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
              blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              blit.dstSubresource.mipLevel = j;
              blit.dstSubresource.baseArrayLayer = 0;
              blit.dstSubresource.layerCount = impl->get_layer_count();

              vkCmdBlitImage(cmd, impl->get_handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, impl->get_handle(),
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

              insert_image_barrier(cmd, impl->get_handle(), VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, mipSubRange);

              if (mipWidth > 1)
                mipWidth /= 2;
              if (mipHeight > 1)
                mipHeight /= 2;
            }

            VkImageSubresourceRange lastMipRange = {};
            lastMipRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            lastMipRange.baseMipLevel = impl->get_mip_level_count() - 1;
            lastMipRange.levelCount = 1;
            lastMipRange.baseArrayLayer = 0;
            lastMipRange.layerCount = impl->get_layer_count();

            insert_image_barrier(cmd, impl->get_handle(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, lastMipRange);
          }
          else
            insert_image_barrier(cmd, impl->get_handle(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, range);
        }
      }
    }));

    vkDeviceWaitIdle(dev->get_handle());

    vmaDestroyBuffer(dev->m_allocator, stagingBuffer, stagingAlloc);

    return {};
  }

  auto VulkanBackend::create_samplers(Device device, u32 count, const SamplerDesc *descs, Sampler *out_handles)
      -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < count; i++)
    {
      const auto &desc = descs[i];

      VkSamplerCreateInfo info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

      VkFilter filter = desc.linear_filter ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
      VkSamplerAddressMode address =
          desc.repeat_uv ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

      info.magFilter = filter;
      info.minFilter = filter;
      info.mipmapMode = desc.linear_filter ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
      info.addressModeU = address;
      info.addressModeV = address;
      info.addressModeW = address;
      info.mipLodBias = 0.0f;
      info.anisotropyEnable = VK_FALSE;
      info.maxAnisotropy = 16.0f;
      info.minLod = 0.0f;
      info.maxLod = VK_LOD_CLAMP_NONE;
      info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

      VkSampler samplerHandle;
      VK_CALL(vkCreateSampler(dev->get_handle(), &info, nullptr, &samplerHandle), "failed to create sampler");

      out_handles[i] = reinterpret_cast<Sampler>(samplerHandle);
    }

    return {};
  }

  auto VulkanBackend::destroy_samplers(Device device, u32 count, Sampler *handles) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < count; i++)
    {
      if (!handles[i])
        continue;

      const auto impl = reinterpret_cast<VkSampler>(handles[i]);

      vkDestroySampler(dev->get_handle(), impl, nullptr);

      handles[i] = nullptr;
    }
  }

  auto VulkanBackend::create_binding_layout(Device device, Span<const BindingLayoutEntry> entries)
      -> Result<BindingLayout>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto layout = AU_TRY(VulkanBindingLayout::create(*dev, entries));
    return reinterpret_cast<BindingLayout>(new VulkanBindingLayout(std::move(layout)));
  }

  auto VulkanBackend::destroy_binding_layout(Device device, BindingLayout layout) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto layout_impl = reinterpret_cast<VulkanBindingLayout *>(layout);
    layout_impl->destroy(*dev);
    delete layout_impl;
  }

  auto VulkanBackend::create_descriptor_tables(Device device, BindingLayout layout, u32 count,
                                               DescriptorTable *out_tables) -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < count; i++)
    {
      auto layout_impl = reinterpret_cast<VulkanBindingLayout *>(layout);
      const auto table = AU_TRY(VulkanDescriptorTable::create(*dev, layout_impl));
      out_tables[i] = reinterpret_cast<DescriptorTable>(new VulkanDescriptorTable(std::move(table)));
    }

    return {};
  }

  auto VulkanBackend::update_descriptor_tables(Device device, u32 count, const DescriptorUpdate *updates) -> void
  {
    auto &logger = auxid::get_thread_logger();

    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < count; i++)
    {
      const auto &update = updates[i];
      auto table_impl = reinterpret_cast<VulkanDescriptorTable *>(update.table);

      const auto target_type = table_impl->layout->binding_types.find(update.binding);

      if (!target_type)
      {
        logger.error("UpdateDescriptorTables: Binding %d not found in layout", update.binding);
        continue;
      }

      VkWriteDescriptorSet write{};
      write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write.dstSet = table_impl->handle;
      write.dstBinding = update.binding;
      write.dstArrayElement = update.array_element;
      write.descriptorCount = 1;
      write.descriptorType = *target_type;

      VkDescriptorBufferInfo bufferInfo{};
      VkDescriptorImageInfo imageInfo{};

      if (update.buffer)
      {
        const auto buffer_impl = reinterpret_cast<VulkanBuffer *>(update.buffer);
        bufferInfo.buffer = buffer_impl->handle;
        bufferInfo.offset = update.buffer_offset;
        bufferInfo.range = (update.buffer_range == 0) ? VK_WHOLE_SIZE : update.buffer_range;
        write.pBufferInfo = &bufferInfo;
      }
      else if (update.image)
      {
        auto image_impl = reinterpret_cast<VulkanImage *>(update.image);

        VkSampler sampler = VK_NULL_HANDLE;
        if (update.sampler)
          sampler = reinterpret_cast<VkSampler>(update.sampler);

        imageInfo.imageView = image_impl->get_view();
        imageInfo.sampler = sampler;

        if (*target_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || *target_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
        {
          imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else if (*target_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        {
          imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        write.pImageInfo = &imageInfo;
      }

      vkUpdateDescriptorSets(dev->get_handle(), 1, &write, 0, nullptr);
    }
  }

  auto VulkanBackend::create_shader(Device device, const void *spirv_code, usize size, EShaderStage stage)
      -> Result<Shader>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto shader =
        AU_TRY(VulkanShaderModule::create(*dev, Span(static_cast<const u32 *>(spirv_code), size >> 2),
                                          static_cast<VkShaderStageFlagBits>(map_shader_stage_enum_to_vk(stage))));
    return reinterpret_cast<Shader>(new VulkanShaderModule(std::move(shader)));
  }

  auto VulkanBackend::destroy_shader(Device device, Shader shader) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto shader_impl = reinterpret_cast<VulkanShaderModule *>(shader);
    shader_impl->destroy(*dev);
    delete shader_impl;
  }

  auto VulkanBackend::create_graphics_pipeline(Device device, const GraphicsPipelineDesc *desc) -> Result<Pipeline>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto pipeline = AU_TRY(VulkanGraphicsPipeline::create(*dev, *desc));
    return reinterpret_cast<Pipeline>(new VulkanGraphicsPipeline(std::move(pipeline)));
  }

  auto VulkanBackend::destroy_pipeline(Device device, Pipeline pipeline) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);
    const auto pipeline_impl = reinterpret_cast<VulkanGraphicsPipeline *>(pipeline);
    pipeline_impl->destroy(*dev);
    delete pipeline_impl;
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
        .clearValue = {.color = {m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]}},
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

  auto VulkanBackend::wait_idle(Device device) -> void
  {
    reinterpret_cast<VulkanDevice *>(device)->wait_idle();
  }

  auto VulkanBackend::set_clear_color(f32 r, f32 g, f32 b, f32 a) -> void
  {
    m_clear_color[0] = r;
    m_clear_color[1] = g;
    m_clear_color[2] = b;
    m_clear_color[3] = a;
  }
} // namespace ghi

namespace ghi
{
  auto VulkanBackend::cmd_bind_vertex_buffers(CommandBuffer cmd, u32 first_binding, u32 count, const Buffer *buffers,
                                              const u64 *offsets) -> void
  {
    Vec<VkBuffer> vkBuffers(count);
    Vec<VkDeviceSize> vkOffsets(count);

    for (u32 i = 0; i < count; ++i)
    {
      const auto *impl = reinterpret_cast<VulkanBuffer *>(buffers[i]);
      vkBuffers[i] = impl->handle;
      vkOffsets[i] = offsets ? offsets[i] : 0;
    }

    vkCmdBindVertexBuffers(reinterpret_cast<VkCommandBuffer>(cmd), first_binding, count, vkBuffers.data(),
                           vkOffsets.data());
  }

  auto VulkanBackend::cmd_bind_index_buffer(CommandBuffer cmd, Buffer buffer, u64 offset, bool use_32_bit_indices)
      -> void
  {
    const auto *impl = reinterpret_cast<VulkanBuffer *>(buffer);
    const VkIndexType type = use_32_bit_indices ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    vkCmdBindIndexBuffer(reinterpret_cast<VkCommandBuffer>(cmd), impl->handle, offset, type);
  }

  auto VulkanBackend::cmd_bind_pipeline(CommandBuffer cmd, Pipeline pipeline) -> void
  {
    const auto p = reinterpret_cast<VulkanGraphicsPipeline*>(pipeline);
    vkCmdBindPipeline(reinterpret_cast<VkCommandBuffer>(cmd), VK_PIPELINE_BIND_POINT_GRAPHICS, p->get_handle());
  }

  auto VulkanBackend::cmd_bind_descriptor_table(CommandBuffer cmd, u32 set_index, DescriptorTable table) -> void
  {
  }

  auto VulkanBackend::cmd_set_viewport(CommandBuffer cmd, f32 x, f32 y, f32 w, f32 h) -> void
  {
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width = w;
    viewport.height = h;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(reinterpret_cast<VkCommandBuffer>(cmd), 0, 1, &viewport);
  }

  auto VulkanBackend::cmd_set_scissor(CommandBuffer cmd, INT32 x, INT32 y, INT32 w, INT32 h) -> void
  {
    VkRect2D scissor{};
    scissor.offset = {x, y};
    scissor.extent = {(u32) std::max(0, w), (u32) std::max(0, h)};

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
                     (INT32) first_vertex, first_instance);
  }

  auto VulkanBackend::cmd_draw_indexed_indirect(CommandBuffer cmd, Buffer indirect_buffer, u64 offset, u32 draw_count,
                                                u32 stride) -> void
  {
    if (!indirect_buffer)
      return;

    const auto *impl = reinterpret_cast<VulkanBuffer *>(indirect_buffer);
    vkCmdDrawIndexedIndirect(reinterpret_cast<VkCommandBuffer>(cmd), impl->handle, offset, draw_count, stride);
  }

  auto VulkanBackend::cmd_pipeline_barrier(CommandBuffer cmd, u32 buffer_barrier_count,
                                           const BufferBarrier *buffer_barriers, u32 texture_barrier_count,
                                           const ImageBarrier *texture_barriers) -> void
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
    images.reserve(texture_barrier_count);
    buffers.reserve(buffer_barrier_count);

    for (u32 i = 0; i < buffer_barrier_count; i++)
    {
      const auto &desc = buffer_barriers[i];
      auto *impl = reinterpret_cast<VulkanBuffer *>(desc.buffer);

      auto srcInfo = GetStateInfo(desc.old_state);
      auto dstInfo = GetStateInfo(desc.new_state);

      VkBufferMemoryBarrier2 barrier = {
          .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
          .srcStageMask = srcInfo.stage,
          .srcAccessMask = srcInfo.access,
          .dstStageMask = dstInfo.stage,
          .dstAccessMask = dstInfo.access,
          .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .buffer = impl->handle,
          .offset = 0,
          .size = impl->size,
      };

      buffers.push_back(barrier);
    }

    for (u32 i = 0; i < texture_barrier_count; i++)
    {
      const auto &desc = texture_barriers[i];
      auto *impl = reinterpret_cast<VulkanImage *>(desc.image);

      auto srcInfo = GetStateInfo(desc.old_state);
      auto dstInfo = GetStateInfo(desc.new_state);

      VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      if (is_vk_depth_format(impl->get_format()))
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

      VkImageMemoryBarrier2 barrier = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
          .srcStageMask = srcInfo.stage,
          .srcAccessMask = srcInfo.access,
          .dstStageMask = dstInfo.stage,
          .dstAccessMask = dstInfo.access,
          .oldLayout = srcInfo.layout,
          .newLayout = dstInfo.layout,
          .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .image = impl->get_handle(),
          .subresourceRange =
              {
                  .aspectMask = aspectMask,
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