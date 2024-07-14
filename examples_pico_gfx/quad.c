/*
    Quad Example

    Draws an image to the screen.

    Demonstrates:
     * Setting up and SDL window and GL context
     * Initializing pico_gfx
     * Loading an image
     - Creating rendering pipelines
     - Drawing the image (texture) to a render target (another texture)
     - Drawing the render target to the screen
     * Defining vertices, and indices
     - Creating a vertex and index buffers
     * Managing state using the state stack
*/

#include <SDL.h>

#include <assert.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PICO_GFX_GL
#define PICO_GFX_IMPLEMENTATION
#include "../pico_gfx.h"

#define SOKOL_SHDC_IMPL
#include "sprite_shader.h"

typedef struct
{
    float pos[3];
    float color[4];
    float uv[2];
} vertex_t;

typedef float mat4_t[16];

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    assert(pg_backend() == PG_BACKEND_GL);

    printf("Quad rendering demo\n");

    //stbi_set_flip_vertically_on_load(true);

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

    SDL_Window* window = SDL_CreateWindow("Quad Example",
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
    pg_shader_t* shader = pg_create_shader(ctx, sprite);

    // Load image

    int w, h, c;
    unsigned char* bitmap = stbi_load("./boomer.png", &w, &h, &c, 0);

    assert(bitmap);

    // Create texture from bitmap

    size_t size = w * h * c;
    pg_texture_t* tex = pg_create_texture(ctx, w, h, bitmap, size, NULL);

    assert(tex && c == 4);

    free(bitmap);

    // Specify vertices

    vertex_t vertices[6] =
    {
        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { {-1.0f, -1.0f }, { 1, 1, 1, 1 }, { 0, 0} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },

        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },
        { { 1.0f,  1.0f }, { 1, 1, 1, 1 }, { 1, 1} }
    };

    // Specify vertices for indexed drawing

    vertex_t indexed_vertices[4] =
    {
        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { {-1.0f, -1.0f }, { 1, 1, 1, 1 }, { 0, 0} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },
        { { 1.0f,  1.0f }, { 1, 1, 1, 1 }, { 1, 1} }
    };

    // Specify indices

    uint32_t indices[6] = { 0, 1, 2, 0, 2, 3 };

    // Create buffers

    pg_buffer_t* vertex_buffer = pg_create_vertex_buffer(ctx, PG_USAGE_STATIC,
                                                         vertices, 6, 6,
                                                         sizeof(vertex_t));

    pg_buffer_t* indexed_vertex_buffer = pg_create_vertex_buffer(ctx, PG_USAGE_STATIC,
                                                                 indexed_vertices,
                                                                 4, 4, sizeof(vertex_t));

    pg_buffer_t* index_buffer = pg_create_index_buffer(ctx, PG_USAGE_STATIC, indices, 6, 6);

    // Create render target (another texture)
    pg_texture_t* target = pg_create_render_texture(ctx, pixel_w, pixel_h, NULL);

    // Default pipeline
    pg_pipeline_t* pipeline = pg_create_pipeline(ctx, shader, &(pg_pipeline_opts_t)
    {
        .layout =
        {
            .attrs =
            {
                [ATTR_vs_a_pos]   = { .format = PG_VERTEX_FORMAT_FLOAT3,
                                      .offset = offsetof(vertex_t, pos) },

                [ATTR_vs_a_color] = { .format = PG_VERTEX_FORMAT_FLOAT4,
                                     .offset = offsetof(vertex_t, color) },

                [ATTR_vs_a_uv]    = { .format = PG_VERTEX_FORMAT_FLOAT2,
                                      .offset = offsetof(vertex_t, uv) },
            },
        },
    });

    // Pipeline for indexed rendering to a render target
    pg_pipeline_t* target_pipeline = pg_create_pipeline(ctx, shader, &(pg_pipeline_opts_t)
    {
        .layout =
        {
            .attrs =
            {
                [ATTR_vs_a_pos]   = { .format = PG_VERTEX_FORMAT_FLOAT3,
                                      .offset = offsetof(vertex_t, pos) },

                [ATTR_vs_a_color] = { .format = PG_VERTEX_FORMAT_FLOAT4,
                                      .offset = offsetof(vertex_t, color) },

                [ATTR_vs_a_uv]    = { .format = PG_VERTEX_FORMAT_FLOAT2,
                                      .offset = offsetof(vertex_t, uv) },
            },
        },
        .indexed = true,
        .target = true
    });

    // Set uniform model-view-projection matrix to the indentity since we don't
    // need it
    vs_block_t block =
    {
        .u_mvp =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        }
    };

    // Initialize and set uniform block
    pg_alloc_uniform_block(shader, PG_STAGE_VS, "vs_block");
    pg_set_uniform_block(shader, "vs_block", &block);

    // Create default sampler
    pg_sampler_t* sampler = pg_create_sampler(ctx, NULL);

    // Bind the global sampler
    pg_bind_sampler(shader, "u_smp", sampler);

    // Set the global pipeline
    pg_set_pipeline(ctx, pipeline);

    // Bind the global texture
    pg_bind_texture(shader, "u_tex", tex);

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

        // Save current state
        pg_push_state(ctx);

        // Note: Drawing to a render target is not neccessary and is only
        // present here to test if it works
        pg_begin_pass(ctx, target, true);

        // Set pipe line configured to draw to render targets
        pg_set_pipeline(ctx, target_pipeline);

        // Bind the vertex buffer
        pg_bind_buffer(ctx, 0, indexed_vertex_buffer);

        // Activate the index buffer
        pg_set_index_buffer(ctx, index_buffer);

        // Issue the commnd to draw the vertex buffer according to the index
        pg_draw(ctx, 0, 6, 1);

        // End first pass
        pg_end_pass(ctx);

        // Restore previous state
        pg_pop_state(ctx);

        // Second pass: draw render target to the screen
        // Save the curent state
        pg_push_state(ctx);

        // Start a new pass (drawing to the screen)
        pg_begin_pass(ctx, NULL, true);

        // Bind a vertex buffer
        pg_bind_buffer(ctx, 0, vertex_buffer);

        // Issue the draw command (no indexing)
        pg_draw(ctx, 0, 6, 1);

        // End the second pass
        pg_end_pass(ctx);

        // Restore original state
        pg_pop_state(ctx);

        // Flush draw commands
        pg_flush(ctx);

        // Finally swap buffers
        SDL_GL_SwapWindow(window);
    }

    pg_destroy_texture(target);
    pg_destroy_texture(tex);
    pg_destroy_sampler(sampler);

    pg_destroy_buffer(vertex_buffer);
    pg_destroy_buffer(indexed_vertex_buffer);
    pg_destroy_buffer(index_buffer);

    pg_destroy_pipeline(pipeline);
    pg_destroy_pipeline(target_pipeline);
    pg_destroy_shader(shader);
    pg_destroy_context(ctx);

    pg_shutdown();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

