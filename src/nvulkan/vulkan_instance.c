#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkInstance
vulkan_instance_create(const char *name, int version, const char **extensions, uint32_t extension_count, const char **layers,
                       uint32_t layer_count)
{
    VkApplicationInfo application_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    application_info.pApplicationName  = name;
    application_info.pEngineName       = name;
    application_info.apiVersion        = version;

    // @Todo: check if extensions and layers are available

    VkInstanceCreateInfo create_info    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo        = &application_info;
    create_info.enabledExtensionCount   = extension_count;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount       = layer_count;
    create_info.ppEnabledLayerNames     = layers;

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&create_info, g_allocator, &instance));

    return instance;
}

void
vulkan_instance_destroy(VkInstance instance)
{
    vkDestroyInstance(instance, g_allocator);
}
