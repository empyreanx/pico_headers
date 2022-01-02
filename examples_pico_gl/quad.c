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

#include <assert.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PGL_IMPLEMENTATION
#include "../pico_gl.h"

bool gles = false;
void* handle = NULL;

void gles_open()
{
    handle = SDL_LoadObject("libGL.so");
}

void gles_close()
{
    SDL_UnloadObject(handle);
}

void* gles_proc(const char* name)
{
    return SDL_LoadFunction(handle, name);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (argc > 1 && 0 == strcmp(argv[1], "-gles"))
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

    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

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
        gles_open();
        // This function must be called AFTER OpenGL context creation, but
        // BEFORE PGL context creation
        pgl_global_init((pgl_loader_fn)gles_proc, true);
    }
    else
    {
        // This function must be called AFTER OpenGL context creation, but
        // BEFORE PGL context creation
        pgl_global_init(NULL, false);
    }

    pgl_print_info();

    pgl_ctx_t* ctx = pgl_create_context(screen_w, screen_h, 0, false, NULL);
    assert(ctx);

    pgl_set_viewport(ctx, 0, 0, screen_w, screen_h);

    pgl_shader_t* shader = pgl_create_shader(ctx, NULL, NULL);
    assert(shader);

    int image_w, image_h, image_c;
    unsigned char* bitmap = stbi_load("./boomer.png", &image_w, &image_h,
                                                      &image_c, 0);
    assert(bitmap);

    pgl_texture_t* tex = pgl_texture_from_bitmap(ctx, PGL_RGBA,
                                                 image_w, image_h,
                                                 false, false, bitmap);
    assert(tex);
    free(bitmap);

    pgl_vertex_t vertices[6] = {
        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { {-1.0f, -1.0f }, { 1, 1, 1, 1 }, { 0, 0} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },

        { {-1.0f,  1.0f }, { 1, 1, 1, 1 }, { 0, 1} },
        { { 1.0f, -1.0f }, { 1, 1, 1, 1 }, { 1, 0} },
        { { 1.0f,  1.0f }, { 1, 1, 1, 1 }, { 1, 1} }
    };

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

        pgl_clear(1, 1, 1, 1);

        pgl_draw_array(ctx, PGL_TRIANGLES, vertices, 6, tex, shader);

        SDL_GL_SwapWindow(window);
    }

    pgl_destroy_shader(shader);
    pgl_destroy_texture(tex);
    pgl_destroy_context(ctx);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();

    if (gles)
        gles_close();

    return 0;
}
