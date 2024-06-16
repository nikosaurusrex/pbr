#pragma once

#include "nvulkan/nvulkan.h"

#include "math/math.h"

#ifndef __cplusplus
typedef struct GlobalUniforms GlobalUniforms;
typedef struct Vertex         Vertex;
typedef struct Model          Model;
typedef struct SceneRenderer  SceneRenderer;
#else
extern "C" {
#endif

struct GlobalUniforms {
    Mat4 proj_matrix;
    Mat4 view_matrix;
};

struct Vertex {
    Vec3 position;
    Vec2 tex_coords;
    Vec3 normal;
};

struct Model {
    Buffer   vertex_buffer;
    Buffer   index_buffer;
    uint32_t index_count;
};

struct SceneRenderer {
    Texture       color_image;
    Texture       depth_image;
    VkRenderPass  render_pass;
    VkFramebuffer framebuffer;
    DescriptorSet desc_set;
    Pipeline      pipeline;
    Buffer        uniforms;
};

void model_load(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, Model *m, const char *path);
void model_free(Device *ldevice, Model *m);

SceneRenderer scene_renderer_create(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool,
                                    VkFormat depth_format);
void          scene_renderer_destroy(Device *ldevice, SceneRenderer *r);
void          scene_renderer_render(Swapchain *sc, VkCommandBuffer cmd_buf, SceneRenderer *r, Model *models, uint32_t model_count,
                                    VkClearValue *clear_colors);
void          scene_renderer_update_uniforms(SceneRenderer *r, VkCommandBuffer cmd_buf, GlobalUniforms *uniforms);

#ifdef __cplusplus
}
#endif
