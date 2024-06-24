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

    U32 image_count;
    U32 semaphore_count;

    VkImage              *images;
    VkImageView          *image_views;
    VkImageMemoryBarrier *barriers;
    VkSemaphore          *read_semaphores;
    VkSemaphore          *write_semaphores;
    VkFence              *fences;

    U32 width;
    U32 height;
    U32 current_semaphore;
    U32 current_image;
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
    U32            count;
};

struct CommandBuffers {
    VkCommandBuffer *handles;
    U32              count;
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
    U32                                shader_count;
    VkVertexInputBindingDescription   *binding_descriptions;
    U32                                binding_description_count;
    VkVertexInputAttributeDescription *attribute_descriptions;
    U32                                attribute_description_count;
};

struct Buffer {
    VkBuffer       handle;
    VkDeviceMemory memory;
};

VkInstance vulkan_instance_create(const char *name, int version, const char **extensions, U32 extension_count, const char **layers,
                                  U32 layer_count);
void       vulkan_instance_destroy(VkInstance instance);

VkSurfaceKHR surface_create(VkInstance instance, GLFWwindow *glfw_window);
void         surface_destroy(VkSurfaceKHR surface, VkInstance instance);

VkPhysicalDevice physical_device_find_compatible(VkInstance instance, const char **required_extensions, U32 required_extension_count);

Device logical_device_create(VkSurfaceKHR surface, VkPhysicalDevice pdevice, const char **extensions, U32 extension_count,
                             const char **layers, U32 layer_count);
void   logical_device_destroy(Device *ldevice);

Swapchain swapchain_create(U32 image_count, VkSurfaceKHR surface, VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool);
void      swapchain_update(Swapchain *sc, VkCommandPool cmd_pool, B8 vsync);
void      swapchain_destroy(Swapchain *sc);
U32       swapchain_acquire(Swapchain *sc);
void      swapchain_present(Swapchain *sc, CommandBuffers *cmd_bufs);

VkCommandPool command_pool_create(Device *ldevice);
void          command_pool_destroy(VkCommandPool cmd_pool, Device *ldevice);

VkCommandBuffer command_buffer_allocate(Device *ldevice, VkCommandPool cmd_pool);
void            command_buffer_free(VkCommandBuffer cmd_buf, Device *ldevice, VkCommandPool cmd_pool);
void            command_buffer_begin(VkCommandBuffer cmd_buf);
void            command_buffer_end(VkCommandBuffer cmd_buf);
void            command_buffer_submit(VkCommandBuffer cmd_buf, Device *ldevice);

CommandBuffers command_buffers_allocate(Device *ldevice, VkCommandPool cmd_pool, U32 count);
void           command_buffers_free(CommandBuffers *cmd_bufs, Device *ldevice, VkCommandPool cmd_pool);

Image image_create(U32 width, U32 height, VkFormat format, U32 mip_levels, VkImageAspectFlags aspect_mask, VkImageUsageFlags usage,
                   VkPhysicalDevice pdevice, Device *ldevice);
void  image_destroy(Image *image, Device *ldevice);
Image image_create_depth(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool);

Texture texture_create(U32 width, U32 height, VkFormat format, VkImageAspectFlags aspect_mask, VkImageUsageFlags usage,
                       VkPhysicalDevice pdevice, Device *ldevice);
Texture texture_from_pixels(U32 width, U32 height, U32 channels, VkFormat format, U8 *pixels, VkSamplerCreateInfo sampler_info,
                            VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool);
void    texture_destroy(Texture *t, Device *ldevice);

void  image_transition_layout(Image *image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_mask,
                              VkCommandBuffer cmd_buf);

VkRenderPass render_pass_create_present(VkFormat color_format, VkFormat depth_format, Device *ldevice);
VkRenderPass render_pass_create_offscreen(VkFormat color_format, VkFormat depth_format, Device *ldevice);
void         render_pass_destroy(VkRenderPass render_pass, Device *ldevice);

VkFramebuffer frame_buffer_create(Swapchain *sc, VkRenderPass render_pass, VkImageView color_view, VkImageView depth_view);
void          frame_buffer_destroy(VkFramebuffer framebuffer, Device *ldevice);

Framebuffers frame_buffers_create(Swapchain *sc, VkRenderPass render_pass, Image *depth_image);
void         frame_buffers_destroy(Framebuffers *framebuffers, Device *ldevice);

DescriptorSet descriptor_set_create(VkDescriptorSetLayoutBinding *bindings, U32 binding_count, Device *ldevice);
void          descriptor_set_destroy(DescriptorSet *descriptor_set, Device *ldevice);

Pipeline pipeline_create(Device *ldevice, DescriptorSet *desc_set, VkRenderPass render_pass, Shader *shaders, U32 shader_count,
                         VkVertexInputBindingDescription *binding_descriptions, U32 binding_description_count,
                         VkVertexInputAttributeDescription *attribute_descriptions, U32 attribute_description_count, U32 cull_mode);
void     pipeline_destroy(Pipeline *pipeline, Device *ldevice);

Buffer buffer_create(VkDeviceSize size, void *data, VkBufferUsageFlags usage, VkPhysicalDevice pdevice, Device *ldevice,
                     VkCommandPool cmd_pool);
Buffer buffer_create_staging(VkDeviceSize size, void *data, VkPhysicalDevice pdevice, Device *ldevice);
void   buffer_destroy(Buffer *buffer, Device *ldevice);

// @Todo move somewhere
static inline U32
memory_type_find(VkPhysicalDevice pdevice, U32 type_bits, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(pdevice, &memory_properties);

    for (U32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if ((type_bits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

    // @Todo error handling
    return 0;
}

C_LINKAGE_END
