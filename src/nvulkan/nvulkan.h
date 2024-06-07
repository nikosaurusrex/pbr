#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#define ARR_COUNT(x) ((sizeof(x) / sizeof(*x)))

#define VK_CHECK(call)                                                                                                                     \
    if (call != VK_SUCCESS) {                                                                                                              \
        fprintf(stderr, "Vulkan call failed at %s:%d\n", __FILE__, __LINE__);                                                              \
        exit(1);                                                                                                                           \
    }

#ifndef __cplusplus
typedef struct VulkanDevice    VulkanDevice;
typedef struct VulkanSwapchain VulkanSwapchain;
#else
extern "C" {
#endif

struct VulkanDevice {
    VkDevice handle;
    VkQueue  graphics_queue;
    uint32_t graphics_queue_index;
};

struct VulkanSwapchain {
    VkSwapchainKHR     handle;
    VkSurfaceKHR       surface;
    VulkanDevice      *ldevice;
    VkPhysicalDevice   pdevice;
    VkSurfaceFormatKHR format;

    uint32_t image_count;
    uint32_t semaphore_count;
    uint32_t fence_count;

    VkImage              *images;
    VkImageView          *image_views;
    VkImageMemoryBarrier *barriers;
    VkSemaphore          *read_semaphores;
    VkSemaphore          *write_semaphores;
    VkFence              *fences;
};

VkInstance vulkan_instance_create(const char *name, int version, const char **extensions, uint32_t extension_count, const char **layers,
                                  uint32_t layer_count);
void       vulkan_instance_destroy(VkInstance instance);

VkSurfaceKHR surface_create(VkInstance instance, GLFWwindow *glfw_window);
void         surface_destroy(VkInstance instance, VkSurfaceKHR surface);

VkPhysicalDevice physical_device_find_compatible(VkInstance instance, const char **required_extensions, uint32_t required_extension_count);

VulkanDevice logical_device_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, const char **extensions, uint32_t extension_count,
                                   const char **layers, uint32_t layer_count);
void         logical_device_destroy(VulkanDevice *ldevice);

VulkanSwapchain swapchain_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, VulkanDevice *ldevice, uint32_t image_count);
void            swapchain_update(VulkanSwapchain *sc, uint8_t vsync);
void            swapchain_destroy(VulkanSwapchain *sc);

#ifdef __cplusplus
}
#endif
