#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

#include "nvulkan/nvulkan.h"

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

static VulkanImage
depth_buffer_create(VkPhysicalDevice pdevice, VulkanDevice *ldevice, VulkanSwapchain *sc, VkCommandPool cmd_pool)
{
    // Create depth buffer
    VkImageAspectFlags depth_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    VulkanImage        depth_image  = image_create(pdevice, ldevice, VK_FORMAT_D24_UNORM_S8_UINT, sc->width, sc->height, 1, depth_aspect,
                                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    VkCommandBuffer    cmd_buf      = command_buffer_allocate(ldevice, cmd_pool);
    command_buffer_begin(cmd_buf);

    VkImageSubresourceRange subresource_range = {0};
    subresource_range.aspectMask              = depth_aspect;
    subresource_range.levelCount              = 1;
    subresource_range.layerCount              = 1;

    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout            = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrier.image                = depth_image.handle;
    barrier.subresourceRange     = subresource_range;
    barrier.srcAccessMask        = VkAccessFlags();
    barrier.dstAccessMask        = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, 0, 0, 0, 1,
                         &barrier);
    command_buffer_submit(ldevice, cmd_buf);
    command_buffer_free(ldevice, cmd_pool, cmd_buf);

    return depth_image;
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

    uint32_t                  req_extension_count;
    const char              **req_extension = glfwGetRequiredInstanceExtensions(&req_extension_count);
    std::vector<const char *> extensions    = {};
    for (uint32_t i = 0; i < req_extension_count; ++i) {
        extensions.push_back(req_extension[i]);
    }

    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkInstance instance =
        vulkan_instance_create("Raytracer", VK_API_VERSION_1_2, extensions.data(), extensions.size(), layers, ARR_COUNT(layers));
    VkSurfaceKHR surface = surface_create(instance, window);

    const char      *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDevice pdevice             = physical_device_find_compatible(instance, device_extensions, ARR_COUNT(device_extensions));
    if (!pdevice) {
        log_fatal("Failed to find compatible GPU device!");
    }

    VulkanDevice ldevice =
        logical_device_create(surface, pdevice, device_extensions, ARR_COUNT(device_extensions), layers, ARR_COUNT(layers));

    VkCommandPool cmd_pool = command_pool_create(&ldevice);

    VulkanSwapchain swapchain = swapchain_create(surface, pdevice, &ldevice, cmd_pool, 2);

    VulkanImage depth_image = depth_buffer_create(pdevice, &ldevice, &swapchain, cmd_pool);
    VkRenderPass render_pass = render_pass_create(&ldevice, swapchain.format.format, depth_image.format);
    VulkanFramebuffers frame_buffers = frame_buffers_create(&swapchain, render_pass, &depth_image);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDeviceWaitIdle(ldevice.handle);

    frame_buffers_destroy(&ldevice, &frame_buffers);
    render_pass_destroy(&ldevice, render_pass);
    image_destroy(&ldevice, &depth_image);
    swapchain_destroy(&swapchain);
    command_pool_destroy(&ldevice, cmd_pool);
    logical_device_destroy(&ldevice);
    surface_destroy(instance, surface);
    vulkan_instance_destroy(instance);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
