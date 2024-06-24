#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkRenderPass
render_pass_create_present(VkFormat color_format, VkFormat depth_format, Device *ldevice)
{
    VkAttachmentDescription attachments[2] = {0};

    VkAttachmentDescription color_attachment = {0};
    color_attachment.format                  = color_format;
    color_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[0] = color_attachment;

    VkAttachmentDescription depth_attachment = {0};
    depth_attachment.format                  = depth_format;
    depth_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.finalLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;

    attachments[1] = depth_attachment;

    VkAttachmentReference color_ref = {0};
    color_ref.attachment            = 0;
    color_ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_ref = {0};
    depth_ref.attachment            = 1;
    depth_ref.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency subpass_dependency = {0};
    subpass_dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass          = 0;
    subpass_dependency.srcStageMask        = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
    subpass_dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpass_dependency.dependencyFlags     = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDescription subpass_description    = {0};
    subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount    = 1;
    subpass_description.pColorAttachments       = &color_ref;
    subpass_description.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount        = 2;
    render_pass_info.pAttachments           = attachments;
    render_pass_info.subpassCount           = 1;
    render_pass_info.pSubpasses             = &subpass_description;
    render_pass_info.dependencyCount        = 1;
    render_pass_info.pDependencies          = &subpass_dependency;

    VkRenderPass render_pass;
    VK_CHECK(vkCreateRenderPass(ldevice->handle, &render_pass_info, g_allocator, &render_pass));

    return render_pass;
}

VkRenderPass
render_pass_create_offscreen(VkFormat color_format, VkFormat depth_format, Device *ldevice)
{
    VkAttachmentDescription attachments[2] = {0};

    VkAttachmentDescription color_attachment = {0};
    color_attachment.format                  = color_format;
    color_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.initialLayout           = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.finalLayout             = VK_IMAGE_LAYOUT_GENERAL;

    attachments[0] = color_attachment;

    VkAttachmentDescription depth_attachment = {0};
    depth_attachment.format                  = depth_format;
    depth_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.initialLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.finalLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;

    attachments[1] = depth_attachment;

    VkAttachmentReference color_ref = {0};
    color_ref.attachment            = 0;
    color_ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_ref = {0};
    depth_ref.attachment            = 1;
    depth_ref.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency subpass_dependency = {0};
    subpass_dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass          = 0;
    subpass_dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask       = 0;
    subpass_dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dependencyFlags     = 0;

    VkSubpassDescription subpass_description    = {0};
    subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount    = 1;
    subpass_description.pColorAttachments       = &color_ref;
    subpass_description.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount        = 2;
    render_pass_info.pAttachments           = attachments;
    render_pass_info.subpassCount           = 1;
    render_pass_info.pSubpasses             = &subpass_description;
    render_pass_info.dependencyCount        = 1;
    render_pass_info.pDependencies          = &subpass_dependency;

    VkRenderPass render_pass;
    VK_CHECK(vkCreateRenderPass(ldevice->handle, &render_pass_info, g_allocator, &render_pass));

    return render_pass;
}

void
render_pass_destroy(VkRenderPass render_pass, Device *ldevice)
{
    vkDestroyRenderPass(ldevice->handle, render_pass, g_allocator);
}
