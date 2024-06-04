#include "vulkan_renderer.h"

#include <stdio.h>
#include <stdlib.h>

#define VK_CHECK(call)                                                         \
    if (call != VK_SUCCESS) {                                                  \
        fprintf(stderr, "Vulkan call failed at %s:%d\n", __FILE__, __LINE__);  \
        exit(1);                                                               \
    }

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkInstance
create_vulkan_instance(const char *name, int version, const char **extensions,
                       int extensions_count, const char **layers,
                       int layers_count)
{
    VkApplicationInfo application_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    application_info.pApplicationName  = name;
    application_info.pEngineName       = name;
    application_info.apiVersion        = version;

    // @Todo: check if extensions and layers are available

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo     = &application_info;
    create_info.enabledExtensionCount   = extensions_count;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount       = layers_count;
    create_info.ppEnabledLayerNames     = layers;

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&create_info, g_allocator, &instance));
}
