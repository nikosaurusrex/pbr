#pragma once

#include <vector>

template <typename T, typename U = std::allocator<T>> using array = std::vector<T, U>;
#define ARR_COUNT(x) ((sizeof(x) / sizeof(*x)))

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

VkInstance create_vulkan_instance(const char *name, int version, array<const char *> extensions, array<const char *> layers);
void       destroy_vulkan_instance(VkInstance instance);

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *glfw_window);
void         destroy_surface(VkInstance instance, VkSurfaceKHR surface);

VkPhysicalDevice find_compatible_device(VkInstance instance, array<const char *> required_extensions);

VkDevice create_logical_device(VkPhysicalDevice pdevice, VkSurfaceKHR surface, array<const char *> extensions, array<const char *> layers);
void     destroy_device(VkDevice ldevice);
