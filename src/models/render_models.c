#include "models.h"

SceneRenderer
scene_renderer_create(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool, VkFormat depth_format)
{
    SceneRenderer r = {0};

    VkFormat color_format = sc->format.format;
    uint32_t width        = sc->width;
    uint32_t height       = sc->height;

    // Create color and depth images
    r.color_image                        = texture_create(pdevice, ldevice, color_format, width, height, VK_IMAGE_ASPECT_COLOR_BIT,
                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    r.color_image.descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    r.depth_image = texture_create(pdevice, ldevice, depth_format, width, height, VK_IMAGE_ASPECT_DEPTH_BIT,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // Transition image layouts
    VkCommandBuffer cmd_buf = command_buffer_allocate(ldevice, cmd_pool);
    command_buffer_begin(cmd_buf);

    image_transition_layout(cmd_buf, &r.color_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    image_transition_layout(cmd_buf, &r.depth_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    command_buffer_submit(ldevice, cmd_buf);
    command_buffer_free(ldevice, cmd_pool, cmd_buf);

    // Create render pass for offscreen rendering
    r.render_pass = render_pass_create_offscreen(ldevice, r.color_image.image.format, r.depth_image.image.format);

    // Create framebuffer
    r.framebuffer = frame_buffer_create(sc, r.render_pass, r.color_image.image.view, r.depth_image.image.view);

    // Create descriptor set
    r.desc_set = descriptor_set_create(ldevice, 0, 0);

    // Setup graphics pipeline
    Shader shaders[] = {
        {"assets/shaders/vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
        {"assets/shaders/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT},
    };

    VkVertexInputBindingDescription vertex_bindings[] = {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};

    VkVertexInputAttributeDescription vertex_attributes[] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
        {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coords)},
        {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
    };

    r.pipeline = pipeline_create(ldevice, &r.desc_set, r.render_pass, shaders, ARR_COUNT(shaders), vertex_bindings,
                                 ARR_COUNT(vertex_bindings), vertex_attributes, ARR_COUNT(vertex_attributes), VK_CULL_MODE_BACK_BIT);

    return r;
}

void
scene_renderer_destroy(Device *ldevice, SceneRenderer *r)
{
    pipeline_destroy(ldevice, &r->pipeline);
    descriptor_set_destroy(ldevice, &r->desc_set);
    frame_buffer_destroy(ldevice, r->framebuffer);
    render_pass_destroy(ldevice, r->render_pass);
    texture_destroy(ldevice, &r->color_image);
    texture_destroy(ldevice, &r->depth_image);
}

void
scene_renderer_render(Swapchain *sc, VkCommandBuffer cmd_buf, SceneRenderer *r, Model *models, uint32_t model_count,
                      VkClearValue *clear_colors)
{
    VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    begin_info.clearValueCount       = 2;
    begin_info.pClearValues          = clear_colors;
    begin_info.renderPass            = r->render_pass;
    begin_info.framebuffer           = r->framebuffer;
    begin_info.renderArea            = (VkRect2D){{0, 0}, {sc->width, sc->height}};

    // Rendering Scene
    vkCmdBeginRenderPass(cmd_buf, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {0.0f, 0.0f, sc->width, sc->height, 0.0f, 1.0f};
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor = {{0, 0}, {sc->width, sc->height}};
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipeline.handle);
    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipeline.layout, 0, 1, &r->desc_set.handle, 0, 0);

    for (uint32_t i = 0; i < model_count; ++i) {
        Model *model = &models[i];

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd_buf, 0, 1, &model->vertex_buffer.handle, &offset);
        vkCmdBindIndexBuffer(cmd_buf, model->index_buffer.handle, offset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd_buf, model->index_count, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(cmd_buf);
}
