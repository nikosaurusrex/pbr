#version 450

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_tex_coords;
layout(location = 2) in vec3 i_normals;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = vec4(i_position, 1.0);	
}
