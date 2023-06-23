#define _POSIX_C_SOURCE 199309L

#include <SDL2/SDL.h>

#define PICO_TIME_IMPLEMENTATION
#include "../pico_time.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define SOKOL_GLCORE33
#define SOKOL_GFX_IMPL
#define PICO_GFX_IMPLEMENTATION
#include "../pico_gfx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 5
#define FIXED_STEP (1.0 / 50.0)

pg_shader_t* shader = NULL;
pg_ctx_t* ctx = NULL;

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



int main(int argc, char* argv[])
{


    return 0;
}