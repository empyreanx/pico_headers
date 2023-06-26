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
     * How to draw 2D sprites using pico_gfx
     * How to implement a basic scene graph
     * Creating and drawing from buffers
     * Using a fixed timestep for animation
     * Using interpolated rendering
*/

#define _POSIX_C_SOURCE 199309L

#include <SDL2/SDL.h>

#define PICO_TIME_IMPLEMENTATION
#include "../pico_time.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_GFX_GL
#define PICO_GFX_IMPLEMENTATION
#include "../pico_gfx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 5
#define FIXED_STEP (1.0 / 50.0)

pg_ctx_t* ctx = NULL;

static struct
{
    SDL_Window* window;
    SDL_GLContext context;
    int screen_w;
    int screen_h;
    pg_vs_block_t vs_block;
} app;

typedef struct
{
    int w, h;
    pg_texture_t* tex;
    pg_vbuffer_t* buf;
} sprite_t;

typedef struct node_s
{
    struct node_s* parent;
    struct node_s* children[MAX_CHILDREN];
    int child_count;
    pm_t2 local;
    pm_t2 world;
    pm_t2 last;
    sprite_t* sprite;
} node_t;

sprite_t* sprite_new(int w, int h, float d, pg_texture_t* tex)
{
    sprite_t* sprite = malloc(sizeof(sprite_t));
    sprite->w = w;
    sprite->h = h;
    sprite->tex = tex;

    pg_vertex_t vertices[6] =
    {
        { { 0, 0, d }, { 1, 1, 1, 1 }, { 0, 1 } },
        { { 0, h, d }, { 1, 1, 1, 1 }, { 0, 0 } },
        { { w, 0, d }, { 1, 1, 1, 1 }, { 1, 1 } },

        { { 0, h, d }, { 1, 1, 1, 1 }, { 0, 0 } },
        { { w, h, d }, { 1, 1, 1, 1 }, { 1, 0 } },
        { { w, 0, d }, { 1, 1, 1, 1 }, { 1, 1 } }
    };

    sprite->buf = pg_create_vbuffer(vertices, 6);

    return sprite;
}

void sprite_free(sprite_t* sprite)
{
    pg_destroy_vbuffer(sprite->buf);
    pg_destroy_texture(sprite->tex);
    free(sprite);
}

node_t* node_new(sprite_t* sprite)
{
    node_t* node = malloc(sizeof(node_t));
    memset(node, 0, sizeof(node_t));
    node->sprite = sprite;
    node->parent = NULL;
    node->local = pm_t2_identity();
    node->world = pm_t2_identity();
    node->last = pm_t2_identity();
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
        node->world = pm_t2_mult(&node->parent->world, &node->local);
    }
    else
    {
        node->world = node->local;
    }
}

pm_t2 node_get_world(node_t* node)
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
    // Render sprite if there is one
    if (node->sprite)
    {
        sprite_t* sprite = node->sprite;

        // Get transforms
        pm_t2 last = node->last;
        pm_t2 world = node_get_world(node);

        // Linearly interpolate between the last and current world transforms
        // by the amount alpha in [0,1]
        pm_t2 render = pm_t2_lerp(&last, &world, alpha);

        // Update model-view

        pg_set_modelview(ctx, (pg_mat4_t)
        {
            render.t00, render.t10, 0.0f, 0.0f,
            render.t01, render.t11, 0.0f, 0.0f,
            0.0f,       0.0f,       0.0f, 0.0f,
            render.tx,  render.ty,  0.0f, 1.0f,
        });

        // Draw vertices
        pg_draw_vbuffer(ctx, sprite->buf, 0, 6, sprite->tex);
    }

    // Render children
    for (int i = 0; i < node->child_count; i++)
    {
        node_render(node->children[i], alpha);
    }
}

pg_texture_t* load_texture(const char* file)
{
    //TODO: clean this up
    int w, h, c;
    unsigned char* bitmap = stbi_load(file, &w, &h, &c, 0);

    assert(bitmap && c == 4);

    size_t size = w * h * c;
    pg_texture_t* tex = pg_create_texture(w, h, bitmap, size, &(pg_texture_opts_t){0});

    return tex;
}

void app_startup()
{
    printf("Scene graph rendering demo\n");

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

    app.window = SDL_CreateWindow("Scene Graph Example",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  1024, 768,
                                  SDL_WINDOW_OPENGL);

    SDL_GL_GetDrawableSize(app.window, &app.screen_w, &app.screen_h);

    SDL_GL_SetSwapInterval(1);
    app.context = SDL_GL_CreateContext(app.window);

    stbi_set_flip_vertically_on_load(true);
}

void app_shutdown()
{
    SDL_GL_DeleteContext(app.context);
    SDL_DestroyWindow(app.window);

    SDL_Quit();
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
} scenegraph_t;

