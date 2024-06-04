#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

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
main(int, char **)
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

    create_

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
