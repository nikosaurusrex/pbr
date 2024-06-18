#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include "base/base.h"

C_LINKAGE_BEGIN

#define VK_CHECK(call)                                                                                                                     \
    if (call != VK_SUCCESS) {                                                                                                              \
        fprintf(stderr, "Vulkan call failed at %s:%d\n", __FILE__, __LINE__);                                                              \
        exit(1);                                                                                                                           \
    }

typedef struct Device         Device;
typedef struct Swapchain      Swapchain;
typedef struct Image          Image;
typedef struct Texture        Texture;
typedef struct Framebuffers   Framebuffers;
typedef struct CommandBuffers CommandBuffers;
typedef struct DescriptorSet  DescriptorSet;
typedef struct Shader         Shader;
typedef struct Pipeline       Pipeline;
typedef struct Buffer         Buffer;

struct Device {
    VkDevice handle;
    VkQueue  graphics_queue;
    uint32_t graphics_queue_index;
};

struct Swapchain {
    VkSwapchainKHR     handle;
    VkSurfaceKHR       surface;
    Device            *ldevice;
    VkPhysicalDevice   pdevice;
    VkSurfaceFormatKHR format;

    u32 image_count;
    u32 semaphore_count;

    VkImage              *images;
    VkImageView          *image_views;
    VkImageMemoryBarrier *barriers;
    VkSemaphore          *read_semaphores;
    VkSemaphore          *write_semaphores;
    VkFence              *fences;

    u32 width;
    u32 height;
    u32 current_semaphore;
    u32 current_image;
};

struct Image {
    VkImage        handle;
    VkImageView    view;
    VkDeviceMemory memory;
    VkFormat       format;
};

struct Texture {
    Image                 image;
    VkDescriptorImageInfo descriptor;
};

struct Framebuffers {
    VkFramebuffer *handles;
    u32            count;
};

struct CommandBuffers {
    VkCommandBuffer *handles;
    u32              count;
};

struct DescriptorSet {
    VkDescriptorSet       handle;
    VkDescriptorSetLayout layout;
    VkDescriptorPool      pool;
};

struct Shader {
    const char *path;
    uint32_t    stage;
};

struct Pipeline {
    VkPipeline                         handle;
    VkPipelineLayout                   layout;
    VkPipelineShaderStageCreateInfo   *shader_stages;
    u32                                shader_count;
    VkVertexInputBindingDescription   *binding_descriptions;
    u32                                binding_description_count;
    VkVertexInputAttributeDescription *attribute_descriptions;
    u32                                attribute_description_count;
};

struct Buffer {
    VkBuffer       handle;
    VkDeviceMemory memory;
};

VkInstance vulkan_instance_create(const char *name, int version, const char **extensions, u32 extension_count, const char **layers,
                                  u32 layer_count);
void       vulkan_instance_destroy(VkInstance instance);

VkSurfaceKHR surface_create(VkInstance instance, GLFWwindow *glfw_window);
void         surface_destroy(VkInstance instance, VkSurfaceKHR surface);

VkPhysicalDevice physical_device_find_compatible(VkInstance instance, const char **required_extensions, u32 required_extension_count);

Device logical_device_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, const char **extensions, u32 extension_count,
                             const char **layers, u32 layer_count);
void   logical_device_destroy(Device *ldevice);

Swapchain swapchain_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, u32 image_count);
void      swapchain_update(Swapchain *sc, VkCommandPool cmd_pool, b8 vsync);
void      swapchain_destroy(Swapchain *sc);
uint32_t  swapchain_acquire(Swapchain *sc);
void      swapchain_present(Swapchain *sc, CommandBuffers *cmd_bufs);

VkCommandPool command_pool_create(Device *ldevice);
void          command_pool_destroy(Device *ldevice, VkCommandPool cmd_pool);

VkCommandBuffer command_buffer_allocate(Device *ldevice, VkCommandPool cmd_pool);
void            command_buffer_free(Device *ldevice, VkCommandPool cmd_pool, VkCommandBuffer cmd_buf);
void            command_buffer_begin(VkCommandBuffer cmd_buf);
void            command_buffer_end(VkCommandBuffer cmd_buf);
void            command_buffer_submit(Device *ldevice, VkCommandBuffer cmd_buf);

CommandBuffers command_buffers_allocate(Device *ldevice, VkCommandPool cmd_pool, u32 count);
void           command_buffers_free(Device *ldevice, VkCommandPool cmd_pool, CommandBuffers *cmd_bufs);

Image image_create(VkPhysicalDevice pdevice, Device *ldevice, VkFormat format, u32 width, u32 height, u32 mip_levels,
                   VkImageAspectFlags aspect_mask, VkImageUsageFlags usage);
void  image_destroy(Device *ldevice, Image *image);
void  image_transition_layout(VkCommandBuffer cmd_buf, Image *image, VkImageLayout old_layout, VkImageLayout new_layout,
                              VkImageAspectFlags aspect_mask);
Image image_create_depth(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool);

Texture texture_create(VkPhysicalDevice pdevice, Device *ldevice, VkFormat format, u32 width, u32 height, VkImageAspectFlags aspect_mask,
                       VkImageUsageFlags usage);
void    texture_destroy(Device *ldevice, Texture *t);

VkRenderPass render_pass_create_present(Device *ldevice, VkFormat color_format, VkFormat depth_format);
VkRenderPass render_pass_create_offscreen(Device *ldevice, VkFormat color_format, VkFormat depth_format);
void         render_pass_destroy(Device *ldevice, VkRenderPass render_pass);

VkFramebuffer frame_buffer_create(Swapchain *sc, VkRenderPass render_pass, VkImageView color_view, VkImageView depth_view);
void          frame_buffer_destroy(Device *ldevice, VkFramebuffer framebuffer);

Framebuffers frame_buffers_create(Swapchain *sc, VkRenderPass render_pass, Image *depth_image);
void         frame_buffers_destroy(Device *ldevice, Framebuffers *framebuffers);

DescriptorSet descriptor_set_create(Device *ldevice, VkDescriptorSetLayoutBinding *bindings, u32 binding_count);
void          descriptor_set_destroy(Device *ldevice, DescriptorSet *descriptor_set);

Pipeline pipeline_create(Device *ldevice, DescriptorSet *desc_set, VkRenderPass render_pass, Shader *shaders, u32 shader_count,
                         VkVertexInputBindingDescription *binding_descriptions, u32 binding_description_count,
                         VkVertexInputAttributeDescription *attribute_descriptions, u32 attribute_description_count, u32 cull_mode);
void     pipeline_destroy(Device *ldevice, Pipeline *pipeline);

Buffer buffer_create(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, VkBufferUsageFlags usage, void *data,
                     VkDeviceSize size);
void   buffer_destroy(Device *ldevice, Buffer *buffer);

// @Todo move somewhere
static inline u32
memory_type_find(VkPhysicalDevice pdevice, u32 type_bits, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(pdevice, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if ((type_bits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

    // @Todo error handling
    return 0;
}

C_LINKAGE_END
