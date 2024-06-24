#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkSurfaceKHR
surface_create(VkInstance instance, GLFWwindow *glfw_window)
{
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, glfw_window, g_allocator, &surface);
    return surface;
}

void
surface_destroy(VkSurfaceKHR surface, VkInstance instance)
{
    vkDestroySurfaceKHR(instance, surface, g_allocator);
}
