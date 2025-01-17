#include "nvulkan.h"

#include <math.h>

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

static U32
get_mip_levels(U32 width, U32 height)
{
  return (U32)(floorf(log2f((F32)Max(width, height)))) + 1;
}

Image
image_create(U32 width, U32 height, VkFormat format, U32 mip_levels, VkImageAspectFlags aspect_mask, VkImageUsageFlags usage,
             VkPhysicalDevice pdevice, Device *ldevice)
{
  // Create image handle
  Image image  = {0};
  image.format = format;

  VkImageCreateInfo image_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_info.imageType         = VK_IMAGE_TYPE_2D;
  image_info.format            = format;
  image_info.extent.width      = width;
  image_info.extent.height     = height;
  image_info.extent.depth      = 1;
  image_info.mipLevels         = mip_levels;
  image_info.arrayLayers       = 1;
  image_info.samples           = VK_SAMPLE_COUNT_1_BIT;
  image_info.tiling            = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage             = usage;
  image_info.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
  image_info.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;

  VK_CHECK(vkCreateImage(ldevice->handle, &image_info, g_allocator, &image.handle));

  // Allocate memory

  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(ldevice->handle, image.handle, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  alloc_info.allocationSize       = mem_reqs.size;
  alloc_info.memoryTypeIndex      = memory_type_find(pdevice, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(ldevice->handle, &alloc_info, g_allocator, &image.memory));
  VK_CHECK(vkBindImageMemory(ldevice->handle, image.handle, image.memory, 0));

  // Create image view

  VkImageViewCreateInfo view_info           = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_info.image                           = image.handle;
  view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format                          = format;
  view_info.subresourceRange.aspectMask     = aspect_mask;
  view_info.subresourceRange.baseMipLevel   = 0;
  view_info.subresourceRange.levelCount     = mip_levels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount     = 1;

  VK_CHECK(vkCreateImageView(ldevice->handle, &view_info, g_allocator, &image.view));

  return image;
}

void
image_destroy(Image *image, Device *ldevice)
{
  vkDestroyImageView(ldevice->handle, image->view, g_allocator);
  vkDestroyImage(ldevice->handle, image->handle, g_allocator);
  vkFreeMemory(ldevice->handle, image->memory, g_allocator);
}

Texture
texture_create(U32 width, U32 height, VkFormat format, VkImageAspectFlags aspect_mask, VkImageUsageFlags usage, VkPhysicalDevice pdevice,
               Device *ldevice)
{
  Texture t = {0};

  t.image = image_create(width, height, format, 1, aspect_mask, usage, pdevice, ldevice);

  VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  vkCreateSampler(ldevice->handle, &sampler_info, g_allocator, &t.descriptor.sampler);

  t.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  t.descriptor.imageView   = t.image.view;

  return t;
}

Texture
texture_from_pixels(U32 width, U32 height, U32 channels, VkFormat format, U8 *pixels, VkSamplerCreateInfo sampler_info,
                    VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool)
{
  Texture t = {0};

  Image image = image_create(width, height, format, get_mip_levels(width, height), VK_IMAGE_ASPECT_COLOR_BIT,
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, pdevice, ldevice);

  // Copy pixels to image
  VkDeviceSize size           = width * height * channels;
  Buffer       staging_buffer = buffer_create_staging(size, pixels, pdevice, ldevice);

  VkCommandBuffer cmd_buf = command_buffer_allocate(ldevice, cmd_pool);
  command_buffer_begin(cmd_buf);

  image_transition_layout(&image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd_buf);

  VkBufferImageCopy region           = {0};
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.layerCount = 1;
  region.imageExtent.width           = width;
  region.imageExtent.height          = height;
  region.imageExtent.depth           = 1;
  region.bufferOffset                = 0;
  region.bufferRowLength             = 0;
  region.bufferImageHeight           = 0;
  region.imageExtent.width           = width;
  region.imageExtent.height          = height;
  region.imageExtent.depth           = 1;
  region.imageOffset                 = (VkOffset3D){0, 0, 0};

  vkCmdCopyBufferToImage(cmd_buf, staging_buffer.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  image_transition_layout(&image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
                          cmd_buf);

  command_buffer_submit(cmd_buf, ldevice);
  command_buffer_free(cmd_buf, ldevice, cmd_pool);

  buffer_destroy(&staging_buffer, ldevice);

  t.image                  = image;
  t.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  t.descriptor.imageView   = image.view;

  vkCreateSampler(ldevice->handle, &sampler_info, g_allocator, &t.descriptor.sampler);

  return t;
}

void
texture_destroy(Texture *t, Device *ldevice)
{
  vkDestroySampler(ldevice->handle, t->descriptor.sampler, g_allocator);
  image_destroy(&t->image, ldevice);
}

void
image_transition_layout(Image *image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask,
                        VkCommandBuffer cmd_buf)
{
  VkImageSubresourceRange subresource_range = {aspect_mask, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout            = old_layout;
  barrier.newLayout            = new_layout;
  barrier.image                = image->handle;
  barrier.subresourceRange     = subresource_range;
  barrier.srcAccessMask        = 0;
  barrier.dstAccessMask        = 0;

  VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  vkCmdPipelineBarrier(cmd_buf, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &barrier);
}

Image
image_create_depth(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool)
{
  // Create depth buffer
  VkImageAspectFlags depth_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  Image              depth_image  = image_create(sc->width, sc->height, VK_FORMAT_D24_UNORM_S8_UINT, 1, depth_aspect,
                                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, pdevice, ldevice);
  VkCommandBuffer    cmd_buf      = command_buffer_allocate(ldevice, cmd_pool);
  command_buffer_begin(cmd_buf);

  VkImageSubresourceRange subresource_range = {0};
  subresource_range.aspectMask              = depth_aspect;
  subresource_range.levelCount              = 1;
  subresource_range.layerCount              = 1;

  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout            = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  barrier.image                = depth_image.handle;
  barrier.subresourceRange     = subresource_range;
  barrier.srcAccessMask        = 0;
  barrier.dstAccessMask        = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, 0, 0, 0, 1, &barrier);
  command_buffer_submit(cmd_buf, ldevice);
  command_buffer_free(cmd_buf, ldevice, cmd_pool);

  return depth_image;
}
