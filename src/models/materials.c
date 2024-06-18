#include "models.h"

#include <string.h>

void
materials_init(Materials *materials)
{
    *materials = (Materials){.names = 0, .materials = 0, .count = 0, .capacity = 1, .buffer = {0}};

    materials_add(materials, "default",
                  (Material){.ambient       = {0.1f, 0.1f, 0.1f, 1.0f},
                             .diffuse       = {0.6f, 0.6f, 0.6f, 1.0f},
                             .specular      = {0.3f, 0.3f, 0.3f, 1.0f},
                             .transmittance = {0.0f, 0.0f, 0.0f, 1.0f},
                             .emission      = {0.0f, 0.0f, 0.0f, 1.0f},
                             .shininess     = 1.0f,
                             .ior           = 1.0f,
                             .dissolve      = 1.0f,
                             .illum         = 2});
}

u32
materials_add(Materials *materials, const char *name, Material mat)
{
    Assert(materials->count < max_u32);

    if (materials->count + 1 >= materials->capacity) {
        Assert(materials->capacity < max_u32 / 2);
        materials->capacity *= 2;

        materials->materials = (Material *)realloc(materials->materials, materials->capacity * sizeof(Material));
        materials->names     = (const char **)realloc(materials->names, materials->capacity * sizeof(const char *));
    }

    materials->materials[materials->count] = mat;
    materials->names[materials->count]     = name;
    materials->count++;

    return materials->count - 1;
}

u8
materials_has(Materials *materials, const char *name)
{
    for (u32 i = 0; i < materials->count; ++i) {
        if (strcmp(materials->names[i], name) == 0) {
            return 1;
        }
    }

    return 0;
}

u32
materials_get_index(Materials *materials, const char *name)
{
    for (u32 i = 0; i < materials->count; ++i) {
        if (strcmp(materials->names[i], name) == 0) {
            return i;
        }
    }

    return max_u32;
}

void
materials_free(Device *ldevice, Materials *materials)
{
    free(materials->materials);
    free(materials->names);

    // @Todo: destroy buffer but crashes
}

void
materials_write_descriptors(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, DescriptorSet *desc_set,
                            Materials *materials)
{
    if (materials->buffer.handle != VK_NULL_HANDLE) {
        buffer_destroy(ldevice, &materials->buffer);
    }

    u32 size = materials->count * sizeof(Material);

    materials->buffer = buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      materials->materials, size);

    VkDescriptorBufferInfo buffer_desc = {materials->buffer.handle, 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    desc_write.dstSet               = desc_set->handle;
    desc_write.dstBinding           = 1;
    desc_write.descriptorCount      = 1;
    desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_write.pBufferInfo          = &buffer_desc;

    vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);
}