// Build the scene graph
scenegraph_t* sg_build(int scene_w, int scene_h)
{
    scenegraph_t* sg = malloc(sizeof(scenegraph_t));
    sg->w = scene_w;
    sg->h = scene_h;

    int w, h;

    //////////// Root Node ////////////

    // Does not have a sprite or a parent
    sg->root_node = node_new(NULL);

    //////////// BG Node ////////////

    // Load texture
    pg_texture_t* bg_tex = load_texture("./space.png");
    pg_get_texture_size(bg_tex, &w, &h);

    // New sprite
    sg->bg_sprite = sprite_new(scene_w, scene_h, 10.0f, bg_tex);

    // Create a new node that uses this sprite. In theory more than one node
    // could have the same sprite.
    node_t* bg_node = node_new(sg->bg_sprite);

    // And the node as a child of the root node
    node_add_child(sg->root_node, bg_node);

    // The rest of the nodes are similar

    //////////// Star Node ////////////

    pg_texture_t* star_tex = load_texture("./star.png");
    pg_get_texture_size(star_tex, &w, &h);

    sg->star_sprite = sprite_new(w / 3, h / 3, 0.0f, star_tex);
    node_t* star_node = node_new(sg->star_sprite);

    pm_v2 screen_center = pm_v2_make(scene_w / 2, scene_h / 2);

    pm_v2 star_center = pm_v2_make(w / 6, h / 6);
    pm_t2_translate(&star_node->local, pm_v2_scale(star_center, -1.0f));
    pm_t2_translate(&star_node->local, screen_center);

    node_add_child(sg->root_node, star_node);

    //////////// Pivot Node ////////////

    node_t* pivot_node = node_new(NULL);
    sg->pivot_node = pivot_node;

    pm_t2_translate(&pivot_node->local, screen_center);
    node_add_child(sg->root_node, pivot_node);

    //////////// Ship Node ////////////

    pg_texture_t* ship_tex = load_texture("./ship.png");
    pg_get_texture_size(ship_tex, &w, &h);

    int ship_w = w;

    sg->ship_sprite = sprite_new(w, h, 5.0f, ship_tex);
    node_t* ship_node = node_new(sg->ship_sprite);

    pm_t2_translate(&ship_node->local, pm_v2_make(-w / 2, -h / 2));
    pm_t2_translate(&ship_node->local, pm_v2_make(200, 0));

    node_add_child(pivot_node, ship_node);

    //////////// Jet Node ////////////

    pg_texture_t* jet_tex = load_texture("./jet.png");
    pg_get_texture_size(jet_tex, &w, &h);

    sg->jet_sprite = sprite_new(w, h, 0.0f, jet_tex);
    node_t* jet_node = node_new(sg->jet_sprite);

    pm_t2_translate(&jet_node->local, pm_v2_make(0, 32));
    pm_t2_translate(&jet_node->local, pm_v2_make(ship_w / 2 - w / 2, 0));

    node_add_child(ship_node, jet_node);

    return sg;
}

void sg_free(scenegraph_t* sg)
{
    sprite_free(sg->bg_sprite);
    sprite_free(sg->star_sprite);
    sprite_free(sg->ship_sprite);
    sprite_free(sg->jet_sprite);
    node_free(sg->root_node);
    free(sg);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    app_startup();

    pg_init();

    int w = app.screen_w;
    int h = app.screen_h;

    ctx = pg_create_context(w, h);
    pg_shader_t* default_shader = pg_get_default_shader(ctx);

    pg_set_projection(ctx, (pg_mat4_t)
    {
        2.0f / w,  0.0f,   0.0f, 0.0,
        0.0f,      2.0/ h, 0.0f, 0.0,
        0.0f,      0.0f,   0.0f, 0.0f,
       -1.0f,     -1.0f,   0.0f, 1.0f
    });

    pg_pipeline_t* pip = pg_create_pipeline(default_shader, &(pg_pipeline_opts_t)
    {
        .blend_enabled = true,
        .blend =
        {
            .color_src = PG_SRC_ALPHA,
            .color_dst = PG_ONE_MINUS_SRC_ALPHA
        }
    });

    pg_set_pipeline(ctx, pip);

    // Build scene graph
    scenegraph_t* sg = sg_build(app.screen_w, app.screen_h);

    double delta, accumulator = 0.0;
    ptime_t now, last = pt_now();

    // Main Loop
    bool done = false;

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
            pm_v2 scene_center = pm_v2_make(sg->w / 2, sg->h / 2);

            pm_t2_translate(&sg->pivot_node->local, pm_v2_scale(scene_center, -1.0f));
            pm_t2_rotate(&sg->pivot_node->local, -(PM_PI / 8.0f) * FIXED_STEP);
            pm_t2_translate(&sg->pivot_node->local, scene_center);
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

        //pgl_clear(0, 0, 0, 1);

        // Render the scene. Pass in amount left over in the accumulator scaled
        // into [0, 1]
        pg_begin_pass(ctx, NULL, true);
        node_render(sg->root_node, accumulator / FIXED_STEP);
        pg_end_pass(ctx);
        pg_flush(ctx);

        SDL_GL_SwapWindow(app.window);
    }

    sg_free(sg);

    pg_destroy_pipeline(pip);

    pg_destroy_context(ctx);

    pg_shutdown();

    app_shutdown();

    return 0;
}
