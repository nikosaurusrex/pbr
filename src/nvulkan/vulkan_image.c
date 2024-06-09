#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

static uint32_t
memory_type_find(VkPhysicalDevice pdevice, uint32_t type_bits, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(pdevice, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if ((type_bits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

    // @Todo error handling
    return 0;
}

VulkanImage
image_create(VkPhysicalDevice pdevice, VulkanDevice *ldevice, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels,
             VkImageAspectFlags aspect_mask, VkImageUsageFlags usage)
{
    // Create image handle
    VulkanImage image = {0};
    image.format      = format;

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
image_destroy(VulkanDevice *ldevice, VulkanImage *image)
{
    vkDestroyImageView(ldevice->handle, image->view, g_allocator);
    vkDestroyImage(ldevice->handle, image->handle, g_allocator);
    vkFreeMemory(ldevice->handle, image->memory, g_allocator);
}
