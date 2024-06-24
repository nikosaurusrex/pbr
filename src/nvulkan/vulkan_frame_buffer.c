#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkFramebuffer
frame_buffer_create(Swapchain *sc, VkRenderPass render_pass, VkImageView color_view, VkImageView depth_view)
{
    VkImageView attachments[2] = {color_view, depth_view};

    VkFramebufferCreateInfo create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    create_info.renderPass              = render_pass;
    create_info.attachmentCount         = 2;
    create_info.pAttachments            = attachments;
    create_info.width                   = sc->width;
    create_info.height                  = sc->height;
    create_info.layers                  = 1;

    VkFramebuffer framebuffer = {0};
    vkCreateFramebuffer(sc->ldevice->handle, &create_info, g_allocator, &framebuffer);

    return framebuffer;
}

void
frame_buffer_destroy(VkFramebuffer framebuffer, Device *ldevice)
{
    vkDestroyFramebuffer(ldevice->handle, framebuffer, g_allocator);
}

Framebuffers
frame_buffers_create(Swapchain *sc, VkRenderPass render_pass, Image *depth_image)
{
    Framebuffers framebuffers = {0};

    VkImageView attachments[2] = {0};

    VkFramebufferCreateInfo create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    create_info.renderPass              = render_pass;
    create_info.attachmentCount         = 2;
    create_info.pAttachments            = attachments;
    create_info.width                   = sc->width;
    create_info.height                  = sc->height;
    create_info.layers                  = 1;

    framebuffers.count   = sc->image_count;
    framebuffers.handles = malloc(framebuffers.count * sizeof(VkFramebuffer));
    for (U32 i = 0; i < framebuffers.count; ++i) {
        attachments[0] = sc->image_views[i];
        attachments[1] = depth_image->view;

        vkCreateFramebuffer(sc->ldevice->handle, &create_info, g_allocator, &framebuffers.handles[i]);
    }

    return framebuffers;
}

void
frame_buffers_destroy(Framebuffers *framebuffers, Device *ldevice)
{
    for (U32 i = 0; i < framebuffers->count; ++i) {
        vkDestroyFramebuffer(ldevice->handle, framebuffers->handles[i], g_allocator);
    }

    free(framebuffers->handles);
};
