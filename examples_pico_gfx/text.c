/*
    Text Example

    Renders text to the screen using a TrueType font.

    Demonstrates:
     * Setting up an SDL window and GL context
     * Initializing pico_gfx and pico_font
     * Loading a TrueType font
     * Creating a font atlas texture
     * Generating text geometry via pf_draw_text
     * Drawing text with alpha blending
*/

#include <SDL.h>

#include <assert.h>
#include <stdio.h>

#define PICO_FONT_IMPLEMENTATION
#include "../pico_font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define PICO_GFX_GL
#define PICO_GFX_IMPLEMENTATION
#include "../pico_gfx.h"

#define SOKOL_SHDC_IMPL
#include "text_shader.h"

typedef struct
{
    float pos[2];
    float uv[2];
    float color[4];
} vertex_t;

typedef float mat4_t[16];

#define MAX_VERTICES 4096

typedef struct
{
    vertex_t vertices[MAX_VERTICES];
    int count;
    float color[4];
} draw_ctx_t;

unsigned char* read_file(const char* filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *data = malloc(fsize + 1);

    if (data)
    {
        fread(data, fsize, 1, f);
        data[fsize] = 0; // Null-terminate
    }

    fclose(f);

    return data;
}

// Callback invoked per glyph quad by pf_draw_text
static bool draw_callback(const pf_quad_t* quad, void* user)
{
    draw_ctx_t* dc = (draw_ctx_t*)user;

    if (dc->count + 6 > MAX_VERTICES)
        return false;

    vertex_t* v = &dc->vertices[dc->count];
    const float* c = dc->color;

    // Triangle 1
    v[0] = (vertex_t){ {quad->x0, quad->y0}, {quad->u0, quad->v0}, {c[0], c[1], c[2], c[3]} };
    v[1] = (vertex_t){ {quad->x0, quad->y1}, {quad->u0, quad->v1}, {c[0], c[1], c[2], c[3]} };
    v[2] = (vertex_t){ {quad->x1, quad->y1}, {quad->u1, quad->v1}, {c[0], c[1], c[2], c[3]} };

    // Triangle 2
    v[3] = (vertex_t){ {quad->x0, quad->y0}, {quad->u0, quad->v0}, {c[0], c[1], c[2], c[3]} };
    v[4] = (vertex_t){ {quad->x1, quad->y1}, {quad->u1, quad->v1}, {c[0], c[1], c[2], c[3]} };
    v[5] = (vertex_t){ {quad->x1, quad->y0}, {quad->u1, quad->v0}, {c[0], c[1], c[2], c[3]} };

    dc->count += 6;
    return true;
}

typedef struct
{
    pg_ctx_t* ctx;
    pg_texture_t* tex;
} upload_ctx_t;

