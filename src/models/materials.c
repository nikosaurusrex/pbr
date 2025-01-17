#include "models.h"

#include <string.h>

void
materials_init(Materials *materials)
{
  *materials = (Materials){.names = 0, .materials = 0, .count = 0, .capacity = 1, .buffer = {0}};

  materials_add(materials, "default",
                (Material){.albedo = vec4(0.123f, 0.0f, 0.754f, 1.0f), .metallic = 0.0f, .specular = 0.0f, .roughness = 0.0f});
}

U32
materials_add(Materials *materials, const char *name, Material mat)
{
  Assert(materials->count < max_U32);

  if (materials->count + 1 >= materials->capacity) {
    Assert(materials->capacity < max_U32 / 2);
    materials->capacity *= 2;

    materials->materials = (Material *)realloc(materials->materials, materials->capacity * sizeof(Material));
    materials->names     = (const char **)realloc(materials->names, materials->capacity * sizeof(const char *));
  }

  materials->materials[materials->count] = mat;
  materials->names[materials->count]     = name;
  materials->count++;

  return materials->count - 1;
}

B8
materials_has(Materials *materials, const char *name)
{
  for (U32 i = 0; i < materials->count; ++i) {
    if (strcmp(materials->names[i], name) == 0) {
      return 1;
    }
  }

  return 0;
}

U32
materials_get_index(Materials *materials, const char *name)
{
  for (U32 i = 0; i < materials->count; ++i) {
    if (strcmp(materials->names[i], name) == 0) {
      return i;
    }
  }

  return max_U32;
}

void
materials_free(Materials *materials, Device *ldevice)
{
  free(materials->materials);
  free(materials->names);

  // @Todo: destroy buffer but crashes
}

void
materials_write_descriptors(Materials *materials, VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool,
                            DescriptorSet *desc_set)
{
  if (materials->buffer.handle != VK_NULL_HANDLE) {
    buffer_destroy(&materials->buffer, ldevice);
  }

  U32 size = materials->count * sizeof(Material);

  materials->buffer = buffer_create(size, materials->materials, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    pdevice, ldevice, cmd_pool);

  VkDescriptorBufferInfo buffer_desc = {materials->buffer.handle, 0, VK_WHOLE_SIZE};

  VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  desc_write.dstSet               = desc_set->handle;
  desc_write.dstBinding           = 1;
  desc_write.descriptorCount      = 1;
  desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  desc_write.pBufferInfo          = &buffer_desc;

  vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);
}

void
materials_update_uniforms(Materials *materials, VkCommandBuffer cmd_buf)
{
  VkDeviceSize size = materials->count * sizeof(Material);

  VkBufferMemoryBarrier beforeBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
  beforeBarrier.srcAccessMask         = VK_ACCESS_SHADER_READ_BIT;
  beforeBarrier.dstAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
  beforeBarrier.buffer                = materials->buffer.handle;
  beforeBarrier.offset                = 0;
  beforeBarrier.size                  = size;
  vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, 0,
                       1, &beforeBarrier, 0, 0);

  vkCmdUpdateBuffer(cmd_buf, materials->buffer.handle, 0, size, materials->materials);

  VkBufferMemoryBarrier afterBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
  afterBarrier.srcAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
  afterBarrier.dstAccessMask         = VK_ACCESS_SHADER_READ_BIT;
  afterBarrier.buffer                = materials->buffer.handle;
  afterBarrier.offset                = 0;
  afterBarrier.size                  = size;
  vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 0, 0,
                       1, &afterBarrier, 0, 0);
};
