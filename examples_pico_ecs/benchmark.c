/*=============================================================================
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 *============================================================================*/

//#define ECS_DEBUG
#define PICO_ECS_MAX_SYSTEMS 16
#define PICO_ECS_MAX_COMPONENTS 64
#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*=============================================================================
 * Preamble
 *============================================================================*/

int random_int(int min, int max)
{
    return rand() % (max + 1 - min) + min;
}

#define MIN_ENTITIES (1000 * 1000)
#define MAX_ENTITIES (1000 * 1000)

static clock_t start, end;
static ecs_t* ecs = NULL;

static void setup();
static void teardown();

static void bench_begin(const char* name)
{
    printf("---------------------------------------------------------------\n");
    printf("Running: %s\n", name);
    start = clock();
}

static void bench_end()
{
    end = clock();
    double elapsed = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time elapsed %f ms\n", elapsed);
}

#define BENCH_RUN(fp, setup_fp, teardown_fp) \
    setup_fp(); \
    bench_begin(#fp); \
    fp(); \
    bench_end(); \
    teardown_fp();

/*=============================================================================
 * Systems/components
 *============================================================================*/

// System IDs
ecs_id_t MovementSystem;
ecs_id_t ComflabSystem;
ecs_id_t BoundsSystem;
ecs_id_t QueueDestroySystem;

// Component IDs
ecs_id_t PosComponent;
ecs_id_t DirComponent;
ecs_id_t RectComponent;
ecs_id_t ComflabComponent;

// Position component
typedef struct
{
    float x, y;
} v2d_t;

// Rectangle component
typedef struct
{
    int x, y, w, h;
} rect_t;

typedef struct
{
    float thingy;
    bool mingy;
    int dingy;
} comflab_t;

/*=============================================================================
 * Setup / teardown functions
 *============================================================================*/

ecs_ret_t movement_system(ecs_t* ecs, ecs_id_t* entities,
                          int entity_count, ecs_dt_t dt,
                          void* udata);

ecs_ret_t comflab_system(ecs_t* ecs, ecs_id_t* entities,
                         int entity_count, ecs_dt_t dt,
                         void* udata);

ecs_ret_t bounds_system(ecs_t* ecs,
                        ecs_id_t* entities,
                        int entity_count,
                        ecs_dt_t dt,
                        void* udata);

static void setup()
{
    // Create ECS instance
    ecs = ecs_new(MIN_ENTITIES, NULL);

    // Register two new components
    PosComponent  = ecs_register_component(ecs, sizeof(v2d_t),  NULL, NULL);
    RectComponent = ecs_register_component(ecs, sizeof(rect_t), NULL, NULL);
}

static void setup_destroy_with_two_components()
{
    // Create ECS instance
    ecs = ecs_new(MIN_ENTITIES, NULL);

    PosComponent  = ecs_register_component(ecs, sizeof(v2d_t),  NULL, NULL);
    RectComponent = ecs_register_component(ecs, sizeof(rect_t), NULL, NULL);

    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, PosComponent, NULL);
        ecs_add(ecs, id, RectComponent, NULL);
    }
}

static void setup_three_systems()
{
    ecs = ecs_new(MIN_ENTITIES, NULL);

    PosComponent = ecs_register_component(ecs, sizeof(v2d_t), NULL, NULL);
    DirComponent = ecs_register_component(ecs, sizeof(v2d_t), NULL, NULL);
    ComflabComponent = ecs_register_component(ecs, sizeof(comflab_t), NULL, NULL);
    RectComponent = ecs_register_component(ecs, sizeof(rect_t), NULL, NULL);

    MovementSystem = ecs_register_system(ecs, movement_system, NULL, NULL, NULL);
    ecs_require_component(ecs, MovementSystem, PosComponent);
    ecs_require_component(ecs, MovementSystem, DirComponent);

    ComflabSystem = ecs_register_system(ecs, comflab_system, NULL, NULL, NULL);
    ecs_require_component(ecs, ComflabSystem, ComflabComponent);

    BoundsSystem = ecs_register_system(ecs, bounds_system, NULL, NULL, NULL);
    ecs_require_component(ecs, BoundsSystem, RectComponent);
}

// Runs after benchmark function
static void teardown()
{
    // Destroy ECS instance
    ecs_free(ecs);
    ecs = NULL;
}

static void setup_get()
{
    // Create ECS instance
    ecs = ecs_new(MIN_ENTITIES, NULL);
    PosComponent = ecs_register_component(ecs, sizeof(v2d_t), NULL, NULL);

    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        ecs_add(ecs, id, PosComponent, NULL);
    }
}

/*=============================================================================
 * Update function callbacks
 *============================================================================*/

ecs_ret_t movement_system(ecs_t* ecs,
                          ecs_id_t* entities,
                          int entity_count,
                          ecs_dt_t dt,
                          void* udata)
{
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        // Get entity ID
        ecs_id_t id = entities[i];

        v2d_t* pos = ecs_get(ecs, id, PosComponent);
        v2d_t* dir = ecs_get(ecs, id, DirComponent);

        pos->x += pos->x + dir->x * dt;
        pos->y += pos->y + dir->y * dt;
    }

    return 0;
}

