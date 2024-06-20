#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

static void
buffer_create_internal(VkPhysicalDevice pdevice, Device *ldevice, VkDeviceSize size, VkBufferUsageFlags usage,
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
buffer_copy_internal(Device *ldevice, VkCommandPool cmd_pool, VkBuffer src, VkBuffer dst, VkDeviceSize size)
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
buffer_create(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, VkBufferUsageFlags usage, void *data, VkDeviceSize size)
{
    Buffer buffer         = {0};
    Buffer staging_buffer = buffer_create_staging(pdevice, ldevice, size, data);

    buffer_create_internal(pdevice, ldevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           &buffer.handle, &buffer.memory);

    buffer_copy_internal(ldevice, cmd_pool, staging_buffer.handle, buffer.handle, size);

    buffer_destroy(ldevice, &staging_buffer);

    return buffer;
}

Buffer
buffer_create_staging(VkPhysicalDevice pdevice, Device *ldevice, VkDeviceSize size, void *data)
{
    Buffer buffer = {0};

    buffer_create_internal(pdevice, ldevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer);

    void *mapped;
    vkMapMemory(ldevice->handle, buffer.memory, 0, size, 0, &mapped);
    memcpy(mapped, data, size);
    vkUnmapMemory(ldevice->handle, buffer.memory);

    return buffer;
}

void
buffer_destroy(Device *ldevice, Buffer *buffer)
{
    vkDestroyBuffer(ldevice->handle, buffer->handle, g_allocator);
    vkFreeMemory(ldevice->handle, buffer->memory, g_allocator);
}