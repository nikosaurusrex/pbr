#version 450

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_tex_coords;
layout(location = 2) in vec3 i_normals;

layout(location = 0) out vec2 o_tex_coords;

layout(binding = 0) uniform globals {
	mat4 proj_matrix;
	mat4 view_matrix;
};

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = proj_matrix * view_matrix * vec4(i_position, 1.0);	

	o_tex_coords = i_tex_coords;
}
