#pragma once

#include "base/base.h"

#include "nvulkan/nvulkan.h"

#include "math/math.h"

C_LINKAGE_BEGIN

typedef struct GlobalUniforms  GlobalUniforms;
typedef struct Vertex          Vertex;
typedef struct Material        Material;
typedef struct Materials       Materials;
typedef struct ModelDescriptor ModelDescriptor;
typedef struct Model           Model;
typedef struct SceneRenderer   SceneRenderer;

struct GlobalUniforms {
    Mat4 proj_matrix;
    Mat4 view_matrix;
};

struct Vertex {
    Vec3 position;
    Vec2 tex_coords;
    Vec3 normal;
};

struct Material {
    Vec4 ambient;
    Vec4 diffuse;
    Vec4 specular;
    Vec4 transmittance;
    Vec4 emission;
    f32  shininess;
    f32  ior;
    f32  dissolve;
    s32  illum;
};

struct Materials {
    Material    *materials;
    const char **names;
    u32          count;
    u32          capacity;
    Buffer       buffer;
};

struct ModelDescriptor {
    u64 material_index_buffer_address;
};

struct Model {
    Buffer vertex_buffer;
    Buffer index_buffer;
    Buffer material_index_buffer;
    u32    index_count;
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

void materials_init(Materials *materials);
u32  materials_add(Materials *materials, const char *name, Material mat);
u8   materials_has(Materials *materials, const char *name);
u32  materials_get_index(Materials *materials, const char *name);
void materials_free(Device *ldevice, Materials *materials);
void materials_write_descriptors(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, DescriptorSet *desc_set,
                                 Materials *materials);

ModelDescriptor model_load(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, Model *m, Materials *materials,
                           const char *path);
void            model_free(Device *ldevice, Model *m);

void models_write_descriptors(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, DescriptorSet *desc_set,
                              ModelDescriptor *descriptors, uint32_t descriptor_count);

SceneRenderer scene_renderer_create(VkPhysicalDevice pdevice, Device *ldevice, Swapchain *sc, VkCommandPool cmd_pool,
                                    VkFormat depth_format);
void          scene_renderer_destroy(Device *ldevice, SceneRenderer *r);
void          scene_renderer_render(Swapchain *sc, VkCommandBuffer cmd_buf, SceneRenderer *r, Model *models, u32 model_count,
                                    VkClearValue *clear_colors);
void          scene_renderer_update_uniforms(SceneRenderer *r, VkCommandBuffer cmd_buf, GlobalUniforms *uniforms);

C_LINKAGE_END
