#include "vulkan_renderer.h"

#include <assert.h>
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
create_vulkan_instance(const char *name, int version, array<const char *> extensions, array<const char *> layers)
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
find_compatible_device(VkInstance instance, array<const char *> required_extensions)
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

VulkanDevice
create_logical_device(VkSurfaceKHR surface, VkPhysicalDevice pdevice, array<const char *> extensions, array<const char *> layers)
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
destroy_device(VulkanDevice *ldevice)
{
    vkDestroyDevice(ldevice->handle, g_allocator);
}

VulkanSwapchain
create_swapchain(VkSurfaceKHR surface, VkPhysicalDevice pdevice, VulkanDevice *ldevice, uint8_t image_count)
{
    VulkanSwapchain swapchain = {};

    swapchain.surface     = surface;
    swapchain.pdevice     = pdevice;
    swapchain.ldevice     = ldevice;
    swapchain.image_count = image_count;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &surface_capabilities);
    if (swapchain.image_count > surface_capabilities.maxImageCount) {
        swapchain.image_count = surface_capabilities.maxImageCount;
    }

    uint32_t formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formats_count, 0);

    array<VkSurfaceFormatKHR> formats;
    formats.resize(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formats_count, formats.data());

    for (uint32_t i = 0; i < formats_count; ++i) {
        swapchain.format = formats[i];
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
            break;
        }
    }

    update_swapchain(&swapchain, true);

    return swapchain;
}

void
update_swapchain(VulkanSwapchain *sc, bool vsync)
{
    VkSurfaceKHR     surface       = sc->surface;
    VkSwapchainKHR   old_swapchain = sc->handle;
    VkPhysicalDevice pdevice       = sc->pdevice;
    VulkanDevice    *ldevice       = sc->ldevice;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &surface_capabilities);

    assert(surface_capabilities.currentExtent.width != (uint32_t)(-1));
    VkExtent2D extent = surface_capabilities.currentExtent;

    uint32_t present_modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &present_modes_count, 0);
    array<VkPresentModeKHR> present_modes;
    present_modes.resize(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &present_modes_count, present_modes.data());

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    if (!vsync) {
        for (uint32_t i = 0; i < present_modes_count; ++i) {
            if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    VkSurfaceTransformFlagBitsKHR transform;
    if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        transform = surface_capabilities.currentTransform;
    }

    VkSwapchainCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    create_info.surface                  = sc->surface;
    create_info.minImageCount            = sc->image_count;
    create_info.imageFormat              = sc->format.format;
    create_info.imageColorSpace          = sc->format.colorSpace;
    create_info.imageExtent              = extent;
    create_info.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    create_info.imageArrayLayers         = 1;
    create_info.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create_info.queueFamilyIndexCount    = 1;
    create_info.pQueueFamilyIndices      = &sc->ldevice->graphics_queue_index;
    create_info.preTransform             = transform;
    create_info.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode              = present_mode;
    create_info.clipped                  = VK_TRUE;
    create_info.oldSwapchain             = old_swapchain;

    VkResult result = vkCreateSwapchainKHR(ldevice->handle, &create_info, g_allocator, &sc->handle);
}

void
destroy_swapchain(VulkanSwapchain *sc)
{
}
