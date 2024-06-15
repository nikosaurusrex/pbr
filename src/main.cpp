#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "imgui.h"

#include <GLFW/glfw3.h>

#include "gui/vulkan_imgui.h"
#include "models/models.h"
#include "nvulkan/nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

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

struct Postprocess {
    DescriptorSet desc_set;
    Pipeline      pipeline;
};

Postprocess
postprocess_create(Device *ldevice, VkRenderPass render_pass, Texture *target_texture)
{
    Postprocess p = {};

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, 0};
    p.desc_set                           = descriptor_set_create(ldevice, &binding, 1);

    Shader shaders[] = {
        {"assets/shaders/pass.spv", VK_SHADER_STAGE_VERTEX_BIT},
        {"assets/shaders/post.spv", VK_SHADER_STAGE_FRAGMENT_BIT},
    };

    p.pipeline = pipeline_create(ldevice, &p.desc_set, render_pass, shaders, ARR_COUNT(shaders), 0, 0, 0, 0, VK_CULL_MODE_NONE);

    VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    desc_write.dstSet               = p.desc_set.handle;
    desc_write.dstBinding           = 0;
    desc_write.descriptorCount      = 1;
    desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_write.pImageInfo           = &target_texture->descriptor;

    vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);

    return p;
}

void
postprocess_destroy(Device *ldevice, Postprocess *p)
{
    pipeline_destroy(ldevice, &p->pipeline);
    descriptor_set_destroy(ldevice, &p->desc_set);
}

struct Camera {
};

struct ResizeInfo {
    Swapchain     *sc;
    VkCommandPool  cmd_pool;
    Image         *depth_buffer;
    Framebuffers  *frame_buffers;
    VkRenderPass   render_pass;
    SceneRenderer *scene_renderer;
    Postprocess   *postprocess;
};

