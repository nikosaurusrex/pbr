#include "models.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void
model_load(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, Model *m, const char *path)
{
    uint32_t  vertex_count = 0;
    Vertex   *vertices     = 0;
    uint32_t  index_count  = 0;
    uint32_t *indices      = 0;

    tinyobj::ObjReader reader;
    reader.ParseFromFile(path);

    const tinyobj::attrib_t &attrib = reader.GetAttrib();

    for (const auto &shape : reader.GetShapes()) {
        vertices = (Vertex *)realloc(vertices, (shape.mesh.indices.size() + vertex_count) * sizeof(Vertex));
        indices  = (uint32_t *)realloc(indices, (shape.mesh.indices.size() + index_count) * sizeof(uint32_t));

        uint32_t vertex_index  = vertex_count;
        uint32_t indices_index = index_count;
        for (const auto &index : shape.mesh.indices) {
            Vertex       vertex = {};
            const float *vp     = &attrib.vertices[3 * index.vertex_index];
            vertex.position     = {*(vp + 0), *(vp + 1), *(vp + 2)};

            if (!attrib.normals.empty() && index.normal_index >= 0) {
                const float *np = &attrib.normals[3 * index.normal_index];
                vertex.normal   = {*(np + 0), *(np + 1), *(np + 2)};
            }

            if (!attrib.texcoords.empty() && index.texcoord_index >= 0) {
                const float *tp   = &attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.tex_coords = {*tp, 1.0f - *(tp + 1)};
            }

            vertices[vertex_index++] = vertex;
            indices[indices_index++] = indices_index;
        }

        vertex_count += shape.mesh.indices.size();
        index_count += shape.mesh.indices.size();
    }

    m->vertex_buffer =
        buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, (void *)vertices, vertex_count * sizeof(Vertex));
    m->index_buffer =
        buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, (void *)indices, index_count * sizeof(uint32_t));
    m->index_count = index_count;
}

void
model_free(Device *ldevice, Model *m)
{
    buffer_destroy(ldevice, &m->vertex_buffer);
    buffer_destroy(ldevice, &m->index_buffer);
}
