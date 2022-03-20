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

#define PICO_ECS_MAX_ENTITIES (1000 * 1000)
#define PICO_ECS_MAX_SYSTEMS 11

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
enum
{
    IterateSystem,
    QueueSyncSystem,
    QueueDestroySystem,
    MovementSystem,
    ComflabSystem
};

// Component IDs
enum {
    PosComponent,
    DirComponent,
    RectComponent,
    ComflabComponent
};

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

ecs_ret_t movement_update(ecs_t* ecs, ecs_id_t* entities,
                          int entity_count, ecs_dt_t dt,
                          void* udata);

ecs_ret_t comflab_update(ecs_t* ecs, ecs_id_t* entities,
                         int entity_count, ecs_dt_t dt,
                         void* udata);

ecs_ret_t movement_update(ecs_t* ecs, ecs_id_t* entities,
                          int entity_count, ecs_dt_t dt,
                          void* udata);

ecs_ret_t iterate_assign_update(ecs_t* ecs, ecs_id_t* entities,
                                int entity_count, ecs_dt_t dt,
                                void* udata);

ecs_ret_t queue_sync_update(ecs_t* ecs, ecs_id_t* entities,
                            int entity_count, ecs_dt_t dt,
                            void* udata);

ecs_ret_t queue_destroy_update(ecs_t* ecs, ecs_id_t* entities,
                               int entity_count, ecs_dt_t dt,
                               void* udata);

static void setup()
{
    // Create ECS instance
    ecs = ecs_new(NULL);

    // Register two new components
    ecs_register_component(ecs, PosComponent, sizeof(v2d_t));
    ecs_register_component(ecs, RectComponent, sizeof(rect_t));
}

static void setup_abeimler()
{
    ecs = ecs_new(NULL);

    ecs_register_component(ecs, PosComponent, sizeof(v2d_t));
    ecs_register_component(ecs, DirComponent, sizeof(v2d_t));
    ecs_register_component(ecs, ComflabComponent, sizeof(comflab_t));

    ecs_register_system(ecs, MovementSystem, movement_update, NULL, NULL, NULL);
    ecs_require_component(ecs, MovementSystem, PosComponent);
    ecs_require_component(ecs, MovementSystem, DirComponent);

    ecs_register_system(ecs, ComflabSystem, comflab_update, NULL, NULL, NULL);
    ecs_require_component(ecs, ComflabSystem, ComflabComponent);

    for (ecs_id_t i = 0; i < PICO_ECS_MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        v2d_t* pos = ecs_add(ecs, id, PosComponent);
        v2d_t* dir = ecs_add(ecs, id, DirComponent);

        if (i % 2 == 0)
        {
            comflab_t* comflab = ecs_add(ecs, id, ComflabComponent);
            *comflab = (comflab_t){ 0 };
        }

        // Set concrete component values
        *pos  = (v2d_t) { 0 };
        *dir  = (v2d_t) { 0 };

        // Adds entity to systems
        ecs_sync(ecs, id);
    }
}

// Runs after benchmark function
static void teardown()
{
    // Destroy ECS instance
    ecs_free(ecs);
    ecs = NULL;
}

// Runs before benchmark function
static void setup_with_entities()
{
    // Create ECS instance
    ecs = ecs_new(NULL);

    // Register two new components
    ecs_register_component(ecs, PosComponent, sizeof(v2d_t));
    ecs_register_component(ecs, RectComponent, sizeof(rect_t));

    ecs_register_system(ecs, IterateSystem, iterate_assign_update, NULL, NULL , NULL);
    ecs_require_component(ecs, IterateSystem, PosComponent);
    ecs_require_component(ecs, IterateSystem, RectComponent);

    ecs_register_system(ecs, QueueSyncSystem, queue_sync_update, NULL, NULL , NULL);
    ecs_require_component(ecs, QueueSyncSystem, PosComponent);
    ecs_require_component(ecs, QueueSyncSystem, RectComponent);

    ecs_register_system(ecs, QueueDestroySystem, queue_destroy_update, NULL, NULL, NULL);
    ecs_require_component(ecs, QueueDestroySystem, PosComponent);
    ecs_require_component(ecs, QueueDestroySystem, RectComponent);

    for (ecs_id_t i = 0; i < PICO_ECS_MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        v2d_t*  pos  = (v2d_t*)ecs_add(ecs, id, PosComponent);
        rect_t* rect = (rect_t*)ecs_add(ecs, id, RectComponent);

        // Set concrete component values
        *pos  = (v2d_t) { 1, 2 };
        *rect = (rect_t){ 1, 2, 3, 4 };

        // Adds entity to systems
        ecs_sync(ecs, id);
    }
}

/*=============================================================================
 * Update function callbacks
 *============================================================================*/

// System update function that iterates over the entities and assigns values to
// their components
ecs_ret_t iterate_assign_update(ecs_t* ecs,
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

        // Get entity components
        v2d_t*  pos  = (v2d_t*)ecs_get(ecs, id, PosComponent);
        rect_t* rect = (rect_t*)ecs_get(ecs, id, RectComponent);

        // Set concrete component values
        *pos  = (v2d_t) { 2, 3 };
        *rect = (rect_t){ 2, 3, 4, 5 };
    }

    return 0;
}

