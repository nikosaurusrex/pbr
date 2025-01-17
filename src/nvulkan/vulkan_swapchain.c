#include "nvulkan.h"

#include <stdio.h>
#include <stdlib.h>

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

Swapchain
swapchain_create(U32 image_count, VkSurfaceKHR surface, VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool)
{
  Swapchain swapchain = {0};

  swapchain.surface     = surface;
  swapchain.pdevice     = pdevice;
  swapchain.ldevice     = ldevice;
  swapchain.image_count = image_count;

  VkSurfaceCapabilitiesKHR surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &surface_capabilities);
  if (swapchain.image_count > surface_capabilities.maxImageCount) {
    swapchain.image_count = surface_capabilities.maxImageCount;
  }

  U32 format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &format_count, 0);

  VkSurfaceFormatKHR *formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
  vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &format_count, formats);

  for (U32 i = 0; i < format_count; ++i) {
    swapchain.format = formats[i];
    if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
      break;
    }
  }

  swapchain_update(&swapchain, cmd_pool, 1);

  // Create fences
  swapchain.fences = malloc(swapchain.image_count * sizeof(VkFence));
  for (U32 i = 0; i < swapchain.image_count; ++i) {
    VkFenceCreateInfo create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    create_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(ldevice->handle, &create_info, g_allocator, &swapchain.fences[i]));
  }

  free(formats);
  return swapchain;
}

void
swapchain_update(Swapchain *sc, VkCommandPool cmd_pool, B8 vsync)
{
  VkSurfaceKHR     surface       = sc->surface;
  VkSwapchainKHR   old_swapchain = sc->handle;
  VkPhysicalDevice pdevice       = sc->pdevice;
  Device          *ldevice       = sc->ldevice;

  VkSurfaceCapabilitiesKHR surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &surface_capabilities);

  Assert(surface_capabilities.currentExtent.width != (U32)(-1));
  VkExtent2D extent = surface_capabilities.currentExtent;
  sc->width         = extent.width;
  sc->height        = extent.height;

  // Select present mode
  U32 present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &present_mode_count, 0);

  VkPresentModeKHR *present_modes = malloc(present_mode_count * sizeof(VkPresentModeKHR));
  vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &present_mode_count, present_modes);

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  if (!vsync) {
    for (U32 i = 0; i < present_mode_count; ++i) {
      if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
        present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
      }
      if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
      }
    }
  }

  free(present_modes);

  VkSurfaceTransformFlagBitsKHR transform;
  if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  } else {
    transform = surface_capabilities.currentTransform;
  }

  // Create actual swapchain
  VkSwapchainCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  create_info.surface                  = sc->surface;
  create_info.minImageCount            = sc->image_count;
  create_info.imageFormat              = sc->format.format;
  create_info.imageColorSpace          = sc->format.colorSpace;
  create_info.imageExtent              = extent;
  create_info.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
  create_info.imageArrayLayers         = 1;
  create_info.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  create_info.queueFamilyIndexCount    = 1;
  create_info.pQueueFamilyIndices      = &sc->ldevice->graphics_queue_index;
  create_info.preTransform             = transform;
  create_info.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode              = present_mode;
  create_info.clipped                  = VK_TRUE;
  create_info.oldSwapchain             = old_swapchain;

  VK_CHECK(vkCreateSwapchainKHR(ldevice->handle, &create_info, g_allocator, &sc->handle));

  // Cleanup old swapchain if there is one (not first creation)
  if (old_swapchain) {
    for (U32 i = 0; i < sc->image_count; ++i) {
      vkDestroyImageView(ldevice->handle, sc->image_views[i], g_allocator);
    }

    for (U32 i = 0; i < sc->image_count + 1; ++i) {
      vkDestroySemaphore(ldevice->handle, sc->read_semaphores[i], g_allocator);
      vkDestroySemaphore(ldevice->handle, sc->write_semaphores[i], g_allocator);
    }

    vkDestroySwapchainKHR(ldevice->handle, old_swapchain, g_allocator);
  }

  // Setup images, image views and barriers
  VK_CHECK(vkGetSwapchainImagesKHR(ldevice->handle, sc->handle, &sc->image_count, 0));

  sc->images      = realloc(sc->images, sc->image_count * sizeof(VkImage));
  sc->image_views = realloc(sc->image_views, sc->image_count * sizeof(VkImageView));
  sc->barriers    = realloc(sc->barriers, sc->image_count * sizeof(VkImageMemoryBarrier));

  VK_CHECK(vkGetSwapchainImagesKHR(ldevice->handle, sc->handle, &sc->image_count, sc->images));

  for (U32 i = 0; i < sc->image_count; ++i) {
    // image view
    VkImageViewCreateInfo create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    create_info.image                 = sc->images[i];
    create_info.viewType              = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format                = sc->format.format;
    create_info.components =
        (VkComponentMapping){VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    create_info.subresourceRange = (VkImageSubresourceRange){VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VK_CHECK(vkCreateImageView(ldevice->handle, &create_info, g_allocator, &sc->image_views[i]));

    // image memory barrier
    VkImageSubresourceRange range = {0};
    range.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel            = 0;
    range.levelCount              = VK_REMAINING_MIP_LEVELS;
    range.baseArrayLayer          = 0;
    range.layerCount              = VK_REMAINING_ARRAY_LAYERS;

    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.dstAccessMask        = 0;
    barrier.srcAccessMask        = 0;
    barrier.oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout            = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.image                = sc->images[i];
    barrier.subresourceRange     = range;

    sc->barriers[i] = barrier;
  }

  // Create semaphores
  sc->semaphore_count  = sc->image_count + 1;
  sc->read_semaphores  = realloc(sc->read_semaphores, sc->semaphore_count * sizeof(VkSemaphore));
  sc->write_semaphores = realloc(sc->write_semaphores, sc->semaphore_count * sizeof(VkSemaphore));

  for (U32 i = 0; i < sc->semaphore_count; ++i) {
    VkSemaphoreCreateInfo create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    VK_CHECK(vkCreateSemaphore(ldevice->handle, &create_info, g_allocator, &sc->read_semaphores[i]));
    VK_CHECK(vkCreateSemaphore(ldevice->handle, &create_info, g_allocator, &sc->write_semaphores[i]));
  }

  VkCommandBuffer cmd_buf = command_buffer_allocate(ldevice, cmd_pool);
  command_buffer_begin(cmd_buf);
  vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, 0, 0, 0, sc->image_count,
                       sc->barriers);
  command_buffer_submit(cmd_buf, ldevice);
  command_buffer_free(cmd_buf, ldevice, cmd_pool);
}

