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

// Vertex attributes
typedef struct
{
    float pos[2];
    float uv[2];
} vertex_t;

// Particle attributes
typedef struct
{
    float pos[2];
    float color[4];
} particle_t;

// Particle velocity
typedef struct
{
    float x, y;
} vel_t;

// Simulation staTe
static struct
{
    particle_t particles[MAX_PARTICLES];
    int particle_count;
    vel_t vel[MAX_PARTICLES];
} state;

// Generate a random number between min and max
static float random_float(float min, float max)
{
    return ((float)rand() / RAND_MAX) * (max - min) + min;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    memset(&state, 0, sizeof(state));

    assert(pg_backend() == PG_BACKEND_GL);

    printf("Particle rendering demo\n");

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

    // Load particle image

    int w, h, c;
    unsigned char* bitmap = stbi_load("./circle.png", &w, &h, &c, 0);

    assert(bitmap);
    assert(c == 4);

    // Create particle texture

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

    pg_pipeline_t* pipeline = pg_create_pipeline(ctx, shader, &(pg_pipeline_opts_t)
    {
        .layout =
        {
            .bufs =
            {
                [1] = { .instanced = true } // Buffer in slot 1 is instanced
            },
            .attrs =
            {
                // Attributes for the vertex buffer
                [ATTR_vs_a_pos] = { .format = PG_VERTEX_FORMAT_FLOAT2,
                                    .offset = offsetof(vertex_t, pos) },

                [ATTR_vs_a_uv]  = { .format = PG_VERTEX_FORMAT_FLOAT2,
                                    .offset = offsetof(vertex_t, uv) },

                // Attributes for the instanced buffer
                [ATTR_vs_a_inst_pos]  = { .format = PG_VERTEX_FORMAT_FLOAT2,
                                          .offset = offsetof(particle_t, pos),
                                          .buffer_index = 1 },

                [ATTR_vs_a_inst_color] = { .format = PG_VERTEX_FORMAT_FLOAT4,
                                           .offset = offsetof(particle_t, color),
                                           .buffer_index = 1 },
            },
        },
        .blend_enabled = true,
        .blend =
        {
            .color_src = PG_SRC_ALPHA,
            .color_dst = PG_ONE_MINUS_SRC_ALPHA
        }
    });

    // Sets the vertex uniform block mvp to project world to NDC coordinates
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

    // Initializes and sets the uniform
    pg_alloc_uniform_block(shader, PG_STAGE_VS, "vs_params");
    pg_set_uniform_block(shader, "vs_params", &block);

    // Create the vertex buffer
    pg_buffer_t* vertex_buffer = pg_create_vertex_buffer(ctx, PG_USAGE_STATIC,
                                                         vertices, 6, 6,
                                                         sizeof(vertex_t));

    // Create the instance buffer
    pg_buffer_t* instance_buffer = pg_create_vertex_buffer(ctx, PG_USAGE_STREAM,
                                                           NULL, 0, MAX_PARTICLES,
                                                           sizeof(particle_t));

    // Create a default sampler
    pg_sampler_t* sampler = pg_create_sampler(ctx, NULL);


    // The main loop
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

            // Calculate a random angle for the particles trajectory
            float angle = random_float(PI_F / 4.f, 3.f * PI_F / 4.f);

            // Set new particle to the center of the screen
            particle_t particle =
            {
                .pos   = { win_w / 2.f, win_h / 2.f },
                .color = { 0.f, 0.f, 0.f, 1.f },
            };

            // Vary the color of the particles
            particle.color[i % 3] = 1.f;

            // Update global state
            state.particles[state.particle_count] = particle;
            state.vel[state.particle_count] = (vel_t){ cosf(angle) * 10, -sinf(angle) * 50 };
            state.particle_count++;
        }

        // Update particle positions
        for (int i = 0; i < state.particle_count; i++)
        {
            state.particles[i].pos[0] += state.vel[i].x * delta;
            state.particles[i].pos[1] += state.vel[i].y * delta;
        }

        pg_update_buffer(instance_buffer, state.particles, state.particle_count);

        // Bind sampler
        pg_bind_sampler(shader, "u_smp", sampler);
        pg_bind_texture(shader, "u_tex", tex);
        pg_set_pipeline(ctx, pipeline);

        // Save current state
        pg_push_state(ctx);
        pg_begin_pass(ctx, NULL, true);

        // Bind buffers and issue a draw command
        pg_bind_buffer(ctx, 0, vertex_buffer);
        pg_bind_buffer(ctx, 1, instance_buffer);

        // Draw using instancing
        pg_draw(ctx, 0, 6, state.particle_count);

        // Clean up
        pg_end_pass(ctx);
        pg_pop_state(ctx);

        // Flush draw commands
        pg_flush(ctx);

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    pg_destroy_texture(tex);
    pg_destroy_sampler(sampler);
    pg_destroy_pipeline(pipeline);
    pg_destroy_shader(shader);
    pg_destroy_buffer(vertex_buffer);
    pg_destroy_buffer(instance_buffer);
    pg_destroy_context(ctx);

    pg_shutdown();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

