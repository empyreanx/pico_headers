@vs vs
@glsl_options flip_vert_y

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

layout(binding = 0) uniform pg_vs {
    mat4 u_proj;
    mat4 u_mv;
};

out vec4 color;
out vec2 uv;

void main() {
    gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);
    color = a_color;
    uv = a_uv;
}

@end

@fs fs

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;

layout (binding = 0) uniform sampler2D u_tex;

out vec4 frag_color;

void main() {
    frag_color = texture(u_tex, uv) * color;
}

@end

@program pg_default vs fs
