#version 450

layout(location = 0) out vec2 o_uv;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	o_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(o_uv * 2.0 - 1.0, 1.0, 1.0);
}
