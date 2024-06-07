#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkPhysicalDevice
physical_device_find_compatible(VkInstance instance, const char **required_extensions, uint32_t required_extension_count)
{
    // Get all physical devices
    uint32_t device_count;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, 0));

    VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, devices));

    // Check for compatability
    for (uint32_t i = 0; i < device_count; ++i) {
        VkPhysicalDevice device = devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        // Only select discrete GPU's
        if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            continue;
        }

        uint32_t device_extension_count;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &device_extension_count, 0));

        VkExtensionProperties *device_extensions = malloc(device_extension_count * sizeof(VkExtensionProperties));
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &device_extension_count, device_extensions));

        uint8_t compatible = 1;

        for (int j = 0; j < required_extension_count; ++j) {
            const char *required_extension = required_extensions[j];
            uint8_t     found              = 0;

            for (int k = 0; k < device_extension_count; ++k) {
                VkExtensionProperties device_extension = device_extensions[k];

                if (strcmp(device_extension.extensionName, required_extension) == 0) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                compatible = 0;
                break;
            }
        }

        if (!compatible) {
            free(device_extensions);
            continue;
        }

        free(device_extensions);
        free(devices);
        return device;
    }

    free(devices);
    return 0;
}
