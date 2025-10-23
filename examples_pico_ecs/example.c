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

// Pull in the ECS implementation
#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <stdio.h>

// Steps to create an ECS:
// 1. Create an ECS instance (using ecs_new)
// 2. Define concrete components types (structs)
// 3. Assign each a unique, zero-based ID (using an enum is recommended)
// 4. Write system update callbacks
// 5. Define zero-based system IDs (using an enum is recommended)
// 6. Register components
// 7. Register systems
// 8. Associate components with systems (using ecs_require_component)

// Concrete component structs
typedef struct
{
    float x, y;
} pos_t;

typedef struct
{
    float vx, vy;
} vel_t;

typedef struct
{
    int x, y, w, h;
} rect_t;

// Component types
ecs_comp_t PosComp;
ecs_comp_t VelComp;
ecs_comp_t RectComp;

// System types
ecs_system_t System1;
ecs_system_t System2;
ecs_system_t System3;

// Register components
void register_components(ecs_t* ecs)
{
    PosComp  = ecs_define_component(ecs, sizeof(pos_t),  NULL, NULL);
    VelComp  = ecs_define_component(ecs, sizeof(vel_t),  NULL, NULL);
    RectComp = ecs_define_component(ecs, sizeof(rect_t), NULL, NULL);
}

// System that prints the entity IDs of entities associated with this system
ecs_ret_t system_update(ecs_t* ecs,
                       ecs_entity_t* entities,
                       size_t entity_count,
                       void* udata)
{
    (void)ecs;
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        printf("%lu ", entities[i].id);
    }

    printf("\n");

    return 0;
}

// Register all systems and required relationships
void register_systems(ecs_t* ecs)
{
    // Register systems
    System1 = ecs_define_system(ecs, 0, system_update, NULL, NULL, NULL);
    System2 = ecs_define_system(ecs, 0, system_update, NULL, NULL, NULL);
    System3 = ecs_define_system(ecs, 0, system_update, NULL, NULL, NULL);

    // System1 requires PosComp compnents
    ecs_require_component(ecs, System1, PosComp);

    // System2 requires both PosComp and VelComp components
    ecs_require_component(ecs, System2, PosComp);
    ecs_require_component(ecs, System2, VelComp);

    // System3 requires the PosComp, VelComp, and RectComp components
    ecs_require_component(ecs, System3, PosComp);
    ecs_require_component(ecs, System3, VelComp);
    ecs_require_component(ecs, System3, RectComp);
}


int main()
{
    // Creates concrete ECS instance
    ecs_t* ecs = ecs_new(1024, NULL);

    // Register components and systems
    register_components(ecs);
    register_systems(ecs);

    // Create three entities
    ecs_entity_t e1 = ecs_create(ecs);
    ecs_entity_t e2 = ecs_create(ecs);
    ecs_entity_t e3 = ecs_create(ecs);

    // Add components to entities
    printf("---------------------------------------------------------------\n");
    printf("Created entities: %lu, %lu, %lu\n", e1.id, e2.id, e3.id);
    printf("---------------------------------------------------------------\n");

    printf("PosComp added to: %lu\n", e1.id);
    ecs_add(ecs, e1, PosComp, NULL);

    printf("---------------------------------------------------------------\n");
    printf("PosComp added to: %lu\n",  e2.id);
    printf("VeloComp added to: %lu\n", e2.id);

    ecs_add(ecs, e2, PosComp, NULL);
    ecs_add(ecs, e2, VelComp, NULL);

    printf("---------------------------------------------------------------\n");
    printf("PosComp added to: %lu\n",  e3.id);
    printf("VeloComp added to: %lu\n", e3.id);
    printf("RectComp added to: %lu\n", e3.id);

    ecs_add(ecs, e3, PosComp, NULL);
    ecs_add(ecs, e3, VelComp, NULL);
    ecs_add(ecs, e3, RectComp, NULL);

    printf("---------------------------------------------------------------\n");

    // Manually execute the systems
    printf("Executing system 1\n");
    ecs_run_system(ecs, System1, 0); // Output: e1 e2 e3

    printf("Executing system 2\n");
    ecs_run_system(ecs, System2, 0); // Output: e2 e3

    printf("Executing system 3\n");
    ecs_run_system(ecs, System3, 0); // Output: e3

    printf("---------------------------------------------------------------\n");

    ecs_free(ecs);

    return 0;
}

