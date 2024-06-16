#version 450

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_tex_coords;
layout(location = 2) in vec3 i_normals;

layout(binding = 0) uniform globals {
	mat4 proj_matrix;
};

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = proj_matrix * vec4(i_position, 1.0);	
}
