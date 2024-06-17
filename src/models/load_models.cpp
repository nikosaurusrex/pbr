#include "models.h"

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

ModelDescriptor
model_load(VkPhysicalDevice pdevice, Device *ldevice, VkCommandPool cmd_pool, Model *m, Materials *materials, const char *path)
{
    uint32_t  vertex_count         = 0;
    Vertex   *vertices             = 0;
    uint32_t  index_count          = 0;
    uint32_t *indices              = 0;
    uint32_t  material_index_count = 0;
    uint32_t *material_indices     = 0;

    tinyobj::ObjReader reader;
    reader.ParseFromFile(path);

    const tinyobj::attrib_t &attrib = reader.GetAttrib();

    // because we store all materials in a single array, local material index have to be mapped to global material index
    std::unordered_map<int, uint32_t> mat_index_map = {};
    mat_index_map.insert({-1, 0}); // means that if no material, then use default material

    uint32_t mat_index = 0;
    for (const auto &obj_mat : reader.GetMaterials()) {
        uint32_t existing_index = materials_get_index(materials, obj_mat.name.c_str());
        if (existing_index != UINT32_MAX) {
            mat_index_map[mat_index] = existing_index;
            mat_index++;
            continue;
        }

        Material mat = {};

        mat.ambient       = vec4(obj_mat.ambient[0], obj_mat.ambient[1], obj_mat.ambient[2], 1.0f);
        mat.diffuse       = vec4(obj_mat.diffuse[0], obj_mat.diffuse[1], obj_mat.diffuse[2], 1.0f);
        mat.specular      = vec4(obj_mat.specular[0], obj_mat.specular[1], obj_mat.specular[2], 1.0f);
        mat.transmittance = vec4(obj_mat.transmittance[0], obj_mat.transmittance[1], obj_mat.transmittance[2], 1.0f);
        mat.emission      = vec4(obj_mat.emission[0], obj_mat.emission[1], obj_mat.emission[2], 1.0f);
        mat.shininess     = obj_mat.shininess;
        mat.ior           = obj_mat.ior;
        mat.dissolve      = obj_mat.dissolve;
        mat.illum         = obj_mat.illum;

        uint32_t index_of_added = materials_add(materials, obj_mat.name.c_str(), mat);

        mat_index_map[mat_index] = index_of_added;
    }

    for (const auto &shape : reader.GetShapes()) {
        vertices = (Vertex *)realloc(vertices, (shape.mesh.indices.size() + vertex_count) * sizeof(Vertex));
        indices  = (uint32_t *)realloc(indices, (shape.mesh.indices.size() + index_count) * sizeof(uint32_t));
        material_indices =
            (uint32_t *)realloc(material_indices, (shape.mesh.material_ids.size() + material_index_count) * sizeof(uint32_t));

        for (uint32_t i = 0; i < shape.mesh.material_ids.size(); i++) {
            material_indices[material_index_count + i] = mat_index_map[shape.mesh.material_ids[i]];
        }

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
        material_index_count += shape.mesh.material_ids.size();
    }

    m->vertex_buffer =
        buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, (void *)vertices, vertex_count * sizeof(Vertex));
    m->index_buffer =
        buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, (void *)indices, index_count * sizeof(uint32_t));
    m->material_index_buffer =
        buffer_create(pdevice, ldevice, cmd_pool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                      (void *)material_indices, material_index_count * sizeof(uint32_t));
    m->index_count = index_count;

    free(vertices);
    free(indices);
    free(material_indices);

    ModelDescriptor descriptor = {};

    VkBufferDeviceAddressInfo address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
    address_info.buffer                    = m->material_index_buffer.handle;

    descriptor.material_index_buffer_address = vkGetBufferDeviceAddress(ldevice->handle, &address_info);

    return descriptor;
}

void
model_free(Device *ldevice, Model *m)
{
    buffer_destroy(ldevice, &m->vertex_buffer);
    buffer_destroy(ldevice, &m->index_buffer);
    buffer_destroy(ldevice, &m->material_index_buffer);
}
