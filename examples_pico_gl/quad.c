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

#include <SDL.h>

#include <assert.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PICO_GL_IMPLEMENTATION
#include "../pico_gl.h"

bool gles = false;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (argc > 1 && 0 == strcmp(argv[1], "--gles"))
        gles = true;

    printf("Quad rendering demo\n");

    stbi_set_flip_vertically_on_load(true);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    if (gles)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_ES);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    }

    SDL_Window* window = SDL_CreateWindow("Quad Example",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1024, 768,
                                          SDL_WINDOW_OPENGL);

    int screen_w, screen_h;
    SDL_GL_GetDrawableSize(window, &screen_w, &screen_h);

    SDL_GL_SetSwapInterval(1);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (gles)
    {
        // This function must be called AFTER OpenGL context creation, but
        // BEFORE PGL context creation
        pgl_global_init((pgl_loader_fn)SDL_GL_GetProcAddress, true);
    }
    else
    {
        // This function must be called AFTER OpenGL context creation, but
        // BEFORE PGL context creation
        pgl_global_init(NULL, false);
    }

    pgl_print_info();

    // Print maximum texture size

    pgl_ctx_t* ctx = pgl_create_context(screen_w, screen_h, true, 8, false, NULL);
    assert(ctx);

    pgl_set_viewport(ctx, 0, 0, screen_w, screen_h);

    pgl_shader_t* shader = pgl_create_shader(ctx, NULL, NULL);
    assert(shader);

    int image_w, image_h, image_c;
    unsigned char* bitmap = stbi_load("./boomer.png", &image_w, &image_h,
                                                      &image_c, 0);
    assert(bitmap);
    assert(image_c == 4);

    pgl_texture_t* tex = pgl_texture_from_bitmap(ctx,
                                                 PGL_RGBA, false,
                                                 image_w, image_h,
                                                 false, false, bitmap);
    assert(tex);
    free(bitmap);

    pgl_texture_t* target_tex = pgl_create_texture(ctx, true,
                                                   PGL_RGBA, false,
                                                   image_w, image_h,
                                                   false, false);

    assert(target_tex);

    pgl_vertex_t vertices[6] =
    {
        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { {-1.0f, -1.0f }, { 1, 1, 1, 1 }, { 0, 0} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },

        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },
        { { 1.0f,  1.0f }, { 1, 1, 1, 1 }, { 1, 1} }
    };

    pgl_vertex_t indexed_vertices[4] =
    {
        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { {-1.0f, -1.0f }, { 1, 1, 1, 1 }, { 0, 0} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },
        { { 1.0f,  1.0f }, { 1, 1, 1, 1 }, { 1, 1} }
    };

    uint32_t indices[6] = { 0, 1, 2, 0, 2, 3 };

    pgl_buffer_t* buffer = pgl_create_buffer(ctx, PGL_TRIANGLES, vertices, 6);

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

        pgl_set_render_target(ctx, target_tex);

        pgl_clear(1, 1, 1, 1);

        pgl_draw_buffer(ctx, buffer, 0, 6, tex, shader);

        //pgl_draw_array(ctx, PGL_TRIANGLES, vertices, 6, tex, shader);
        pgl_draw_indexed_array(ctx, PGL_TRIANGLES, indexed_vertices, 4, indices, 6, tex, shader);

        pgl_set_render_target(ctx, NULL);

        pgl_clear(1, 1, 1, 1);

        pgl_draw_array(ctx, PGL_TRIANGLES, vertices, 6, target_tex, shader);
        //pgl_draw_indexed(ctx, PGL_TRIANGLES, indexed_vertices, 4, indices, 6, tex, shader);

        SDL_GL_SwapWindow(window);
    }

    pgl_destroy_shader(shader);
    pgl_destroy_buffer(buffer);
    pgl_destroy_texture(target_tex);
    pgl_destroy_texture(tex);
    pgl_destroy_context(ctx);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
