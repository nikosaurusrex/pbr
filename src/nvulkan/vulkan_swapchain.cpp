#include "nvulkan.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VulkanSwapchain
swapchain_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, VulkanDevice *ldevice, uint8_t image_count)
{
    VulkanSwapchain swapchain = {};

    swapchain.surface     = surface;
    swapchain.pdevice     = pdevice;
    swapchain.ldevice     = ldevice;
    swapchain.image_count = image_count;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &surface_capabilities);
    if (swapchain.image_count > surface_capabilities.maxImageCount) {
        swapchain.image_count = surface_capabilities.maxImageCount;
    }

    uint32_t formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formats_count, 0);

    array<VkSurfaceFormatKHR> formats;
    formats.resize(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formats_count, formats.data());

    for (uint32_t i = 0; i < formats_count; ++i) {
        swapchain.format = formats[i];
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
            break;
        }
    }

    swapchain_update(&swapchain, true);

    return swapchain;
}

void
swapchain_update(VulkanSwapchain *sc, bool vsync)
{
    VkSurfaceKHR     surface       = sc->surface;
    VkSwapchainKHR   old_swapchain = sc->handle;
    VkPhysicalDevice pdevice       = sc->pdevice;
    VulkanDevice    *ldevice       = sc->ldevice;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &surface_capabilities);

    assert(surface_capabilities.currentExtent.width != (uint32_t)(-1));
    VkExtent2D extent = surface_capabilities.currentExtent;

    uint32_t present_modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &present_modes_count, 0);
    array<VkPresentModeKHR> present_modes;
    present_modes.resize(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &present_modes_count, present_modes.data());

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    if (!vsync) {
        for (uint32_t i = 0; i < present_modes_count; ++i) {
            if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    VkSurfaceTransformFlagBitsKHR transform;
    if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        transform = surface_capabilities.currentTransform;
    }

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

    VkResult result = vkCreateSwapchainKHR(ldevice->handle, &create_info, g_allocator, &sc->handle);
}

void
swapchain_destroy(VulkanSwapchain *sc)
{
}
