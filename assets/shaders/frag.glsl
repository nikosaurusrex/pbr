#version 450

layout(location = 0) out vec4 o_color;

// @Todo: check if these extensions are supported
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : enable

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

layout(buffer_reference, scalar) buffer MaterialIndices {int i[]; }; 

void main() {
    ModelDescription description = modelDescriptionsBuffer.descriptions[0];
    MaterialIndices material_indices = MaterialIndices(description.material_indices_address);

    int mat_index = material_indices.i[gl_PrimitiveID];
    Material material = materialsBuffer.materials[mat_index];

	o_color = vec4(material.diffuse.xyz, 1.0);
}
