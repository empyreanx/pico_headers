@vs vs

layout(binding = 0) uniform vs_params {
    mat4 u_mvp;
};

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

layout(location = 2) in vec2 a_inst_pos;
layout(location = 3) in vec4 a_inst_color;

out vec4 color;
out vec2 uv;

void main() {
    vec4 pos = vec4(a_pos + a_inst_pos, 0.0, 1.0);
    gl_Position = u_mvp * pos;
    uv = a_uv;
    color = a_inst_color;
}
@end

@fs fs

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;

layout (binding = 0) uniform texture2D u_tex;
layout (binding = 1) uniform sampler   u_smp;

out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(u_tex, u_smp), uv) * color;
}

@end

@program particle vs fs