// Queues sync calls on all entitiea
ecs_ret_t queue_sync_update(ecs_t* ecs,
                             ecs_id_t* entities,
                             int entity_count,
                             ecs_dt_t dt,
                             void* udata)
{
    (void)dt;
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_queue_sync(ecs, entities[i]);
    }

    return 0;
}

// Call queue destroy on all entities
ecs_ret_t queue_destroy_update(ecs_t* ecs,
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

ecs_ret_t movement_update(ecs_t* ecs,
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

ecs_ret_t comflab_update(ecs_t* ecs,
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

/*=============================================================================
 * Benchmark functions
 *============================================================================*/

// Creates entity IDs as fast as possible
static void bench_create()
{
    for (ecs_id_t i = 0; i < PICO_ECS_MAX_ENTITIES; i++)
        ecs_create(ecs);
}

// Creates entity IDs as fast as possible and immediately destroys the
// coresponding entity
static void bench_create_destroy()
{
    for (ecs_id_t i = 0; i < PICO_ECS_MAX_ENTITIES; i++)
        ecs_destroy(ecs, ecs_create(ecs));
}

// Adds components to entities and assigns values to them
static void bench_add_assign()
{
    for (ecs_id_t i = 0; i < PICO_ECS_MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        v2d_t*  pos  = (v2d_t*)ecs_add(ecs, id, PosComponent);
        rect_t* rect = (rect_t*)ecs_add(ecs, id, RectComponent);

        // Set concrete component values
        *pos  = (v2d_t) { 1, 2 };
        *rect = (rect_t){ 1, 2, 3, 4 };
    }
}

// Adds components to entities, retrieves the components, and assigns
// values to them
static void bench_add_get_assign()
{
    for (ecs_id_t i = 0; i < PICO_ECS_MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        ecs_add(ecs, id, PosComponent);
        ecs_add(ecs, id, RectComponent);

        // Get components
        v2d_t*  pos  = (v2d_t*)ecs_get(ecs, id, PosComponent);
        rect_t* rect = (rect_t*)ecs_get(ecs, id, RectComponent);

        // Set concrete component values
        *pos  = (v2d_t) { 1, 2 };
        *rect = (rect_t){ 1, 2, 3, 4 };
    }
}

// Adds components to entities, assigns values to them, and adds them to
// the systems
static void bench_add_assign_sync()
{
    for (ecs_id_t i = 0; i < PICO_ECS_MAX_ENTITIES; i++)
    {
        // Create entity
        ecs_id_t id = ecs_create(ecs);

        // Add components
        v2d_t*  pos  = (v2d_t*)ecs_add(ecs, id, PosComponent);
        rect_t* rect = (rect_t*)ecs_add(ecs, id, RectComponent);

        // Set concrete component values
        *pos  = (v2d_t) { 1, 2 };
        *rect = (rect_t){ 1, 2, 3, 4 };

        // Adds entity to systems
        ecs_sync(ecs, id);
    }
}

// Registers a new system. Adds components to entities, and assigns values to
// them. Adds the entities to the system, and executes the system.
static void bench_iterate_assign()
{
    ecs_update_system(ecs, IterateSystem, 0.0f);
}

static void bench_iterate_assign2()
{
    // Run the system
    ecs_update_system(ecs, IterateSystem, 0.0f);
    ecs_update_system(ecs, IterateSystem, 0.0f);
}

// Adds a system that enqueues sync on all entities.
static void bench_queue_sync()
{
    // Run the system
    ecs_update_system(ecs, QueueSyncSystem, 0.0f);
}

// Adds a system that enqueues destroy on all entities.
static void bench_queue_destroy()
{
    // Run the system
    ecs_update_system(ecs, QueueDestroySystem, 0.0f);
}

static void bench_abeimler()
{
    // Run the system
    ecs_update_system(ecs, MovementSystem, 1.0f);
    ecs_update_system(ecs, ComflabSystem, 1.0f);
}

int main()
{
    printf("===============================================================\n");

    printf("Number of entities: %u\n", PICO_ECS_MAX_ENTITIES);

    BENCH_RUN(bench_create, setup, teardown);
    BENCH_RUN(bench_create_destroy, setup, teardown);
    BENCH_RUN(bench_add_assign, setup, teardown);
    BENCH_RUN(bench_add_get_assign, setup, teardown);
    BENCH_RUN(bench_add_assign_sync, setup, teardown);
    BENCH_RUN(bench_iterate_assign, setup_with_entities, teardown);
    BENCH_RUN(bench_iterate_assign2, setup_with_entities, teardown);
    BENCH_RUN(bench_queue_sync, setup_with_entities, teardown);
    BENCH_RUN(bench_queue_destroy, setup_with_entities, teardown);
    BENCH_RUN(bench_abeimler, setup_abeimler, teardown);

    printf("---------------------------------------------------------------\n");

    return 0;
}

//#define ECS_DEBUG
#define PICO_ECS_MAX_COMPONENTS 64
#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

