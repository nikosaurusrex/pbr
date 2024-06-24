#include "vulkan_imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

VkDescriptorPool
gui_init(GLFWwindow *window, VkInstance instance, VkPhysicalDevice pdevice, Device *ldevice, U32 image_count, VkRenderPass render_pass,
         VkCommandPool cmd_pool)
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

void
gui_deinit()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void
gui_new_frame(void)
{
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void
gui_end_frame(VkCommandBuffer cmd_buf)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd_buf);
}

void
gui_render(void)
{
    ImGui::ShowDemoWindow();
}

void
gui_render_materials(Materials *materials)
{
    // ImGui::ShowDemoWindow();

    ImGui::Begin("Materials");

    for (U32 i = 0; i < materials->count; ++i) {
        ImGui::PushID(i);

        char buf[20];
        snprintf(buf, sizeof(buf), "Material %d\n", i);

        ImGui::SeparatorText(buf);

        Material *mat = &materials->materials[i];

        ImGui::ColorEdit3("albedo", &mat->albedo.x);
        ImGui::SliderFloat("metallic", &mat->metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("specular", &mat->specular, 0.0f, 1.0f);
        ImGui::SliderFloat("roughness", &mat->roughness, 0.0f, 1.0f);

        ImGui::PopID();
    }

    ImGui::End();
}
