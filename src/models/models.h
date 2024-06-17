#pragma once

#include <stdalign.h>

#include "nvulkan/nvulkan.h"

#include "math/math.h"

#ifndef __cplusplus
typedef struct GlobalUniforms GlobalUniforms;
typedef struct Vertex         Vertex;
typedef struct Material       Material;
typedef struct Materials      Materials;
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

struct Material
{
    Vec4  ambient;
    Vec4  diffuse;
    Vec4  specular;
    Vec4  transmittance;
    Vec4  emission;
    float shininess;
    float ior;
    float dissolve;
    int   illum;
};

struct Materials {
    Material    *materials;
    const char **names;
    uint32_t     count;
    uint32_t     capacity;
    Buffer       buffer;
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

void     materials_init(Materials *materials);
uint32_t materials_add(Materials *materials, const char *name, Material mat);
uint8_t  materials_has(Materials *materials, const char *name);
uint32_t materials_get_index(Materials *materials, const char *name);
void     materials_free(Materials *materials);
void     materials_write_descriptors(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, DescriptorSet *desc_set,
                                     Materials *materials);

void model_load(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, Model *m, Materials *materials, const char *path);
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
