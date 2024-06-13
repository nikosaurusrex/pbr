#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

Image
image_create(VkPhysicalDevice pdevice, Device *ldevice, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels,
             VkImageAspectFlags aspect_mask, VkImageUsageFlags usage)
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
image_destroy(Device *ldevice, Image *image)
{
    vkDestroyImageView(ldevice->handle, image->view, g_allocator);
    vkDestroyImage(ldevice->handle, image->handle, g_allocator);
    vkFreeMemory(ldevice->handle, image->memory, g_allocator);
}

Texture
texture_create(VkPhysicalDevice pdevice, Device *ldevice, VkFormat format, uint32_t width, uint32_t height, VkImageAspectFlags aspect_mask,
               VkImageUsageFlags usage)
{
    Texture t = {0};

    t.image = image_create(pdevice, ldevice, format, width, height, 1, aspect_mask, usage);

    VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    vkCreateSampler(ldevice->handle, &sampler_info, g_allocator, &t.descriptor.sampler);

    return t;
}

void
texture_destroy(Device *ldevice, Texture *t)
{
    vkDestroySampler(ldevice->handle, t->descriptor.sampler, g_allocator);
    image_destroy(ldevice, &t->image);
}
