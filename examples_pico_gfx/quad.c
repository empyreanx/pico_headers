/*
    Quad Example

    Draws an image to the screen.

    Demonstrates:
     * Setting up and SDL window and GL context
     * Initializing pico_gl
     * Loading an image
     * Creating a texture from the image
     * Defining vertices
     * Drawing the vertices

    Todo:
     * Toggle MSAA in command line arguments
     * Toggle GLES in command line arguments
     * Froper error handling
*/

#include <SDL2/SDL.h>

#include <assert.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PICO_GFX_GL
#include "../pico_gfx.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

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
    pg_ctx_t* ctx = pg_create_context(pixel_w, pixel_h);

    // Register/set uniform
    pg_shader_t* default_shader = pg_get_default_shader(ctx);

    /*pg_register_uniform_block(default_shader, PG_VS_STAGE, "pg_vs_block", sizeof(pg_vs_block_t));

    pg_vs_block_t vs_block = (pg_vs_block_t)
    {
        { 1.0f,  0.0f, 0.0f, 0.0f,
          0.0f,  1.0f, 0.0f, 0.0f,
          0.0f,  0.0f, 0.0f, 0.0f,
          0.0f,  0.0f, 0.0f, 1.0f },

        { 1.0f,  0.0f, 0.0f, 0.0f,
          0.0f,  1.0f, 0.0f, 0.0f,
          0.0f,  0.0f, 0.0f, 0.0f,
          0.0f,  0.0f, 0.0f, 1.0f }
    };

    pg_set_uniform_block(default_shader, "pg_vs_block", &vs_block);*/

    // Load image

    int w, h, c;
    unsigned char* bitmap = stbi_load("./boomer.png", &w, &h, &c, 0);

    assert(bitmap);

    // Load texture

    size_t size = w * h * c;
    pg_texture_t* tex = pg_create_texture(w, h, bitmap, size, 0, false, false);

    assert(tex && c == 4);

    free(bitmap);

    // Specify vertices

    pg_vertex_t vertices[6] =
    {
        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { {-1.0f, -1.0f }, { 1, 1, 1, 1 }, { 0, 0} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },

        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },
        { { 1.0f,  1.0f }, { 1, 1, 1, 1 }, { 1, 1} }
    };

    pg_vertex_t indexed_vertices[4] =
    {
        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { {-1.0f, -1.0f }, { 1, 1, 1, 1 }, { 0, 0} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },
        { { 1.0f,  1.0f }, { 1, 1, 1, 1 }, { 1, 1} }
    };

    uint32_t indices[6] = { 0, 1, 2, 0, 2, 3 };

    pg_texture_t* target = pg_create_render_texture(pixel_w, pixel_h, 0, false, false);
    pg_pass_t* pass = pg_create_pass(target);

    pg_pipeline_t* pip = pg_create_pipeline(default_shader, &(pg_pipeline_desc_t)
    {
        .indexed = true,
        .target = true
    });

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

        pg_push_state(ctx);

        pg_begin_pass(ctx, pass, true);
        pg_set_pipeline(ctx, pip);
        pg_draw_indexed_array(ctx, indexed_vertices, 4, indices, 6, tex);
        pg_end_pass(ctx);

        pg_pop_state(ctx);

        pg_begin_pass(ctx, NULL, true);
        pg_draw_array(ctx, vertices, 6, target);
        pg_end_pass(ctx);

        pg_flush(ctx);

        SDL_GL_SwapWindow(window);
    }

    pg_destroy_texture(target);
    pg_destroy_texture(tex);

    pg_destroy_pipeline(pip);
    pg_destroy_pass(pass);
    pg_destroy_context(ctx);

    pg_shutdown();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

#define PICO_GFX_IMPLEMENTATION
#include "../pico_gfx.h"


