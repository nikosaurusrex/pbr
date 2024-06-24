#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkCommandPool
command_pool_create(Device *ldevice)
{
    VkCommandPoolCreateInfo create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    create_info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool cmd_pool;
    VK_CHECK(vkCreateCommandPool(ldevice->handle, &create_info, g_allocator, &cmd_pool));

    return cmd_pool;
}

void
command_pool_destroy(VkCommandPool cmd_pool, Device *ldevice)
{
    vkDestroyCommandPool(ldevice->handle, cmd_pool, g_allocator);
}
