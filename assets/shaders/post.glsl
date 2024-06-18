#version 450

layout(location = 0) in vec2 i_uv;

layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform sampler2D u_tex;

void main() {
	 o_color = pow(texture(u_tex, i_uv).rgba, vec4(1.0 / 2.2));
	// o_color = texture(u_tex, i_uv).rgba;
}