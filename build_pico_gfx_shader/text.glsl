@vs vs
@glsl_options flip_vert_y

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

layout(binding = 0) uniform vs_block {
    mat4 u_mvp;
};

out vec2 uv;
out vec4 color;

void main() {
    gl_Position = u_mvp * vec4(a_pos, 1.0);
    uv = a_uv;
    color = a_color;
}

@end

@fs fs

layout (location = 0) in vec2 uv;
layout (location = 1) in vec4 color;

layout (binding = 0) uniform texture2D u_tex;
layout (binding = 1) uniform sampler   u_smp;

out vec4 frag_color;

void main() {
    float alpha = texture(sampler2D(u_tex, u_smp), uv).r * color.a;
    frag_color = vec4(color.rgb, alpha);
}

@end

@program text vs fs
