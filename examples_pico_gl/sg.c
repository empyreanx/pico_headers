/*
    Scene Graph Example

    A scene graph is a directed tree where each node in the tree is associated
    with an affine tranformation (translation/rotation/scaling). These
    transforms are concatentated from the root to descendent nodes allowing
    the graphs to represent highly articulated models or complex world geometry.
    Every sub-tree of a scene graph is also a scene graph. This means that
    complex scene graphs can be built incrementally from simpler ones.

    The scene graph implemented in this example is basic, however it should be
    enough to impart the fundamental concepts. The pico_ml library is also
    used thoughout and thus should also provide an introduction to it as well.

    This example demonstrates a number of concepts
     * How to draw 2D sprites using pico_gl
     * How to implement a basic scene graph
     * Creating and drawing from buffers
     * Using a fixed timestep for animation
     * Using interpolated rendering

    Todo:
     * Toggle MSAA in command line arguments
     * Toggle GLES in command line arguments
     * Froper error handling
     * Toggle full screen, interpolation
     * More docs
*/

#define _POSIX_C_SOURCE 199309L

#include <SDL.h>

#define PICO_TIME_IMPLEMENTATION
#include "../pico_time.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_GL_IMPLEMENTATION
#include "../pico_gl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 5
#define FIXED_STEP (1.0 / 50.0)

pgl_shader_t* shader = NULL;
pgl_ctx_t* ctx = NULL;

typedef struct
{
    int w, h;
    pgl_texture_t* tex;
    pgl_buffer_t* buf;
} sprite_t;

// Scene graph node
typedef struct node_s
{
    struct node_s* parent;
    struct node_s* children[MAX_CHILDREN];
    int child_count;
    pt2 local;
    pt2 world;
    pt2 last;
    sprite_t* sprite;
} node_t;

