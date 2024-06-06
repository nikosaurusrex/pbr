#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

template <typename T, typename U = std::allocator<T>> using array = std::vector<T, U>;
#define ARR_COUNT(x) ((sizeof(x) / sizeof(*x)))

#define VK_CHECK(call)                                                                                                                                                                                 \
    if (call != VK_SUCCESS) {                                                                                                                                                                          \
        fprintf(stderr, "Vulkan call failed at %s:%d\n", __FILE__, __LINE__);                                                                                                                          \
        exit(1);                                                                                                                                                                                       \
    }

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
    uint8_t            image_count;
    VkSurfaceFormatKHR format;
};

VkInstance vulkan_instance_create(const char *name, int version, array<const char *> extensions, array<const char *> layers);
void       vulkan_instance_destroy(VkInstance instance);

VkSurfaceKHR surface_create(VkInstance instance, GLFWwindow *glfw_window);
void         surface_destroy(VkInstance instance, VkSurfaceKHR surface);

VkPhysicalDevice physical_device_find_compatible(VkInstance instance, array<const char *> required_extensions);

VulkanDevice logical_device_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, array<const char *> extensions, array<const char *> layers);
void         logical_device_destroy(VulkanDevice *ldevice);

VulkanSwapchain swapchain_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, VulkanDevice *ldevice, uint8_t image_count);
void            swapchain_update(VulkanSwapchain *sc, bool vsync);
void            swapchain_destroy(VulkanSwapchain *sc);
