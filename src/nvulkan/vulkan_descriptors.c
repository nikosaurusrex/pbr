#include "nvulkan.h"

#include <assert.h>

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

DescriptorSet
descriptor_set_create(Device *ldevice, DescriptorBinding *bindings, uint32_t binding_count)
{
    DescriptorSet desc_set = {0};

    VkDescriptorSetLayoutCreateFlags flags = 0;

    VkDescriptorBindingFlags *binding_flags = malloc(binding_count * sizeof(VkDescriptorBindingFlags));
    for (uint32_t i = 0; i < binding_count; i++) {
        binding_flags[i] = 0;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};
    binding_info.bindingCount                                = binding_count;
    binding_info.pBindingFlags                               = binding_flags;

    VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layout_info.bindingCount                    = binding_count;
    layout_info.pBindings                       = bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(ldevice->handle, &layout_info, g_allocator, &desc_set.layout));

    // binding_count is an upper bound on the number of descriptor sets that can be allocated from the pool
    VkDescriptorPoolSize *pool_sizes      = malloc(binding_count * sizeof(VkDescriptorPoolSize));
    uint32_t              pool_size_count = 0;

    for (uint32_t i = 0; i < binding_count; i++) {
        DescriptorBinding binding = bindings[i];
        if (binding.count == 0) {
            continue;
        }

        uint8_t found = 0;
        for (uint32_t j = 0; j < pool_size_count; j++) {
            if (pool_sizes[j].type == binding.type) {
                pool_sizes[j].descriptorCount += binding.count;
                found = 1;
                break;
            }
        }

        if (!found) {
            VkDescriptorPoolSize pool_size = {binding.type, binding.count};

            assert(pool_size_count < binding_count);
            pool_sizes[pool_size_count++]   = pool_size;
        }
    }

    VkDescriptorPoolCreateInfo desc_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    desc_pool_info.maxSets                    = 1;
    desc_pool_info.poolSizeCount              = pool_size_count;
    desc_pool_info.pPoolSizes                 = pool_sizes;

    VK_CHECK(vkCreateDescriptorPool(ldevice->handle, &desc_pool_info, g_allocator, &desc_set.pool));

    VkDescriptorSetAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocate_info.descriptorPool               = desc_set.pool;
    allocate_info.descriptorSetCount           = 1;
    allocate_info.pSetLayouts                  = &desc_set.layout;

    VK_CHECK(vkAllocateDescriptorSets(ldevice->handle, &allocate_info, &desc_set.handle));

    free(pool_sizes);
    free(binding_flags);

    return desc_set;
}

void
descriptor_set_destroy(Device *ldevice, DescriptorSet *descriptor_set)
{
    // vkFreeDescriptorSets(ldevice->handle, descriptor_set->pool, 1, &descriptor_set->handle);
    vkDestroyDescriptorPool(ldevice->handle, descriptor_set->pool, g_allocator);
    vkDestroyDescriptorSetLayout(ldevice->handle, descriptor_set->layout, g_allocator);
}
