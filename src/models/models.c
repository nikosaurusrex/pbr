#include "models.h"

void
models_write_descriptors(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, DescriptorSet *desc_set,
                         ModelDescriptor *descriptors, u32 descriptor_count)
{
    u32 size = descriptor_count * sizeof(ModelDescriptor);

    // @Todo: this buffer is never destroyed - do when we have a model manager
    Buffer buffer =
        buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, descriptors, size);

    VkDescriptorBufferInfo buffer_desc = {buffer.handle, 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    desc_write.dstSet               = desc_set->handle;
    desc_write.dstBinding           = 2; // @Todo this should be a constant best shared with the shader
    desc_write.descriptorCount      = 1;
    desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_write.pBufferInfo          = &buffer_desc;

    vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);
}
