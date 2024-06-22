#version 450

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_tex_coords;
layout(location = 2) in vec3 i_normal;

layout(location = 0) out vec3 o_frag_pos;
layout(location = 1) out vec2 o_tex_coords;
layout(location = 2) out vec3 o_normal;
layout(location = 3) out vec3 o_view_pos;

layout(binding = 0) uniform globals {
	mat4 proj_matrix;
	mat4 view_matrix;
    vec3 view_pos;
};

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
    mat4 model_matrix = mat4(1.0);
    vec4 world_pos = model_matrix * vec4(i_position, 1.0);

	gl_Position = proj_matrix * view_matrix * world_pos;

    o_frag_pos = world_pos.xyz; 
    o_normal = mat3(transpose(inverse(model_matrix))) * i_normal;
	o_tex_coords = i_tex_coords;
    o_view_pos = view_pos;
}
