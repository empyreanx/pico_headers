@vs vs
@glsl_options flip_vert_y

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

layout(binding = 0) uniform vs_block {
    mat4 u_mvp;
};

out vec4 color;
out vec2 uv;

void main() {
    gl_Position = u_mvp * vec4(a_pos, 1.0);
    color = a_color;
    uv = a_uv;
}

@end

@fs fs

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;

layout (binding = 0) uniform texture2D u_tex;
layout (binding = 1) uniform sampler   u_smp;

out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(u_tex, u_smp), uv) * color;
}

@end

@program sprite vs fs
