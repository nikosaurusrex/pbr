#version 450

layout(location = 0) in vec2 i_tex_coord;

layout(location = 0) out vec4 o_color;

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct Material {
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    vec4  transmittance;
    vec4  emission;
    float shininess;
    float ior;
    float dissolve;
    int   illum;
    int   texture_offset;
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

vec4 standard_shading(vec4 ambient, vec4 diffuse, vec4 specular, vec3 normal, vec3 view) {
    float NoV = clamp(abs(dot(normal, view)) + 1e-5, 0.0, 1.0);

    vec4 diffuse_lambert = diffuse * (1.0 / PI);

    return vec4(diffuse_lambert.xyz, 1.0);
}

void main() {
    ModelDescription description     = modelDescriptionsBuffer.descriptions[0];
    MaterialIndices material_indices = MaterialIndices(description.material_indices_address);

    int mat_index = material_indices.i[gl_PrimitiveID];
    Material m    = materialsBuffer.materials[mat_index];

    vec4 diffuse = m.diffuse;
    if (m.texture_offset > 0) {
        vec4 tex = texture(textureSamplers[m.texture_offset], i_tex_coord);

        diffuse *= tex;
    }

	o_color = standard_shading(m.ambient, diffuse, m.specular, vec3(0.0), vec3(0.0));
}
