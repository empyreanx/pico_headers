/*
    Quad Example

    Draws an image to the screen.

    Demonstrates:
     * Setting up and SDL window and GL context
     * Initializing pico_gfx
     * Loading an image
     * Creating a texture from the image
     * Defining vertices
     * Drawing the vertices
*/
#define _POSIX_C_SOURCE 199309L

#include <SDL.h>

#define PICO_TIME_IMPLEMENTATION
#include "../pico_time.h"

#include <assert.h>
#include <stdio.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PICO_GFX_GL
#define PICO_GFX_IMPLEMENTATION
#include "../pico_gfx.h"

#define SOKOL_SHDC_IMPL
#include "particle_shader.h"

#define MAX_PARTICLES (512 * 1024)
#define NUM_PARTICLES_EMITTED_PER_FRAME (10)

#define PI_F 3.1415927f

typedef struct
{
    float pos[2];
    float uv[2];
} vertex_t;

typedef struct
{
    float pos[2];
    float color[4];
    float vel[2];
} particle_t;

static struct
{
    particle_t particles[MAX_PARTICLES];
    int particle_count;
} state;

float random(float min, float max)
{
    return ((float)rand() / RAND_MAX) * (max - min) + min;
}

typedef float vec2_t[2];

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    memset(&state, 0, sizeof(state));

    assert(pg_backend() == PG_BACKEND_GL);

    printf("Quad rendering demo\n");

    stbi_set_flip_vertically_on_load(true);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_CONTEXT_PROFILE_CORE);

    int win_w = 1024;
    int win_h = 768;

    SDL_Window* window = SDL_CreateWindow("Quad Example",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          win_w, win_h,
                                          SDL_WINDOW_OPENGL);

    int pixel_w, pixel_h;
    SDL_GL_GetDrawableSize(window, &pixel_w, &pixel_h);

    SDL_GL_SetSwapInterval(1);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    pg_init();

    // Initialize context
    pg_ctx_t* ctx = pg_create_context(pixel_w, pixel_h, NULL);
    pg_shader_t* shader = pg_create_shader(ctx, particle);

    // Load image

    int w, h, c;
    unsigned char* bitmap = stbi_load("./circle.png", &w, &h, &c, 0);

    assert(bitmap);

    // Load texture

    size_t size = w * h * c;
    pg_texture_t* tex = pg_create_texture(ctx, w, h, bitmap, size, NULL);

    assert(tex && c == 4);

    free(bitmap);

    // Specify vertices

    vertex_t vertices[6] =
    {
        { { 0, 0, }, { 0, 1 } },
        { { 0, h, }, { 0, 0 } },
        { { w, 0, }, { 1, 1 } },

        { { 0, h, }, { 0, 0 } },
        { { w, h, }, { 1, 0 } },
        { { w, 0, }, { 1, 1 } }
    };

    pg_texture_t* target = pg_create_render_texture(ctx, pixel_w, pixel_h, NULL);
    pg_pipeline_t* pipeline = pg_create_pipeline(ctx, shader, &(pg_pipeline_opts_t)
    {
        .layout =
        {
            .bufs =
            {
                [1] = { .instanced = true }
            },
            .attrs =
            {
                [ATTR_vs_a_pos] = { .format = PG_VFORMAT_FLOAT2,
                                    .offset = offsetof(vertex_t, pos) },

                [ATTR_vs_a_uv] = { .format = PG_VFORMAT_FLOAT2,
                                   .offset = offsetof(vertex_t, uv) },

                [ATTR_vs_a_inst_pos] = { .format = PG_VFORMAT_FLOAT2,
                                         .offset = offsetof(particle_t, pos),
                                         .buffer_index = 1 },

                [ATTR_vs_a_inst_color] = { .format = PG_VFORMAT_FLOAT4,
                                           .offset = offsetof(particle_t, color),
                                           .buffer_index = 1 },
            },
        },
    });

    vs_params_t block =
    {
        .u_mvp =
        {
            2.0f / win_w, 0.0f,         0.0f,       0.0f,
            0.0f,        -2.0/ win_h,   0.0f,       0.0f,
            0.0f,         0.0f,         0.0f,       0.0f,
           -1.0f,         1.0f,         0.0f,       1.0f,
        }
    };

    pg_init_uniform_block(shader, PG_STAGE_VS, "vs_params");
    pg_set_uniform_block(shader, "vs_params", &block);

    pg_buffer_t* vertex_buffer = pg_create_buffer(ctx, PG_USAGE_STATIC,
                                                       vertices, 6, 6,
                                                       sizeof(vertex_t));

    pg_buffer_t* instance_buffer = pg_create_buffer(ctx, PG_USAGE_STREAM,
                                                         NULL, 0, MAX_PARTICLES,
                                                         sizeof(particle_t));

    pg_sampler_t* sampler = pg_create_sampler(ctx, NULL);

    double delta = 0.0;
    ptime_t now, last = pt_now();

    bool done = false;

    while (!done)
    {
        // Calculate delta
        now = pt_now();
        delta = pt_to_sec(now - last);
        last = now;

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

        // New particles
        for (int i = 0; i < NUM_PARTICLES_EMITTED_PER_FRAME; i++)
        {
            if (state.particle_count >= MAX_PARTICLES)
                break;

            float angle = random(PI_F / 4.f, 3.f * PI_F / 4.f);

            particle_t particle =
            {
                .pos = { win_w / 2.f, win_h / 2.f },
                .color = { 1.f, 1.f, 1.f, 1.f },
                //.vel = { cosf(angle) * 100, sinf(angle) * 50 }
                .vel = { 0, 50 }
            };

            state.particles[state.particle_count] = particle;
            state.particle_count++;
        }

        // Update particle positions
        for (int i = 0; i < state.particle_count; i++) {
            //state.particles[i].vel[1] -= 1.0f * delta;
            state.particles[i].pos[0] += state.particles[i].vel[0] * delta;
            state.particles[i].pos[1] += state.particles[i].vel[1] * delta;
            // Bounce back from 'ground'
/*&            if (state.pos[i].Y < -2.0f) {
                state.pos[i].Y = -1.8f;
                state.vel[i].Y = -state.vel[i].Y;
                state.vel[i].X *= 0.8f; state.vel[i].Y *= 0.8f; state.vel[i].Z *= 0.8f;
            }*/
        }

        //printf("particle_count: %d\n", state.particle_count);

        pg_update_buffer(instance_buffer, state.particles, state.particle_count);

        // Bind sampler
        pg_bind_sampler(shader, "u_smp", sampler);
        pg_bind_texture(shader, "u_tex", tex);
        pg_set_pipeline(ctx, pipeline);

        // Save current state
        pg_push_state(ctx);
        pg_begin_pass(ctx, NULL, true);

        pg_draw_buffers(ctx, 6, state.particle_count, (const pg_buffer_t*[]){
            vertex_buffer,
            instance_buffer,
            NULL
        });

        pg_end_pass(ctx);
        pg_pop_state(ctx);

        // Flush draw commands
        pg_flush(ctx);

        SDL_GL_SwapWindow(window);
    }

    pg_destroy_texture(target);
    pg_destroy_texture(tex);
    pg_destroy_sampler(sampler);

    pg_destroy_pipeline(pipeline);
    pg_destroy_shader(shader);
    pg_destroy_context(ctx);

    pg_shutdown();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

