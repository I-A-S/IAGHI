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

#include <backends/vulkan/image.hpp>
#include <backends/vulkan/device.hpp>

namespace ghi
{
  auto VulkanImage::create(VkDevice device, VmaAllocator allocator, VkFormat format, VkExtent3D extent,
                           VkImageUsageFlags usage, VkImageUsageFlags aspect_flags, u32 layer_count,
                           u32 mip_level_count) -> Result<VulkanImage>
  {
    VulkanImage result{};

    result.m_format = format;
    result.m_extent = extent;
    result.m_layer_count = layer_count;
    result.m_mip_level_count = mip_level_count;
    result.m_format_enum = VulkanBackend::map_vk_to_format_enum(format);

    VkImageCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = mip_level_count,
        .arrayLayers = layer_count,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImageViewCreateInfo view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = (layer_count == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .format = create_info.format,
        .subresourceRange =
            {
                .aspectMask = aspect_flags,
                .baseMipLevel = 0,
                .levelCount = mip_level_count,
                .baseArrayLayer = 0,
                .layerCount = layer_count,
            },
    };

    VmaAllocationCreateInfo alloc_info{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VMA_MEMORY_USAGE_GPU_ONLY,
        .preferredFlags = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    VK_CALL(vmaCreateImage(allocator, &create_info, &alloc_info, &result.m_handle, &result.m_allocation,
                           &result.m_allocation_info),
            "creating and allocating image");

    view_create_info.image = result.m_handle;
    VK_CALL(vkCreateImageView(device, &view_create_info, nullptr, &result.m_view), "creating image view");

    return result;
  }

  auto VulkanImage::destroy(VkDevice device, VmaAllocator allocator) -> void
  {
    vkDestroyImageView(device, m_view, nullptr);
    vmaDestroyImage(allocator, m_handle, m_allocation);
    m_handle = VK_NULL_HANDLE;
  }

  auto VulkanBackend::create_images(Device device, Span<const ImageDesc> descs, Image *out_handles) -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    u32 i{0};
    for (const auto &desc : descs)
    {
      const auto image = AU_TRY(VulkanImage::create(
          dev->m_handle, dev->m_allocator, map_format_enum_to_vk(desc.format), {desc.width, desc.height, desc.depth},
          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT, desc.array_layers,
          desc.mip_levels)); // [IATODO]: Use ETextureType
      out_handles[i++] = reinterpret_cast<Image>(new VulkanImage(std::move(image)));
    }

    return {};
  }

  auto VulkanBackend::destroy_images(Device device, Span<const Image> handles) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (const auto &handle : handles)
    {
      auto image = reinterpret_cast<VulkanImage *>(handle);
      image->destroy(dev->m_handle, dev->m_allocator);
      delete image;
    }
  }

  auto VulkanBackend::upload_image_data(Device device, Span<const Image> handles, Span<const u8 *const> image_data,
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

    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    if (handles.size() == 0)
      return {};

    VkDeviceSize totalSize = 0;
    Vec<VkDeviceSize> offsets(handles.size());
    Vec<VkDeviceSize> textureByteSizes(handles.size());

    for (u32 i = 0; i < handles.size(); i++)
    {
      auto impl = reinterpret_cast<VulkanImage *>(handles[i]);
      if (!impl || !image_data[i])
        continue;

      offsets[i] = totalSize;
      VkDeviceSize currentTextureSize = 0;

      if (ghi::is_compressed_format(impl->get_format_enum()))
      {
        u32 mipW = impl->get_extent().width;
        u32 mipH = impl->get_extent().height;
        u32 blockSize = ghi::get_compressed_format_block_size(impl->get_format_enum());

        for (u32 m = 0; m < impl->get_mip_level_count(); m++)
        {
          u32 blocksX = (mipW + 3) / 4;
          u32 blocksY = (mipH + 3) / 4;
          currentTextureSize += blocksX * blocksY * blockSize;

          mipW = std::max(1u, mipW / 2);
          mipH = std::max(1u, mipH / 2);
        }
        currentTextureSize *= impl->get_layer_count();

        generate_mip_maps = false;
      }
      else
      {
        currentTextureSize = impl->get_extent().width * impl->get_extent().height * impl->get_extent().depth *
                             ghi::get_format_byte_size(impl->get_format_enum()) * impl->get_layer_count();
      }

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
    for (u32 i = 0; i < handles.size(); i++)
    {
      auto impl = reinterpret_cast<VulkanImage *>(handles[i]);
      if (!impl || !image_data[i])
        continue;

      memcpy(dataDst + offsets[i], image_data[i], textureByteSizes[i]);
    }
    vmaFlushAllocation(dev->m_allocator, stagingAlloc, 0, VK_WHOLE_SIZE);

    AU_TRY_DISCARD(dev->execute_single_time_commands([&](VkCommandBuffer cmd) {
      for (u32 i = 0; i < handles.size(); i++)
      {
        auto impl = reinterpret_cast<VulkanImage *>(handles[i]);
        if (!impl || !image_data[i])
          continue;

        if (ghi::is_compressed_format(impl->get_format_enum()))
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

          std::vector<VkBufferImageCopy> regions;
          u32 currentBufferOffset = 0;
          u32 mipW = impl->get_extent().width;
          u32 mipH = impl->get_extent().height;
          u32 blockSize = ghi::get_compressed_format_block_size(impl->get_format_enum());

          for (u32 layer = 0; layer < impl->get_layer_count(); layer++)
          {
            for (u32 m = 0; m < impl->get_mip_level_count(); m++)
            {
              VkBufferImageCopy region = {};
              region.bufferOffset = offsets[i] + currentBufferOffset;
              region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              region.imageSubresource.mipLevel = m;
              region.imageSubresource.baseArrayLayer = layer;
              region.imageSubresource.layerCount = 1;
              region.imageExtent = {mipW, mipH, 1};

              regions.push_back(region);

              u32 blocksX = (mipW + 3) / 4;
              u32 blocksY = (mipH + 3) / 4;
              u32 mipSize = blocksX * blocksY * blockSize;
              currentBufferOffset += mipSize;

              mipW = std::max(1u, mipW / 2);
              mipH = std::max(1u, mipH / 2);
            }
            mipW = impl->get_extent().width;
            mipH = impl->get_extent().height;
          }

          vkCmdCopyBufferToImage(cmd, stagingBuffer, impl->get_handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 static_cast<u32>(regions.size()), regions.data());

          insert_image_barrier(cmd, impl->get_handle(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, range);
        }
        else
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

            for (u32 j = 1; j < impl->get_mip_level_count(); j++)
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

    vmaDestroyBuffer(dev->m_allocator, stagingBuffer, stagingAlloc);

    return {};
  }

  auto VulkanBackend::create_samplers(Device device, Span<const SamplerDesc> descs, Sampler *out_handles)
      -> Result<void>
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < descs.size(); i++)
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

  auto VulkanBackend::destroy_samplers(Device device, Span<const Sampler> handles) -> void
  {
    const auto dev = reinterpret_cast<VulkanDevice *>(device);

    for (u32 i = 0; i < handles.size(); i++)
    {
      if (!handles[i])
        continue;

      const auto impl = reinterpret_cast<VkSampler>(handles[i]);

      vkDestroySampler(dev->get_handle(), impl, nullptr);
    }
  }
} // namespace ghi