#pragma once

#include <vector>

template <typename T, typename U = std::allocator<T>> using array = std::vector<T, U>;
#define ARR_COUNT(x) ((sizeof(x) / sizeof(*x)))

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

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

VkInstance create_vulkan_instance(const char *name, int version, array<const char *> extensions, array<const char *> layers);
void       destroy_vulkan_instance(VkInstance instance);

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *glfw_window);
void         destroy_surface(VkInstance instance, VkSurfaceKHR surface);

VkPhysicalDevice find_compatible_device(VkInstance instance, array<const char *> required_extensions);

VulkanDevice create_logical_device(VkSurfaceKHR surface, VkPhysicalDevice pdevice, array<const char *> extensions, array<const char *> layers);
void         destroy_device(VulkanDevice *ldevice);

VulkanSwapchain create_swapchain(VkSurfaceKHR surface, VkPhysicalDevice pdevice, VulkanDevice *ldevice, uint8_t image_count);
void            update_swapchain(VulkanSwapchain *sc, bool vsync);
void            destroy_swapchain(VulkanSwapchain *sc);
