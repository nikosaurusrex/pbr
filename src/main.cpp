#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

#include "vulkan_renderer.h"

void
log_fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "[Fatal] ");

    vfprintf(stderr, fmt, args);
    putc('\n', stderr);
    va_end(args);

    exit(1);
}

void
log_dev(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stdout, "[Dev] ");

    vfprintf(stdout, fmt, args);
    putc('\n', stdout);
    va_end(args);
}

static void
check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

static void
glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int
main(int argc, char *argv[])
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        log_fatal("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "raytracer", 0, 0);
    if (!window) {
        log_fatal("Failed to create window!");
    }

    uint32_t            req_extensions_count;
    const char        **req_extensions = glfwGetRequiredInstanceExtensions(&req_extensions_count);
    array<const char *> extensions     = {};
    for (uint32_t i = 0; i < req_extensions_count; ++i) {
        extensions.push_back(req_extensions[i]);
    }

    array<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

    VkInstance   instance = create_vulkan_instance("Raytracer", VK_API_VERSION_1_2, extensions, layers);
    VkSurfaceKHR surface  = create_surface(instance, window);

    array<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDevice    pdevice           = find_compatible_device(instance, device_extensions);
    if (!pdevice) {
        log_fatal("Failed to find compatible GPU device!");
    }

    VkDevice ldevice = create_logical_device(pdevice, surface, device_extensions, layers);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    destroy_device(ldevice);
    destroy_surface(instance, surface);
    destroy_vulkan_instance(instance);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