void
resize_callback(GLFWwindow *window, int width, int height)
{
    ResizeInfo *info = (ResizeInfo *)glfwGetWindowUserPointer(window);

    Swapchain       *sc      = info->sc;
    Device          *ldevice = info->sc->ldevice;
    VkPhysicalDevice pdevice = sc->pdevice;

    vkDeviceWaitIdle(ldevice->handle);
    vkQueueWaitIdle(ldevice->graphics_queue);

    // Recreate swapchain
    swapchain_update(info->sc, info->cmd_pool, 0);

    // Update imgui
    ImGui::GetIO().DisplaySize = ImVec2(width, height);

    // Recreate depth buffer, framebuffers, renderer and postprocess
    image_destroy(ldevice, info->depth_buffer);
    *info->depth_buffer = image_create_depth(pdevice, ldevice, sc, info->cmd_pool);

    frame_buffers_destroy(ldevice, info->frame_buffers);
    *info->frame_buffers = frame_buffers_create(sc, info->render_pass, info->depth_buffer);

    scene_renderer_destroy(ldevice, info->scene_renderer);
    *info->scene_renderer = scene_renderer_create(pdevice, ldevice, sc, info->cmd_pool, info->depth_buffer->format);

    VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    desc_write.dstSet               = info->postprocess->desc_set.handle;
    desc_write.dstBinding           = 0;
    desc_write.descriptorCount      = 1;
    desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_write.pImageInfo           = &info->scene_renderer->color_image.descriptor;

    vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);
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

    uint32_t     req_extension_count;
    const char **req_extensions = glfwGetRequiredInstanceExtensions(&req_extension_count);

    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkInstance instance =
        vulkan_instance_create("Raytracer", VK_API_VERSION_1_2, req_extensions, req_extension_count, layers, ARR_COUNT(layers));

    VkSurfaceKHR surface = surface_create(instance, window);

    const char      *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDevice pdevice             = physical_device_find_compatible(instance, device_extensions, ARR_COUNT(device_extensions));
    if (!pdevice) {
        log_fatal("Failed to find compatible GPU device!");
    }

    Device ldevice = logical_device_create(surface, pdevice, device_extensions, ARR_COUNT(device_extensions), layers, ARR_COUNT(layers));

    VkCommandPool cmd_pool = command_pool_create(&ldevice);

    Swapchain swapchain = swapchain_create(surface, pdevice, &ldevice, cmd_pool, 2);

    Image          depth_image         = image_create_depth(pdevice, &ldevice, &swapchain, cmd_pool);
    VkRenderPass   present_render_pass = render_pass_create_present(&ldevice, swapchain.format.format, depth_image.format);
    Framebuffers   frame_buffers       = frame_buffers_create(&swapchain, present_render_pass, &depth_image);
    CommandBuffers cmd_bufs            = command_buffers_allocate(&ldevice, cmd_pool, swapchain.image_count);

    SceneRenderer scene_renderer = scene_renderer_create(pdevice, &ldevice, &swapchain, cmd_pool, depth_image.format);
    Postprocess   postprocess    = postprocess_create(&ldevice, present_render_pass, &scene_renderer.color_image);

    Model model = {};
    model_load(pdevice, &ldevice, cmd_pool, &model, "assets/models/cube.obj");

    VkDescriptorPool imgui_desc_pool = gui_init(window, instance, pdevice, &ldevice, swapchain.image_count, present_render_pass, cmd_pool);

    ResizeInfo resize_info     = {0};
    resize_info.cmd_pool       = cmd_pool;
    resize_info.sc             = &swapchain;
    resize_info.depth_buffer   = &depth_image;
    resize_info.frame_buffers  = &frame_buffers;
    resize_info.render_pass    = present_render_pass;
    resize_info.scene_renderer = &scene_renderer;
    resize_info.postprocess    = &postprocess;

    glfwSetWindowUserPointer(window, &resize_info);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        gui_new_frame();
        gui_render();

        uint32_t        current_image = swapchain_acquire(&swapchain);
        VkCommandBuffer cmd_buf       = cmd_bufs.handles[current_image];
        command_buffer_begin(cmd_buf);

        VkClearValue clear_colors[2] = {0};
        clear_colors[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_colors[1].depthStencil = {1.0f, 0};

        // Scene
        scene_renderer_render(&swapchain, cmd_buf, &scene_renderer, &model, 1, clear_colors);

        // Render UI
        {
            VkRenderPassBeginInfo ui_render_pass = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            ui_render_pass.clearValueCount       = 2;
            ui_render_pass.pClearValues          = clear_colors;
            ui_render_pass.renderPass            = present_render_pass;
            ui_render_pass.framebuffer           = frame_buffers.handles[current_image];
            ui_render_pass.renderArea            = {{0, 0}, {swapchain.width, swapchain.height}};

            vkCmdBeginRenderPass(cmd_buf, &ui_render_pass, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{0.0f, 0.0f, swapchain.width, swapchain.height, 0.0f, 1.0f};
            vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

            VkRect2D scissor{{0, 0}, {swapchain.width, swapchain.height}};
            vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

            vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, postprocess.pipeline.handle);
            vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, postprocess.pipeline.layout, 0, 1,
                                    &postprocess.desc_set.handle, 0, 0);

            vkCmdDraw(cmd_buf, 3, 1, 0, 0);

            gui_end_frame(cmd_buf);
            vkCmdEndRenderPass(cmd_buf);
        }

        command_buffer_end(cmd_buf);
        swapchain_present(&swapchain, &cmd_bufs);
    }

    vkDeviceWaitIdle(ldevice.handle);

    vkDestroyDescriptorPool(ldevice.handle, imgui_desc_pool, 0);

    model_free(&ldevice, &model);
    postprocess_destroy(&ldevice, &postprocess);
    scene_renderer_destroy(&ldevice, &scene_renderer);
    command_buffers_free(&ldevice, cmd_pool, &cmd_bufs);
    frame_buffers_destroy(&ldevice, &frame_buffers);
    render_pass_destroy(&ldevice, present_render_pass);
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
