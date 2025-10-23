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

#define MIN_ENTITIES (1 * 1000)
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

// System types
ecs_system_t MovementSystem;
ecs_system_t ComflabSystem;
ecs_system_t BoundsSystem;
ecs_system_t QueueDestroySystem;

// Component types
ecs_comp_t PosComponent;
ecs_comp_t DirComponent;
ecs_comp_t RectComponent;
ecs_comp_t ComflabComponent;

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

ecs_ret_t movement_system(ecs_t* ecs,
                          ecs_entity_t* entities,
                          size_t entity_count,
                          void* udata);

ecs_ret_t comflab_system(ecs_t* ecs,
                         ecs_entity_t* entities,
                         size_t entity_count,
                         void* udata);

ecs_ret_t bounds_system(ecs_t* ecs,
                        ecs_entity_t* entities,
                        size_t entity_count,
                        void* udata);

static void setup()
{
    // Create ECS instance
    ecs = ecs_new(MIN_ENTITIES, NULL);

    // Register two new components
    PosComponent  = ecs_define_component(ecs, sizeof(v2d_t),  NULL, NULL);
    RectComponent = ecs_define_component(ecs, sizeof(rect_t), NULL, NULL);
}

static void setup_destroy_with_two_components()
{
    // Create ECS instance
    ecs = ecs_new(MIN_ENTITIES, NULL);

    PosComponent  = ecs_define_component(ecs, sizeof(v2d_t),  NULL, NULL);
    RectComponent = ecs_define_component(ecs, sizeof(rect_t), NULL, NULL);

    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, PosComponent, NULL);
        ecs_add(ecs, entity, RectComponent, NULL);
    }
}

static void setup_three_systems_min()
{
    ecs = ecs_new(MIN_ENTITIES, NULL);

    PosComponent = ecs_define_component(ecs, sizeof(v2d_t), NULL, NULL);
    DirComponent = ecs_define_component(ecs, sizeof(v2d_t), NULL, NULL);
    ComflabComponent = ecs_define_component(ecs, sizeof(comflab_t), NULL, NULL);
    RectComponent = ecs_define_component(ecs, sizeof(rect_t), NULL, NULL);

    MovementSystem = ecs_define_system(ecs, 0, movement_system, NULL, NULL, NULL);
    ecs_require_component(ecs, MovementSystem, PosComponent);
    ecs_require_component(ecs, MovementSystem, DirComponent);

    ComflabSystem = ecs_define_system(ecs, 0, comflab_system, NULL, NULL, NULL);
    ecs_require_component(ecs, ComflabSystem, ComflabComponent);

    BoundsSystem = ecs_define_system(ecs, 0, bounds_system, NULL, NULL, NULL);
    ecs_require_component(ecs, BoundsSystem, RectComponent);
}

static void setup_three_systems_max()
{
    ecs = ecs_new(MAX_ENTITIES, NULL);

    PosComponent = ecs_define_component(ecs, sizeof(v2d_t), NULL, NULL);
    DirComponent = ecs_define_component(ecs, sizeof(v2d_t), NULL, NULL);
    ComflabComponent = ecs_define_component(ecs, sizeof(comflab_t), NULL, NULL);
    RectComponent = ecs_define_component(ecs, sizeof(rect_t), NULL, NULL);

    MovementSystem = ecs_define_system(ecs, 0, movement_system, NULL, NULL, NULL);
    ecs_require_component(ecs, MovementSystem, PosComponent);
    ecs_require_component(ecs, MovementSystem, DirComponent);

    ComflabSystem = ecs_define_system(ecs, 0, comflab_system, NULL, NULL, NULL);
    ecs_require_component(ecs, ComflabSystem, ComflabComponent);

    BoundsSystem = ecs_define_system(ecs, 0, bounds_system, NULL, NULL, NULL);
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
    PosComponent = ecs_define_component(ecs, sizeof(v2d_t), NULL, NULL);

    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_entity_t entity = ecs_create(ecs);

        // Add components
        ecs_add(ecs, entity, PosComponent, NULL);
    }
}

/*=============================================================================
 * Update function callbacks
 *============================================================================*/

ecs_ret_t movement_system(ecs_t* ecs,
                          ecs_entity_t* entities,
                          size_t entity_count,
                          void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        // Get entity ID
        ecs_entity_t entity = entities[i];

        v2d_t* pos = ecs_get(ecs, entity, PosComponent);
        v2d_t* dir = ecs_get(ecs, entity, DirComponent);

        pos->x += pos->x + dir->x * 1.f / 60.f;
        pos->y += pos->y + dir->y * 1.f / 60.f;
    }

    return 0;
}

