#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

VkPhysicalDevice
physical_device_find_compatible(VkInstance instance, array<const char *> required_extensions)
{
    // Get all physical devices
    uint32_t count;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, 0));

    array<VkPhysicalDevice> devices;
    devices.resize(count);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, devices.data()));

    // Check for compatability
    for (uint32_t i = 0; i < devices.size(); ++i) {
        VkPhysicalDevice device = devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        // Only select discrete GPU's
        if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            continue;
        }

        uint32_t device_extensions_count;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &device_extensions_count, 0));

        array<VkExtensionProperties> device_extensions;
        device_extensions.resize(device_extensions_count);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &device_extensions_count, device_extensions.data()));

        bool compatible = true;

        for (int j = 0; j < required_extensions.size(); ++j) {
            const char *required_extension = required_extensions[j];
            bool        found              = false;

            for (int k = 0; k < device_extensions.size(); ++k) {
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
