#include "nvulkan.h"

#include <string.h>

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

static void
buffer_create_internal(VkDeviceSize size, VkBufferUsageFlags usage, VkPhysicalDevice pdevice, Device *ldevice,
                       VkMemoryPropertyFlags properties, Buffer *buffer)
{
    VkBufferCreateInfo info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size               = size;
    info.usage              = usage;
    info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(ldevice->handle, &info, 0, &buffer->handle));

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(ldevice->handle, buffer->handle, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize       = mem_reqs.size;
    alloc_info.memoryTypeIndex      = memory_type_find(pdevice, mem_reqs.memoryTypeBits, properties);

    if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0) {
        VkMemoryAllocateFlagsInfo alloc_flags = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
        alloc_flags.flags                     = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

        alloc_info.pNext = &alloc_flags;
    }

    VK_CHECK(vkAllocateMemory(ldevice->handle, &alloc_info, g_allocator, &buffer->memory) != VK_SUCCESS);

    vkBindBufferMemory(ldevice->handle, buffer->handle, buffer->memory, 0);
}

static void
buffer_copy_internal(VkBuffer dst, VkBuffer src, VkDeviceSize size, Device *ldevice, VkCommandPool cmd_pool)
{
    VkCommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandPool                 = cmd_pool;
    info.commandBufferCount          = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(ldevice->handle, &info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size      = size;
    vkCmdCopyBuffer(command_buffer, src, dst, 1, &region);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers    = &command_buffer;

    vkQueueSubmit(ldevice->graphics_queue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(ldevice->graphics_queue);

    vkFreeCommandBuffers(ldevice->handle, cmd_pool, 1, &command_buffer);
}

Buffer
buffer_create(VkDeviceSize size, void *data, VkBufferUsageFlags usage, VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool)
{
    Buffer buffer         = {0};
    Buffer staging_buffer = buffer_create_staging(size, data, pdevice, ldevice);

    buffer_create_internal(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, pdevice, ldevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer);

    buffer_copy_internal(buffer.handle, staging_buffer.handle, size, ldevice, cmd_pool);

    buffer_destroy(&staging_buffer, ldevice);

    return buffer;
}

Buffer
buffer_create_staging(VkDeviceSize size, void *data, VkPhysicalDevice pdevice, Device *ldevice)
{
    Buffer buffer = {0};

    buffer_create_internal(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, pdevice, ldevice,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer);

    void *mapped;
    vkMapMemory(ldevice->handle, buffer.memory, 0, size, 0, &mapped);
    MemoryCopy(mapped, data, size);
    vkUnmapMemory(ldevice->handle, buffer.memory);

    return buffer;
}

void
buffer_destroy(Buffer *buffer, Device *ldevice)
{
    vkDestroyBuffer(ldevice->handle, buffer->handle, g_allocator);
    vkFreeMemory(ldevice->handle, buffer->memory, g_allocator);
}
