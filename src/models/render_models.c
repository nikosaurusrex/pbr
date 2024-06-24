#include "models.h"

PBRRenderer
pbr_renderer_create(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool, VkFormat depth_format,
                    uint32_t diffuse_texture_count)
{
    PBRRenderer r = {0};

    VkFormat color_format = VK_FORMAT_R32G32B32A32_SFLOAT;
    U32      width        = sc->width;
    U32      height       = sc->height;

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
    r.render_pass = render_pass_create_offscreen(ldevice, color_format, r.depth_image.image.format);

    // Create framebuffer
    r.framebuffer = frame_buffer_create(sc, r.render_pass, r.color_image.image.view, r.depth_image.image.view);

    // Create descriptor set
    VkDescriptorSetLayoutBinding bindings[] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, 0},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, 0},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, 0},
        {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, diffuse_texture_count,
         VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0},
    };

    r.desc_set = descriptor_set_create(ldevice, bindings, ArrayCount(bindings));

    // Setup graphics pipeline
    Shader shaders[] = {
        {"assets/shaders/vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
        {"assets/shaders/pbr_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT},
    };

    VkVertexInputBindingDescription vertex_bindings[] = {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};

    VkVertexInputAttributeDescription vertex_attributes[] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
        {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coords)},
        {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
    };

    r.pipeline = pipeline_create(ldevice, &r.desc_set, r.render_pass, shaders, ArrayCount(shaders), vertex_bindings,
                                 ArrayCount(vertex_bindings), vertex_attributes, ArrayCount(vertex_attributes), VK_CULL_MODE_FRONT_BIT);

    float null_uniforms[sizeof(GlobalUniforms)] = {0};
    r.uniforms = buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               null_uniforms, sizeof(GlobalUniforms));

    VkDescriptorBufferInfo buffer_desc = {r.uniforms.handle, 0, VK_WHOLE_SIZE};

    // Write uniform buffer descriptor GlobalUniforms
    VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    desc_write.dstSet               = r.desc_set.handle;
    desc_write.dstBinding           = 0;
    desc_write.descriptorCount      = 1;
    desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_write.pBufferInfo          = &buffer_desc;

    vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);

    return r;
}

void
pbr_renderer_destroy(Device *ldevice, PBRRenderer *r)
{
    buffer_destroy(ldevice, &r->uniforms);
    pipeline_destroy(ldevice, &r->pipeline);
    descriptor_set_destroy(ldevice, &r->desc_set);
    frame_buffer_destroy(ldevice, r->framebuffer);
    render_pass_destroy(ldevice, r->render_pass);
    texture_destroy(ldevice, &r->color_image);
    texture_destroy(ldevice, &r->depth_image);
}

void
pbr_renderer_render(Swapchain *sc, VkCommandBuffer cmd_buf, PBRRenderer *r, Model *models, U32 model_count, VkClearValue *clear_colors)
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

void
pbr_renderer_update_uniforms(PBRRenderer *r, VkCommandBuffer cmd_buf, GlobalUniforms *uniforms)
{
    VkBufferMemoryBarrier beforeBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    beforeBarrier.srcAccessMask         = VK_ACCESS_SHADER_READ_BIT;
    beforeBarrier.dstAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
    beforeBarrier.buffer                = r->uniforms.handle;
    beforeBarrier.offset                = 0;
    beforeBarrier.size                  = sizeof(GlobalUniforms);
    vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, 0,
                         1, &beforeBarrier, 0, 0);

    vkCmdUpdateBuffer(cmd_buf, r->uniforms.handle, 0, sizeof(GlobalUniforms), uniforms);

    VkBufferMemoryBarrier afterBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    afterBarrier.srcAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
    afterBarrier.dstAccessMask         = VK_ACCESS_SHADER_READ_BIT;
    afterBarrier.buffer                = r->uniforms.handle;
    afterBarrier.offset                = 0;
    afterBarrier.size                  = sizeof(GlobalUniforms);
    vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, 0,
                         1, &afterBarrier, 0, 0);
};

/*
SceneRenderer
scene_renderer_create(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool, VkFormat depth_format, uint32_t
diffuse_texture_count)
{
    SceneRenderer r = {0};

    VkFormat color_format = sc->format.format;
    U32      width        = sc->width;
    U32      height       = sc->height;

    // Create color and depth images
    r.color_image                        = texture_create(pdevice, ldevice, color_format, width, height, VK_IMAGE_ASPECT_COLOR_BIT,
                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
VK_IMAGE_USAGE_STORAGE_BIT); r.color_image.descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

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
    VkDescriptorSetLayoutBinding bindings[] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, 0},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, 0},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, 0},
        {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, diffuse_texture_count,
         VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0},
    };

    r.desc_set = descriptor_set_create(ldevice, bindings, ArrayCount(bindings));

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

    r.pipeline = pipeline_create(ldevice, &r.desc_set, r.render_pass, shaders, ArrayCount(shaders), vertex_bindings,
                                 ArrayCount(vertex_bindings), vertex_attributes, ArrayCount(vertex_attributes), VK_CULL_MODE_FRONT_BIT);

    float null_uniforms[sizeof(GlobalUniforms)] = {0};
    r.uniforms = buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               null_uniforms, sizeof(GlobalUniforms));

    VkDescriptorBufferInfo buffer_desc = {r.uniforms.handle, 0, VK_WHOLE_SIZE};

    // Write uniform buffer descriptor GlobalUniforms
    VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    desc_write.dstSet               = r.desc_set.handle;
    desc_write.dstBinding           = 0;
    desc_write.descriptorCount      = 1;
    desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_write.pBufferInfo          = &buffer_desc;

    vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);

    return r;
}

void
scene_renderer_destroy(Device *ldevice, SceneRenderer *r)
{
    buffer_destroy(ldevice, &r->uniforms);
    pipeline_destroy(ldevice, &r->pipeline);
    descriptor_set_destroy(ldevice, &r->desc_set);
    frame_buffer_destroy(ldevice, r->framebuffer);
    render_pass_destroy(ldevice, r->render_pass);
    texture_destroy(ldevice, &r->color_image);
    texture_destroy(ldevice, &r->depth_image);
}

void
scene_renderer_render(Swapchain *sc, VkCommandBuffer cmd_buf, SceneRenderer *r, Model *models, U32 model_count, VkClearValue *clear_colors)
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

void
scene_renderer_update_uniforms(SceneRenderer *r, VkCommandBuffer cmd_buf, GlobalUniforms *uniforms)
{
    VkBufferMemoryBarrier beforeBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    beforeBarrier.srcAccessMask         = VK_ACCESS_SHADER_READ_BIT;
    beforeBarrier.dstAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
    beforeBarrier.buffer                = r->uniforms.handle;
    beforeBarrier.offset                = 0;
    beforeBarrier.size                  = sizeof(GlobalUniforms);
    vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, 0,
                         1, &beforeBarrier, 0, 0);

    vkCmdUpdateBuffer(cmd_buf, r->uniforms.handle, 0, sizeof(GlobalUniforms), uniforms);

    VkBufferMemoryBarrier afterBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    afterBarrier.srcAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
    afterBarrier.dstAccessMask         = VK_ACCESS_SHADER_READ_BIT;
    afterBarrier.buffer                = r->uniforms.handle;
    afterBarrier.offset                = 0;
    afterBarrier.size                  = sizeof(GlobalUniforms);
    vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, 0,
                         1, &afterBarrier, 0, 0);
};*/