ecs_ret_t comflab_system(ecs_t* ecs,
                        ecs_entity_t* entities,
                        size_t entity_count,
                        void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        // Get entity ID
        ecs_entity_t entity = entities[i];

        comflab_t* comflab = ecs_get(ecs, entity, ComflabComponent);
        comflab->thingy *= 1.000001f;
    	comflab->mingy = !comflab->mingy;
	    comflab->dingy++;
    }

    return 0;
}

ecs_ret_t bounds_system(ecs_t* ecs,
                        ecs_entity_t* entities,
                        size_t entity_count,
                        void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        // Get entity ID
        ecs_entity_t entity = entities[i];

        rect_t* bounds = ecs_get(ecs, entity, RectComponent);

        bounds->x = 1;
        bounds->y = 1;
        bounds->w = 1;
        bounds->h = 1;
    }

    return 0;
}

ecs_ret_t queue_destroy_system(ecs_t* ecs,
                               ecs_entity_t* entities,
                               size_t entity_count,
                               void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
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
    for (size_t i = 0; i < MAX_ENTITIES; i++)
        ecs_create(ecs);
}

// Creates entity IDs as fast as possible and immediately destroys the
// coresponding entity
static void bench_create_destroy()
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
        ecs_destroy(ecs, ecs_create(ecs));
}

static void bench_destroy_with_two_components()
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = {(ecs_id_t)i};
        ecs_destroy(ecs, entity);
    }
}

static void bench_create_with_two_components()
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_entity_t entity = ecs_create(ecs);

        // Add components
        ecs_add(ecs, entity, PosComponent, NULL);
        ecs_add(ecs, entity, RectComponent, NULL);
    }
}

// Adds components to entities and assigns values to them
static void bench_add_remove()
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, PosComponent, NULL);
        ecs_remove(ecs, entity, PosComponent);
    }
}

// Adds components to entities and assigns values to them
static void bench_add_assign()
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_entity_t entity = ecs_create(ecs);

        // Add components
        v2d_t*  pos  = (v2d_t*) ecs_add(ecs, entity, PosComponent,  NULL);
        rect_t* rect = (rect_t*)ecs_add(ecs, entity, RectComponent, NULL);

        // Set concrete component values
        *pos  = (v2d_t) { 1, 2 };
        *rect = (rect_t){ 1, 2, 3, 4 };
    }
}

// Adds components to entities, retrieves the components, and assigns
// values to them
static void bench_get()
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity id
        ecs_entity_t entity = {(ecs_id_t)i};
        ecs_get(ecs, entity, PosComponent);
    }
}

static void bench_queue_destroy()
{
    QueueDestroySystem = ecs_define_system(ecs, 0, queue_destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, QueueDestroySystem, PosComponent);
    ecs_require_component(ecs, QueueDestroySystem, RectComponent);

    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_create(ecs);
    }

    ecs_run_system(ecs, QueueDestroySystem, 0);
}

static void bench_three_systems()
{
    // Create entities
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_entity_t entity = ecs_create(ecs);

        // Add components
        v2d_t*  pos    = ecs_add(ecs, entity, PosComponent, NULL);
        v2d_t*  dir    = ecs_add(ecs, entity, DirComponent, NULL);
        rect_t* bounds = ecs_add(ecs, entity, RectComponent, NULL);

        if (i % 2 == 0)
        {
            comflab_t* comflab = ecs_add(ecs, entity, ComflabComponent, NULL);
            *comflab = (comflab_t){ 0 };
        }

        // Set concrete component values
        *pos    = (v2d_t)  { 0 };
        *dir    = (v2d_t)  { 0 };
        *bounds = (rect_t) { 0 };
    }

    // Run the system
    ecs_run_system(ecs, MovementSystem, 0);
    ecs_run_system(ecs, ComflabSystem, 0);
    ecs_run_system(ecs, BoundsSystem, 0);
}

static void bench_three_systems_min()
{
    bench_three_systems();
}

static void bench_three_systems_max()
{
    bench_three_systems();
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
    BENCH_RUN(bench_three_systems_min, setup_three_systems_min, teardown);
    BENCH_RUN(bench_three_systems_max, setup_three_systems_max, teardown);

    printf("---------------------------------------------------------------\n");

    return 0;
}

