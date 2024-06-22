#version 450

layout(location = 0) in vec3 i_frag_pos;
layout(location = 1) in vec2 i_tex_coords;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in vec3 i_view_pos;

layout(location = 0) out vec4 o_color;

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct Material {
    vec4 albedo;
    float metallic;
    float specular;
    float roughness;
};

struct ModelDescription {
    uint64_t material_indices_address;
};

layout(std140, set=0, binding=1) readonly buffer MaterialsBuffer {
	Material materials[];
} materialsBuffer;

layout(std140, set=0, binding=2) readonly buffer ModelDescriptionsBuffer {
    ModelDescription descriptions[];
} modelDescriptionsBuffer;

layout(binding = 3) uniform sampler2D[] textureSamplers;

layout(buffer_reference, scalar) buffer MaterialIndices {int i[]; }; 

#define PI 3.14159265359

void main() {
    ModelDescription description     = modelDescriptionsBuffer.descriptions[0];
    MaterialIndices material_indices = MaterialIndices(description.material_indices_address);

    int mat_index = material_indices.i[gl_PrimitiveID];
    Material m    = materialsBuffer.materials[mat_index];

    vec3 light_pos = vec3(5.0, 5.0, 3.0);
    vec3 light_color = vec3(1.0) * 20;

    vec3 N = normalize(i_normal);
    vec3 V = normalize(i_view_pos - i_frag_pos);
    vec3 L = normalize(light_pos - i_frag_pos);
    float cos_theta = dot(L, N);

    float distance = length(light_pos - i_frag_pos);
    float attentuation = 1.0 / (distance * distance);
    vec3 radiance = light_color * attentuation;

	o_color = vec4(radiance, 1.0);
}
