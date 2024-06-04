#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <vulkan/vulkan.h>

VkInstance create_vulkan_instance(const char *name, int version,
                                  const char **extensions, int extensions_count,
                                  const char **layers, int layers_count);

#endif