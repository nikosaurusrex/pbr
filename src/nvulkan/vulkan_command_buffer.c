#include "nvulkan.h"

VkCommandBuffer
command_buffer_allocate(VulkanDevice *ldevice, VkCommandPool cmd_pool)
{
    VkCommandBufferAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandBufferCount          = 1;
    alloc_info.commandPool                 = cmd_pool;
    alloc_info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd_buf;
    VK_CHECK(vkAllocateCommandBuffers(ldevice->handle, &alloc_info, &cmd_buf));

    return cmd_buf;
}

void
command_buffer_free(VulkanDevice *ldevice, VkCommandPool cmd_pool, VkCommandBuffer cmd_buf)
{
    vkFreeCommandBuffers(ldevice->handle, cmd_pool, 1, &cmd_buf);
}

void
command_buffer_begin(VkCommandBuffer cmd_buf)
{
    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // @Todo make parameter
    VK_CHECK(vkBeginCommandBuffer(cmd_buf, &begin_info));
}

void
command_buffer_submit(VulkanDevice *ldevice, VkCommandBuffer cmd_buf)
{
    VK_CHECK(vkEndCommandBuffer(cmd_buf));

    VkSubmitInfo submit_info       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &cmd_buf;

    vkQueueSubmit(ldevice->graphics_queue, 1, &submit_info, (VkFence){0});
    vkQueueWaitIdle(ldevice->graphics_queue);
}