void
swapchain_destroy(Swapchain *sc)
{
  Device *ldevice = sc->ldevice;

  for (U32 i = 0; i < sc->image_count; ++i) {
    vkDestroyImageView(ldevice->handle, sc->image_views[i], g_allocator);
    vkDestroyFence(ldevice->handle, sc->fences[i], g_allocator);
  }
  free(sc->images);
  free(sc->image_views);
  free(sc->fences);
  free(sc->barriers);

  for (U32 i = 0; i < sc->image_count + 1; ++i) {
    vkDestroySemaphore(ldevice->handle, sc->read_semaphores[i], g_allocator);
    vkDestroySemaphore(ldevice->handle, sc->write_semaphores[i], g_allocator);
  }
  free(sc->read_semaphores);
  free(sc->write_semaphores);

  vkDestroySwapchainKHR(ldevice->handle, sc->handle, g_allocator);
}

U32
swapchain_acquire(Swapchain *sc)
{
  // @Todo check for resize and destroy and recreate swapchain
  VkDevice ldevice = sc->ldevice->handle;

  VkSemaphore sempahore = sc->read_semaphores[sc->current_semaphore];

  VkResult result = vkAcquireNextImageKHR(ldevice, sc->handle, UINT64_MAX, sempahore, 0, &sc->current_image);

  if (result != VK_SUCCESS) {
    // @Todo handle resize
  }

  do {
    result = vkWaitForFences(ldevice, 1, &sc->fences[sc->current_image], VK_TRUE, UINT64_MAX);
  } while (result != VK_SUCCESS);

  return sc->current_image;
}

void
swapchain_present(Swapchain *sc, CommandBuffers *cmd_bufs)
{
  Device *ldevice       = sc->ldevice;
  U32     current_image = sc->current_image;

  // @Todo move this
  vkResetFences(ldevice->handle, 1, &sc->fences[current_image]);

  VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo         submitInfo      = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.pWaitDstStageMask         = &wait_stage_mask;
  submitInfo.pWaitSemaphores           = &sc->read_semaphores[sc->current_semaphore];
  submitInfo.waitSemaphoreCount        = 1;
  submitInfo.pSignalSemaphores         = &sc->write_semaphores[sc->current_semaphore];
  submitInfo.signalSemaphoreCount      = 1;
  submitInfo.pCommandBuffers           = &cmd_bufs->handles[current_image];
  submitInfo.commandBufferCount        = 1;
  submitInfo.pNext                     = 0;

  vkQueueSubmit(ldevice->graphics_queue, 1, &submitInfo, sc->fences[current_image]);

  VkPresentInfoKHR present_info   = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  present_info.swapchainCount     = 1;
  present_info.pSwapchains        = &sc->handle;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores    = &sc->write_semaphores[sc->current_semaphore];
  present_info.pImageIndices      = &sc->current_image;

  vkQueuePresentKHR(ldevice->graphics_queue, &present_info);

  sc->current_semaphore = (sc->current_semaphore + 1) % sc->semaphore_count;
}
