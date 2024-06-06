#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VulkanDevice
logical_device_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, array<const char *> extensions, array<const char *> layers)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(pdevice, &memory_properties);

    uint32_t queue_families_count;
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_families_count, 0);
    array<VkQueueFamilyProperties> queue_families;
    queue_families.resize(queue_families_count);
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_families_count, queue_families.data());

    // Setup device queues
    VkDeviceQueueCreateInfo queue_create_infos[1];

    float queue_priority = 1.0f;

    uint32_t graphics_index = ~0u;
    for (uint32_t i = 0; i < queue_families_count; ++i) {
        VkQueueFamilyProperties queue_family = queue_families[i];

        if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            VkBool32 present_supported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, surface, &present_supported);

            if (!present_supported) {
                continue;
            }

            VkDeviceQueueCreateInfo create_info = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            create_info.queueFamilyIndex        = i;
            create_info.queueCount              = 1; // @Todo: check: Maybe we want more?
            create_info.pQueuePriorities        = &queue_priority;

            queue_create_infos[0] = create_info;

            graphics_index = i;
            break;
        }
    }

    VkPhysicalDeviceFeatures features_core = {};

    VkDeviceCreateInfo create_info      = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.queueCreateInfoCount    = ARR_COUNT(queue_create_infos);
    create_info.pQueueCreateInfos       = queue_create_infos;
    create_info.enabledExtensionCount   = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount       = layers.size();
    create_info.ppEnabledLayerNames     = layers.data();
    create_info.pEnabledFeatures        = &features_core;

    VkDevice handle;
    VK_CHECK(vkCreateDevice(pdevice, &create_info, g_allocator, &handle));

    VkQueue graphics_queue;
    vkGetDeviceQueue(handle, graphics_index, 0, &graphics_queue);

    VulkanDevice ldevice;
    ldevice.handle               = handle;
    ldevice.graphics_queue       = graphics_queue;
    ldevice.graphics_queue_index = graphics_index;

    return ldevice;
}

void
logical_device_destroy(VulkanDevice *ldevice)
{
    vkDestroyDevice(ldevice->handle, g_allocator);
}