sprite_t* sprite_new(int w, int h, pgl_texture_t* tex)
{
    sprite_t* sprite = malloc(sizeof(sprite_t));
    sprite->w = w;
    sprite->h = h;
    sprite->tex = tex;

    pgl_vertex_t vertices[6] =
    {
        { { 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 1 } },
        { { 0, h, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },
        { { w, 0, 0 }, { 1, 1, 1, 1 }, { 1, 1 } },

        { { 0, h, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },
        { { w, h, 0 }, { 1, 1, 1, 1 }, { 1, 0 } },
        { { w, 0, 0 }, { 1, 1, 1, 1 }, { 1, 1 } }
    };

    sprite->buf = pgl_create_buffer(ctx, PGL_TRIANGLES, vertices, 6);

    return sprite;
}

void sprite_free(sprite_t* sprite)
{
    pgl_destroy_buffer(sprite->buf);
    pgl_destroy_texture(sprite->tex);
    free(sprite);
}

node_t* node_new(sprite_t* sprite)
{
    node_t* node = malloc(sizeof(node_t));
    memset(node, 0, sizeof(node_t));
    node->sprite = sprite;
    node->parent = NULL;
    node->local = pt2_identity();
    node->world = pt2_identity();
    node->last = pt2_identity();
    return node;
}

void node_free(node_t* node)
{
    for (int i = 0; i < node->child_count; i++)
    {
        node_free(node->children[i]);
    }

    free(node);
}

void node_add_child(node_t* parent, node_t* node)
{
    assert(parent->child_count < MAX_CHILDREN);
    node->parent = parent;
    parent->children[parent->child_count] = node;
    parent->child_count++;
}

// Recursive updates world transforms from local ones. This function is at the
// core of a scene graph.
void node_update_transform(node_t* node)
{
    if (node->parent)
    {
        node_update_transform(node->parent);
        node->world = pt2_mult(&node->parent->world, &node->local);
    }
    else
    {
        node->world = node->local;
    }
}

pt2 node_get_world(node_t* node)
{
    node_update_transform(node);
    return node->world;
}

void node_update_last(node_t* node)
{
    node->last = node_get_world(node);

    for (int i = 0; i < node->child_count; i++)
    {
        node_update_last(node->children[i]);
    }
}

void node_render(node_t* node, double alpha)
{
    // Render children
    for (int i = 0; i < node->child_count; i++)
    {
        node_render(node->children[i], alpha);
    }

    // Render sprite if there is one
    if (node->sprite)
    {
        sprite_t* sprite = node->sprite;

        // Get transforms
        pt2 last = node->last;
        pt2 world = node_get_world(node);

        // Linearly interpolate between the last and current world transforms
        // by the amount alpha in [0,1]
        pt2 render = pt2_lerp(&last, &world, alpha);

        // Update model-view

        // The following transforms are equivalent

        /*pgl_set_transform(ctx, (pgl_m4_t)
        {
            render.t00, render.t01, 0.0f, render.tx,
            render.t10, render.t11, 0.0f, render.ty,
            0.0f,       0.0f,       1.0f, 0.0f,
            0.0f,       0.0f,       0.0f, 1.0f
        });*/

        pgl_set_transform_3d(ctx, (pgl_m3_t)
        {
            render.t00, render.t01, render.tx,
            render.t10, render.t11, render.ty,
            0.0f,       0.0f,       1.0f
        });

        // Draw vertices
        pgl_draw_buffer(ctx, sprite->buf, 0, 6, sprite->tex, shader);
    }
}

pgl_texture_t* load_texture(const char* file, int* w, int* h)
{

    int c;
    unsigned char* bitmap = stbi_load(file, w, h, &c, 0);
    assert(bitmap);

    pgl_texture_t* tex = pgl_texture_from_bitmap(ctx,
                                                 PGL_RGBA, false,
                                                 *w, *h,
                                                 false, false, bitmap);

    assert(tex);
    free(bitmap);

    return tex;
}

typedef struct
{
    SDL_Window* window;
    SDL_GLContext context;
    int screen_w;
    int screen_h;
} app_t;

app_t* app_startup()
{
    printf("Scene graph rendering demo\n");

    app_t* app = malloc(sizeof(app_t));
    memset(app, 0, sizeof(app_t));

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

    app->window = SDL_CreateWindow("Scene Graph Example",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    1024, 768,
                                    SDL_WINDOW_OPENGL);

    SDL_GL_GetDrawableSize(app->window, &app->screen_w, &app->screen_h);

    SDL_GL_SetSwapInterval(1);
    app->context = SDL_GL_CreateContext(app->window);

    stbi_set_flip_vertically_on_load(true);

    return app;
}

void app_shutdown(app_t* app)
{
    SDL_GL_DeleteContext(app->context);
    SDL_DestroyWindow(app->window);

    SDL_Quit();

    free(app);
}

typedef struct
{
    int w;
    int h;
    node_t* root_node;
    node_t* pivot_node;
    sprite_t* bg_sprite;
    sprite_t* star_sprite;
    sprite_t* ship_sprite;
    sprite_t* jet_sprite;
} sg_t;

// Build the scene graph
sg_t* sg_build(int scene_w, int scene_h)
{
    sg_t* sg = malloc(sizeof(sg_t));
    sg->w = scene_w;
    sg->h = scene_h;

    int w, h;

    //////////// Root Node ////////////

    // Does not have a sprite or a parent
    sg->root_node = node_new(NULL);

    //////////// BG Node ////////////

    // Load texture
    pgl_texture_t* bg_tex = load_texture("./space.png", &w, &h);

    // New sprite
    sg->bg_sprite = sprite_new(scene_w, scene_h, bg_tex);

    // Create a new node that uses this sprite. In theory more than one node
    // could have the same sprite.
    node_t* bg_node = node_new(sg->bg_sprite);

    // And the node as a child of the root node
    node_add_child(sg->root_node, bg_node);

    // The rest of the nodes are similar

    //////////// Star Node ////////////

    pgl_texture_t* star_tex = load_texture("./star.png", &w, &h);

    sg->star_sprite = sprite_new(w / 3, h / 3, star_tex);
    node_t* star_node = node_new(sg->star_sprite);

    pv2 screen_center = pv2_make(scene_w / 2, scene_h / 2);

    pv2 star_center = pv2_make(w / 6, h / 6);
    pt2_translate(&star_node->local, pv2_scale(star_center, -1.0f));
    pt2_translate(&star_node->local, screen_center);

    node_add_child(sg->root_node, star_node);

    //////////// Pivot Node ////////////

    node_t* pivot_node = node_new(NULL);
    sg->pivot_node = pivot_node;

    pt2_translate(&pivot_node->local, screen_center);
    node_add_child(sg->root_node, pivot_node);

    //////////// Ship Node ////////////

    pgl_texture_t* ship_tex = load_texture("./ship.png", &w, &h);

    int ship_w = w;

    sg->ship_sprite = sprite_new(w, h, ship_tex);
    node_t* ship_node = node_new(sg->ship_sprite);

    pt2_translate(&ship_node->local, pv2_make(-w / 2, -h / 2));
    pt2_translate(&ship_node->local, pv2_make(200, 0));

    node_add_child(pivot_node, ship_node);

    //////////// Jet Node ////////////

    pgl_texture_t* jet_tex = load_texture("./jet.png", &w, &h);

    sg->jet_sprite = sprite_new(w, h, jet_tex);
    node_t* jet_node = node_new(sg->jet_sprite);

    pt2_translate(&jet_node->local, pv2_make(0, 32));
    pt2_translate(&jet_node->local, pv2_make(ship_w / 2 - w / 2, 0));

    node_add_child(ship_node, jet_node);

    return sg;
}

void sg_free(sg_t* sg)
{
    sprite_free(sg->bg_sprite);
    sprite_free(sg->star_sprite);
    sprite_free(sg->ship_sprite);
    sprite_free(sg->jet_sprite);
    node_free(sg->root_node);
    free(sg);
}

// Hires time
double time_now()
{
    return (double)SDL_GetPerformanceCounter() /
           (double)SDL_GetPerformanceFrequency();
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // Application initialization
    app_t* app = app_startup();

    // This function must be called AFTER OpenGL context creation, but BEFORE
    // PGL context creation
    pgl_global_init(NULL, false);
    pgl_print_info();

    // Create global PGL context
    ctx = pgl_create_context(app->screen_w, app->screen_h, false, 0, false, NULL);
    pgl_set_viewport(ctx, 0, 0, app->screen_w, app->screen_h);

    // Make sure matrices are row-major order
    pgl_set_transpose(ctx, true);

    // Construct default shader
    shader = pgl_create_shader(ctx, NULL, NULL);
    assert(shader);

    // Set projection matrix
    int w = app->screen_w;
    int h = app->screen_h;

    pgl_set_projection(ctx, (pgl_m4_t)
    {
        2.0f / w, 0.0f,     0.0f, -1.0f,
        0.0f,    -2.0f / h, 0.0f,  1.0f,
        0.0f,     0.0f,     0.0f,  0.0f,
        0.0f,     0.0f,     0.0f,  1.0f
    });

    // Build scene graph
    sg_t* sg = sg_build(app->screen_w, app->screen_h);

    // Main Loop
    bool done = false;

    double delta, accumulator = 0.0;
    ptime_t now, last = pt_now();

    while (!done)
    {
        // Calculate delta
        now = pt_now();
        delta = pt_to_sec(now - last);
        last = now;

        // Update last world transforms for interpolation
        node_update_last(sg->root_node);

        // Fixed timestep.
        // The purpose of a fixed timestep is to ensure that some code runs
        // at a fixed (average) rate. This important for things like animation
        // which need to be updated at a consistent rate in order to appear
        // smooth. In the code below, the pivot node gets updated
        // approximately 1.0 / FIXED_STEP times per second.
        accumulator += delta;

        while (accumulator >= FIXED_STEP)
        {
            accumulator -= FIXED_STEP;

            // Update last world transforms for interpolation
            node_update_last(sg->root_node);

            // Rotate the pivot
            pv2 scene_center = pv2_make(sg->w / 2, sg->h / 2);

            pt2_translate(&sg->pivot_node->local, pv2_scale(scene_center, -1.0f));
            pt2_rotate(&sg->pivot_node->local, -(PM_PI / 8.0f) * FIXED_STEP);
            pt2_translate(&sg->pivot_node->local, scene_center);
        }

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

        pgl_clear(0, 0, 0, 1);

        // Render the scene. Pass in amount left over in the accumulator scaled
        // into [0, 1]
        node_render(sg->root_node, accumulator / FIXED_STEP);

        SDL_GL_SwapWindow(app->window);
    }

    // Release resources
    sg_free(sg);

    pgl_destroy_shader(shader);
    pgl_destroy_context(ctx);

    app_shutdown(app);

    return 0;
}
