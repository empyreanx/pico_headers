@vs vs

uniform vs_params {
    mat4 u_mvp;
};

in vec2 a_pos;
in vec2 a_uv;

in vec2 a_inst_pos;
in vec4 a_inst_color;

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

in vec4 color;
in vec2 uv;

uniform texture2D u_tex;
uniform sampler   u_smp;

out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(u_tex, u_smp), uv) * color;
}

@end

@program particle vs fs
