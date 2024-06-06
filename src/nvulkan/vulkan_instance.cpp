#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkInstance
vulkan_instance_create(const char *name, int version, array<const char *> extensions, array<const char *> layers)
{
    VkApplicationInfo application_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    application_info.pApplicationName  = name;
    application_info.pEngineName       = name;
    application_info.apiVersion        = version;

    // @Todo: check if extensions and layers are available

    VkInstanceCreateInfo create_info    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo        = &application_info;
    create_info.enabledExtensionCount   = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount       = layers.size();
    create_info.ppEnabledLayerNames     = layers.data();

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&create_info, g_allocator, &instance));

    return instance;
}

void
vulkan_instance_destroy(VkInstance instance)
{
    vkDestroyInstance(instance, g_allocator);
}
