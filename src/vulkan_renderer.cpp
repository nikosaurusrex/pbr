#include "vulkan_renderer.h"

#include <stdio.h>
#include <stdlib.h>

#define VK_CHECK(call)                                                                                                                                                                                 \
    if (call != VK_SUCCESS) {                                                                                                                                                                          \
        fprintf(stderr, "Vulkan call failed at %s:%d\n", __FILE__, __LINE__);                                                                                                                          \
        exit(1);                                                                                                                                                                                       \
    }

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkInstance
create_vulkan_instance(const char *name, int version, Array<const char *> extensions, Array<const char *> layers)
{
    VkApplicationInfo application_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    application_info.pApplicationName  = name;
    application_info.pEngineName       = name;
    application_info.apiVersion        = version;

    // @Todo: check if extensions and layers are available

    VkInstanceCreateInfo create_info    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo        = &application_info;
    create_info.enabledExtensionCount   = extensions.count;
    create_info.ppEnabledExtensionNames = extensions.data;
    create_info.enabledLayerCount       = layers.count;
    create_info.ppEnabledLayerNames     = layers.data;

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&create_info, g_allocator, &instance));

    return instance;
}

void
destroy_vulkan_instance(VkInstance instance)
{
    vkDestroyInstance(instance, g_allocator);
}

VkSurfaceKHR
create_surface(VkInstance instance, GLFWwindow *glfw_window)
{
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, glfw_window, g_allocator, &surface);
    return surface;
}

void
destroy_surface(VkInstance instance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(instance, surface, g_allocator);
}

VkPhysicalDevice
find_compatible_device(VkInstance instance, Array<const char *> required_extensions)
{
    // Get all physical devices
    uint32_t count;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, 0));

    Array<VkPhysicalDevice> devices(count);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, devices.data));

    // Check for compatability
    for (uint32_t i = 0; i < devices.count; ++i) {
        VkPhysicalDevice device = devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        // Only select discrete GPU's
        if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            continue;
        }

        uint32_t device_extensions_count;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &device_extensions_count, 0));

        Array<VkExtensionProperties> device_extensions(device_extensions_count);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &device_extensions_count, device_extensions.data));

        bool compatible = true;

        for (int j = 0; j < required_extensions.count; ++j) {
            const char *required_extension = required_extensions[j];
            bool        found              = false;

            for (int k = 0; k < device_extensions.count; ++k) {
                VkExtensionProperties device_extension = device_extensions[k];

                if (strcmp(device_extension.extensionName, required_extension) == 0) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                compatible = false;
                break;
            }
        }

        if (!compatible) {
            continue;
        }

        return device;
    }

    return 0;
}

VkDevice
create_logical_device(VkPhysicalDevice pdevice, VkSurfaceKHR surface, Array<const char *> extensions, Array<const char *> layers)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(pdevice, &memory_properties);

    uint32_t queue_families_count;
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_families_count, 0);
    Array<VkQueueFamilyProperties> queue_families(queue_families_count);
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_families_count, queue_families.data);

    // Setup device queues
    Array<VkDeviceQueueCreateInfo> queue_create_infos(2);

    float queue_priority = 1.0f;

    bool graphics_found = false;
    bool present_found  = false;
    for (uint32_t i = 0; i < queue_families_count; ++i) {
        VkQueueFamilyProperties queue_family = queue_families[i];

        if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !graphics_found) {
            VkDeviceQueueCreateInfo create_info = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            create_info.queueFamilyIndex        = i;
            create_info.queueCount              = 1; // @Todo: check: Maybe we want more?
            create_info.pQueuePriorities        = &queue_priority;

            queue_create_infos[0] = create_info;

            graphics_found = true;
        }

        VkBool32 present_supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, surface, &present_supported);

        if (present_supported && !present_found) {
            VkDeviceQueueCreateInfo create_info = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            create_info.queueFamilyIndex        = i;
            create_info.queueCount              = 1; // @Todo: check: Maybe we want more?
            create_info.pQueuePriorities        = &queue_priority;

            queue_create_infos[1] = create_info;

            present_found = true;
        }
    }

    VkDeviceCreateInfo create_info      = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.queueCreateInfoCount    = queue_create_infos.count;
    create_info.pQueueCreateInfos       = queue_create_infos.data;
    create_info.enabledExtensionCount   = extensions.count;
    create_info.ppEnabledExtensionNames = extensions.data;
    create_info.enabledLayerCount       = layers.count;
    create_info.ppEnabledLayerNames     = layers.data;

    VkDevice ldevice;
    VK_CHECK(vkCreateDevice(pdevice, &create_info, g_allocator, &ldevice));
    return ldevice;
}

void
destroy_device(VkDevice ldevice)
{
    vkDestroyDevice(ldevice, g_allocator);
}