// Callback invoked per dirty atlas page by pf_upload_atlas
static bool upload_callback(size_t page, const unsigned char* pixels,
                           int width, int height, void* user)
{
    (void)page;

    upload_ctx_t* uc = (upload_ctx_t*)user;

    if (!uc->tex)
    {
        uc->tex = pg_create_texture(uc->ctx, width, height,
                                    PG_PIXEL_FORMAT_RED, pixels,
                                    (size_t)(width * height), NULL);
    }
    else
    {
        pg_update_texture(uc->tex, (char*)pixels, width, height);
    }

    return true;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    assert(pg_backend() == PG_BACKEND_GL);

    printf("Text rendering demo\n");

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Text Example",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1024, 768,
                                          SDL_WINDOW_OPENGL);

    int pixel_w, pixel_h;
    SDL_GL_GetDrawableSize(window, &pixel_w, &pixel_h);

    SDL_GL_SetSwapInterval(1);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    pg_init();

    // Initialize context
    pg_ctx_t* ctx = pg_create_context(pixel_w, pixel_h, NULL);
    pg_shader_t* shader = pg_create_shader(ctx, text);

    // Load TTF font
    unsigned char* ttf = read_file("devroye.ttf");
    assert(ttf);

    // Create font atlas and face
    pf_atlas_t* atlas = pf_create_atlas(512, 512);
    pf_face_t* face = pf_create_face(atlas, ttf, 48.0f);

    // Generate text geometry using pico_font
    draw_ctx_t draw = { .color = { 1.0f, 1.0f, 1.0f, 1.0f } };

    // Measure both lines to center the text block
    pf_metrics_t metrics;
    pf_get_metrics(face, &metrics);

    float line1_w, line2_w;
    pf_measure_text(face, "Hello, World!", &line1_w, NULL);
    pf_measure_text(face, "Pico Font + Pico GFX", &line2_w, NULL);

    float text_w = line1_w > line2_w ? line1_w : line2_w;
    float text_h = metrics.line_height * 2;

    float start_x = pixel_w / 2.f - text_w / 2.f;
    float x = start_x;
    float y = pixel_h / 2.f - text_h / 2.f;

    // Line 1: "Hello" in red, ", World!" in white
    draw.color[0] = 1.f;
    draw.color[1] = 0.f;
    draw.color[2] = 0.f;
    draw.color[3] = 1.f;

    pf_draw_text(face, "Hello", &x, &y, draw_callback, &draw);

    draw.color[0] = 1.f;
    draw.color[1] = 1.f;
    draw.color[2] = 1.f;
    draw.color[3] = 1.f;

    pf_draw_text(face, ", World!", &x, &y, draw_callback, &draw);

    // Line 2: manual line break
    x = start_x;
    y += metrics.line_height;

    pf_draw_text(face, "pico_font + pico_gfx", &x, &y, draw_callback, &draw);

    // Upload font atlas to GPU texture
    upload_ctx_t upload = { .ctx = ctx, .tex = NULL };
    pf_upload_atlas(atlas, upload_callback, &upload);
    assert(upload.tex);

    // Create vertex buffer from text geometry
    pg_buffer_t* vertex_buffer = pg_create_vertex_buffer(ctx,
        PG_USAGE_STATIC, draw.vertices, draw.count, draw.count,
        sizeof(vertex_t));

    // Pipeline with alpha blending for text rendering
    pg_pipeline_t* pipeline = pg_create_pipeline(ctx, shader, &(pg_pipeline_opts_t)
    {
        .layout =
        {
            .attrs =
            {
                [ATTR_text_a_pos] = { .format = PG_VERTEX_FORMAT_FLOAT2,
                                      .offset = offsetof(vertex_t, pos) },

                [ATTR_text_a_uv]  = { .format = PG_VERTEX_FORMAT_FLOAT2,
                                      .offset = offsetof(vertex_t, uv) },

                [ATTR_text_a_color] = { .format = PG_VERTEX_FORMAT_FLOAT4,
                                        .offset = offsetof(vertex_t, color) },
            },
        },

        .blend_enabled = true,
        .blend =
        {
            .color_src = PG_SRC_ALPHA,
            .color_dst = PG_ONE_MINUS_SRC_ALPHA,
            .color_eq  = PG_ADD,
            .alpha_src = PG_ONE,
            .alpha_dst = PG_ONE_MINUS_SRC_ALPHA,
            .alpha_eq  = PG_ADD,
        },
    });

    // Orthographic projection mapping pixel coordinates to clip space
    float w = (float)pixel_w;
    float h = (float)pixel_h;

    vs_block_t vs_block =
    {
        .u_mvp =
        {
            2.0f/w, 0.0f,   0.0f, 0.0f,
            0.0f,   2.0f/h, 0.0f, 0.0f,
            0.0f,   0.0f,  -1.0f, 0.0f,
           -1.0f,  -1.0f,   0.0f, 1.0f,
        }
    };

    // Set uniform blocks
    pg_set_uniform_block(shader, "vs_block", &vs_block);

    // Create sampler and bind texture
    pg_sampler_t* sampler = pg_create_sampler(ctx, NULL);
    pg_bind_sampler(shader, "u_smp", sampler);
    pg_bind_texture(shader, "u_tex", upload.tex);

    // Set the pipeline
    pg_set_pipeline(ctx, pipeline);

    // The main loop
    bool done = false;

    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    done = true;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
                    break;
            }
        }

        // Draw text to the screen
        pg_begin_pass(ctx, NULL, true);

        pg_bind_buffer(ctx, 0, vertex_buffer);
        pg_draw(ctx, 0, draw.count, 1);

        pg_end_pass(ctx);

        // Flush draw commands
        pg_flush(ctx);

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    pg_destroy_texture(upload.tex);
    pg_destroy_sampler(sampler);

    pg_destroy_buffer(vertex_buffer);

    pg_destroy_pipeline(pipeline);
    pg_destroy_shader(shader);
    pg_destroy_context(ctx);

    pg_shutdown();

    pf_destroy_face(face);
    pf_destroy_atlas(atlas);
    free(ttf);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

