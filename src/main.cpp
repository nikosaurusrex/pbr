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

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

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

static Image
depth_buffer_create(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool)
{
    // Create depth buffer
    VkImageAspectFlags depth_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    Image        depth_image  = image_create(pdevice, ldevice, VK_FORMAT_D24_UNORM_S8_UINT, sc->width, sc->height, 1, depth_aspect,
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

static VkDescriptorPool
init_gui(GLFWwindow *window, VkInstance instance, VkPhysicalDevice pdevice, Device *ldevice, uint32_t image_count,
         VkRenderPass render_pass, VkCommandPool cmd_pool)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    VkDescriptorPool           imgui_desc_pool;
    VkDescriptorPoolSize       pool_size[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
    VkDescriptorPoolCreateInfo pool_info   = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags                        = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets                      = 1000;
    pool_info.poolSizeCount                = 2;
    pool_info.pPoolSizes                   = pool_size;
    vkCreateDescriptorPool(ldevice->handle, &pool_info, 0, &imgui_desc_pool);

    // Setup Platform/Renderer back end
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = instance;
    init_info.PhysicalDevice            = pdevice;
    init_info.Device                    = ldevice->handle;
    init_info.QueueFamily               = ldevice->graphics_queue_index;
    init_info.Queue                     = ldevice->graphics_queue;
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.DescriptorPool            = imgui_desc_pool;
    init_info.Subpass                   = 0;
    init_info.MinImageCount             = 2;
    init_info.ImageCount                = image_count;
    init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn           = 0;
    init_info.Allocator                 = 0;
    init_info.UseDynamicRendering       = VK_FALSE;

    ImGui_ImplVulkan_Init(&init_info, render_pass);

    VkCommandBuffer command_buffer = command_buffer_allocate(ldevice, cmd_pool);
    command_buffer_begin(command_buffer);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    command_buffer_submit(ldevice, command_buffer);
    command_buffer_free(ldevice, cmd_pool, command_buffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    return imgui_desc_pool;
}

struct Texture {
    Image           image;
    VkDescriptorImageInfo descriptor;
};

Texture
texture_create(VkPhysicalDevice pdevice, Device *ldevice, VkFormat format, uint32_t width, uint32_t height,
               VkImageAspectFlags aspect_mask, VkImageUsageFlags usage)
{
    Texture t = {0};

    t.image = image_create(pdevice, ldevice, format, width, height, 1, aspect_mask, usage);

    VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    vkCreateSampler(ldevice->handle, &sampler_info, g_allocator, &t.descriptor.sampler);

    return t;
}

void
texture_destroy(Device *ldevice, Texture *t)
{
    vkDestroySampler(ldevice->handle, t->descriptor.sampler, g_allocator);
    image_destroy(ldevice, &t->image);
}

struct Renderer {
    Texture       color_image;
    Texture       depth_image;
    VkRenderPass  render_pass;
    VkFramebuffer framebuffer;
};

Renderer
renderer_create(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkFormat depth_format)
{
    Renderer r = {};

    VkFormat color_format = sc->format.format;
    uint32_t width        = sc->width;
    uint32_t height       = sc->height;

    r.color_image                        = texture_create(pdevice, ldevice, color_format, width, height, VK_IMAGE_ASPECT_COLOR_BIT,
                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    r.color_image.descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    r.depth_image = texture_create(pdevice, ldevice, depth_format, width, height, VK_IMAGE_ASPECT_DEPTH_BIT,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    r.render_pass = render_pass_create(ldevice, r.color_image.image.format, r.depth_image.image.format);

    r.framebuffer = frame_buffer_create(sc, r.render_pass, r.color_image.image.view, r.depth_image.image.view);

    descriptor_set_create(ldevice);

    return r;
}

void
renderer_destroy(Device *ldevice, Renderer *r)
{
    frame_buffer_destroy(ldevice, r->framebuffer);
    render_pass_destroy(ldevice, r->render_pass);
    texture_destroy(ldevice, &r->color_image);
    texture_destroy(ldevice, &r->depth_image);
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

    Device ldevice =
        logical_device_create(surface, pdevice, device_extensions, ARR_COUNT(device_extensions), layers, ARR_COUNT(layers));

    VkCommandPool cmd_pool = command_pool_create(&ldevice);

    Swapchain swapchain = swapchain_create(surface, pdevice, &ldevice, cmd_pool, 2);

    Image          depth_image   = depth_buffer_create(pdevice, &ldevice, &swapchain, cmd_pool);
    VkRenderPass         render_pass   = render_pass_create(&ldevice, swapchain.format.format, depth_image.format);
    Framebuffers   frame_buffers = frame_buffers_create(&swapchain, render_pass, &depth_image);
    CommandBuffers cmd_bufs      = command_buffers_allocate(&ldevice, cmd_pool, swapchain.image_count);

    Renderer renderer = renderer_create(pdevice, &ldevice, &swapchain, depth_image.format);

    VkDescriptorPool imgui_desc_pool = init_gui(window, instance, pdevice, &ldevice, swapchain.image_count, render_pass, cmd_pool);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::ShowDemoWindow();
        }

        uint32_t        current_image = swapchain_acquire(&swapchain);
        VkCommandBuffer cmd_buf       = cmd_bufs.handles[current_image];
        command_buffer_begin(cmd_buf);

        // Render UI
        {
            VkClearValue clear_colors[2] = {0};
            clear_colors[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clear_colors[1].depthStencil = {1.0f, 0};

            VkRenderPassBeginInfo ui_render_pass = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            ui_render_pass.clearValueCount       = 2;
            ui_render_pass.pClearValues          = clear_colors;
            ui_render_pass.renderPass            = render_pass;
            ui_render_pass.framebuffer           = frame_buffers.handles[current_image];
            ui_render_pass.renderArea            = {{0, 0}, {swapchain.width, swapchain.height}};

            vkCmdBeginRenderPass(cmd_buf, &ui_render_pass, VK_SUBPASS_CONTENTS_INLINE);
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd_buf);
            vkCmdEndRenderPass(cmd_buf);
        }

        command_buffer_end(cmd_buf);
        swapchain_present(&swapchain, &cmd_bufs);
    }

    vkDeviceWaitIdle(ldevice.handle);

    vkDestroyDescriptorPool(ldevice.handle, imgui_desc_pool, 0);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    renderer_destroy(&ldevice, &renderer);
    command_buffers_free(&ldevice, cmd_pool, &cmd_bufs);
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