ecs_ret_t comflab_system(ecs_t* ecs,
                        ecs_id_t* entities,
                        int entity_count,
                        ecs_dt_t dt,
                        void* udata)
{
    (void)dt;
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        // Get entity ID
        ecs_id_t id = entities[i];

        comflab_t* comflab = ecs_get(ecs, id, ComflabComponent);
        comflab->thingy *= 1.000001f;
    	comflab->mingy = !comflab->mingy;
	    comflab->dingy++;
    }

    return 0;
}

ecs_ret_t bounds_system(ecs_t* ecs,
                        ecs_id_t* entities,
                        int entity_count,
                        ecs_dt_t dt,
                        void* udata)
{
    (void)dt;
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        // Get entity ID
        ecs_id_t id = entities[i];

        rect_t* bounds = ecs_get(ecs, id, RectComponent);

        bounds->x = 1;
        bounds->y = 1;
        bounds->w = 1;
        bounds->h = 1;
    }

    return 0;
}

ecs_ret_t queue_destroy_system(ecs_t* ecs,
                               ecs_id_t* entities,
                               int entity_count,
                               ecs_dt_t dt,
                               void* udata)
{
    (void)dt;
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_queue_destroy(ecs, entities[i]);
    }

    return 0;
}

/*=============================================================================
 * Benchmark functions
 *============================================================================*/

// Creates entity IDs as fast as possible
static void bench_create()
{
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
        ecs_create(ecs);
}

// Creates entity IDs as fast as possible and immediately destroys the
// coresponding entity
static void bench_create_destroy()
{
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
        ecs_destroy(ecs, ecs_create(ecs));
}

static void bench_destroy_with_two_components()
{
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_destroy(ecs, i);
    }
}

static void bench_create_with_two_components()
{
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        ecs_add(ecs, id, PosComponent, NULL);
        ecs_add(ecs, id, RectComponent, NULL);
    }
}

// Adds components to entities and assigns values to them
static void bench_add_remove()
{
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, PosComponent, NULL);
        ecs_remove(ecs, id, PosComponent);
    }
}

// Adds components to entities and assigns values to them
static void bench_add_assign()
{
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        v2d_t*  pos  = (v2d_t*)ecs_add(ecs, id, PosComponent, NULL);
        rect_t* rect = (rect_t*)ecs_add(ecs, id, RectComponent, NULL);

        // Set concrete component values
        *pos  = (v2d_t) { 1, 2 };
        *rect = (rect_t){ 1, 2, 3, 4 };
    }
}

// Adds components to entities, retrieves the components, and assigns
// values to them
static void bench_get()
{
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_get(ecs, i, PosComponent);
    }
}

static void bench_queue_destroy()
{
    QueueDestroySystem = ecs_register_system(ecs, queue_destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, QueueDestroySystem, PosComponent);
    ecs_require_component(ecs, QueueDestroySystem, RectComponent);

    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_create(ecs);
    }

    ecs_update_system(ecs, QueueDestroySystem, 1.0f);
}

static void bench_three_systems()
{
    // Create entities
    for (ecs_id_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        v2d_t*  pos    = ecs_add(ecs, id, PosComponent, NULL);
        v2d_t*  dir    = ecs_add(ecs, id, DirComponent, NULL);
        rect_t* bounds = ecs_add(ecs, id, RectComponent, NULL);

        if (i % 2 == 0)
        {
            comflab_t* comflab = ecs_add(ecs, id, ComflabComponent, NULL);
            *comflab = (comflab_t){ 0 };
        }

        // Set concrete component values
        *pos    = (v2d_t)  { 0 };
        *dir    = (v2d_t)  { 0 };
        *bounds = (rect_t) { 0 };
    }

    // Run the system
    ecs_update_system(ecs, MovementSystem, 1.0f);
    ecs_update_system(ecs, ComflabSystem, 1.0f);
    ecs_update_system(ecs, BoundsSystem, 1.0f);
}

int main()
{
    printf("===============================================================\n");

    printf("Number of entities: %u\n", MAX_ENTITIES);

    BENCH_RUN(bench_create, setup, teardown);
    BENCH_RUN(bench_create_destroy, setup, teardown);
    BENCH_RUN(bench_create_with_two_components, setup, teardown);
    BENCH_RUN(bench_destroy_with_two_components, setup_destroy_with_two_components, teardown);
    BENCH_RUN(bench_add_remove, setup, teardown);
    BENCH_RUN(bench_add_assign, setup, teardown);
    BENCH_RUN(bench_get, setup_get, teardown);
    BENCH_RUN(bench_queue_destroy, setup, teardown);
    BENCH_RUN(bench_three_systems, setup_three_systems, teardown);

    printf("---------------------------------------------------------------\n");

    return 0;
}

