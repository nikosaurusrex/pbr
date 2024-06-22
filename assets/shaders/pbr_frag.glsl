// Resources
// https://learnopengl.com/PBR/Lighting
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

#version 450

// do we need this?
precision highp float;

layout(location = 0) in vec3 i_frag_pos;
layout(location = 1) in vec2 i_tex_coords;
layout(location = 2) in vec3 i_normal;
// @Todo I want this to a uniform here, together with model id
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

vec3 fresnel_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float N_dot_H  = max(dot(N, H), 0.0);
    float N_dot_H_2 = N_dot_H * N_dot_H;
	
    float num   = a2;
    float denom = (N_dot_H_2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float geometry_schlick_ggx(float N_dot_wo, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = N_dot_wo;
    float denom = N_dot_wo * (1.0 - k) + k;
	
    return num / denom;
}

float geometry_smith(vec3 N, vec3 wo, vec3 wi, float roughness) {
    float N_dot_wo = max(dot(N, wo), 0.0);
    float N_dot_wi = max(dot(N, wi), 0.0);
    float ggx2  = geometry_schlick_ggx(N_dot_wo, roughness);
    float ggx1  = geometry_schlick_ggx(N_dot_wi, roughness);
	
    return ggx1 * ggx2;
}

void main() {
    ModelDescription description     = modelDescriptionsBuffer.descriptions[0];
    MaterialIndices material_indices = MaterialIndices(description.material_indices_address);

    int mat_index = material_indices.i[gl_PrimitiveID];
    Material m    = materialsBuffer.materials[mat_index];

    vec3 light_pos   = vec3(5.0, 5.0, 3.0);
    vec3 light_color = vec3(1.0) * 300;

    vec3 albedo     = m.albedo.rgb;
    float metallic  = m.metallic;
    float specular  = m.specular;
    float roughness = m.roughness;

    vec3 N  = normalize(i_normal);
    vec3 wi = normalize(light_pos - i_frag_pos);
    vec3 wo = normalize(i_view_pos - i_frag_pos);
    vec3 H  = normalize(wo + wi);

    float distance     = length(light_pos - i_frag_pos);
    float attentuation = 1.0 / (distance * distance);
    vec3 radiance      = light_color * attentuation;

    float cos_theta = dot(wi, N);

    // calculate fresnel using fresnel-schlick approximation
    vec3 F0 = mix(vec3(0.04), albedo, metallic); // the base reflectivity of the surface
    vec3 F  = fresnel_schlick(max(dot(H, wo), 0.0), F0);

    float ndf = distribution_ggx(N, H, roughness); // normal distribution function
    float G   = geometry_smith(N, wo, wi, roughness); // geometry function

    // calculate Cook-Torrance BRDF
    vec3 numerator    = ndf * G * F;
    float denominator = 4.0 * max(dot(N, wo), 0.0) * max(dot(N, wi), 0.0) + 0.0001; // to avoid division by 0

    vec3 f_cook_torrance = numerator / denominator;

    vec3 ks = F;
    vec3 kd = (vec3(1.0) - ks) * (1.0 - metallic);

    vec3 f_lambert = albedo.rgb * (1.0 / PI); // Lambertian diffuse
    
    vec3 fr = kd * f_lambert + f_cook_torrance;
    
    vec3 lo = fr * radiance * max(dot(N, wi), 0.0);

    vec3 ambient = vec3(0.03) * albedo;

	vec3 color = ambient + lo;

    // vec3 color = radiance;
    color = color / (color + vec3(1.0));
    o_color = vec4(color, 1.0);
}
