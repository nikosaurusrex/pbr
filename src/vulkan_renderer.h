#pragma once

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include "data_structures.h"

VkInstance create_vulkan_instance(const char *name, int version, Array<const char *> extensions, Array<const char *> layers);
void       destroy_vulkan_instance(VkInstance instance);

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *glfw_window);
void         destroy_surface(VkInstance instance, VkSurfaceKHR surface);

VkPhysicalDevice find_compatible_device(VkInstance instance, Array<const char *> required_extensions);

VkDevice create_logical_device(VkPhysicalDevice pdevice, VkSurfaceKHR surface, Array<const char *> extensions, Array<const char *> layers);
void     destroy_device(VkDevice ldevice);
