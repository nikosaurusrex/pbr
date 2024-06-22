#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

Device
logical_device_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, const char **extensions, u32 extension_count, const char **layers,
                      u32 layer_count)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(pdevice, &memory_properties);

    u32 queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_family_count, 0);

    VkQueueFamilyProperties *queue_families = malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_family_count, queue_families);

    // Setup device queues
    VkDeviceQueueCreateInfo queue_create_infos[1];

    f32 queue_priority = 1.0f;

    u32 graphics_index = ~0u;
    for (u32 i = 0; i < queue_family_count; ++i) {
        VkQueueFamilyProperties queue_family = queue_families[i];

        if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            b32 present_supported = VK_FALSE;
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
    VkPhysicalDeviceVulkan12Features features_vulkan12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features_vulkan12.bufferDeviceAddress              = VK_TRUE;
    features_vulkan12.runtimeDescriptorArray           = VK_TRUE;

    VkPhysicalDeviceFeatures features_core = {0};
    features_core.shaderInt64              = VK_TRUE;
    features_core.geometryShader           = VK_TRUE;

    VkDeviceCreateInfo create_info      = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.queueCreateInfoCount    = ArrayCount(queue_create_infos);
    create_info.pQueueCreateInfos       = queue_create_infos;
    create_info.enabledExtensionCount   = extension_count;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount       = layer_count;
    create_info.ppEnabledLayerNames     = layers;
    create_info.pEnabledFeatures        = &features_core;
    create_info.pNext                   = &features_vulkan12;

    VkDevice handle;
    VK_CHECK(vkCreateDevice(pdevice, &create_info, g_allocator, &handle));

    VkQueue graphics_queue;
    vkGetDeviceQueue(handle, graphics_index, 0, &graphics_queue);

    Device ldevice;
    ldevice.handle               = handle;
    ldevice.graphics_queue       = graphics_queue;
    ldevice.graphics_queue_index = graphics_index;

    free(queue_families);
    return ldevice;
}

void
logical_device_destroy(Device *ldevice)
{
    vkDestroyDevice(ldevice->handle, g_allocator);
}
