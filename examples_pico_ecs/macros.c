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

#include <stdio.h>

#include "macros.h"

// Pull in the ECS implementation
#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

ECS_DEFINE_COMPONENT(pos_t);
ECS_DEFINE_COMPONENT(vel_t);
ECS_DEFINE_COMPONENT(rect_t);

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

static ecs_t* ecs = NULL;

ecs_system_t sys1;
ecs_system_t sys2;
ecs_system_t sys3;

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
    // Define systems
    sys1 = ecs_define_system(ecs, system_update, NULL);
    sys2 = ecs_define_system(ecs, system_update, NULL);
    sys3 = ecs_define_system(ecs, system_update, NULL);

    // System 1 requires the position component
    ecs_require_(ecs, sys1, pos_t);

    // System 2 requires both position and velocity components
    ecs_require_(ecs, sys2, pos_t);
    ecs_require_(ecs, sys2, vel_t);

    // System3 requires the position, velocity, and rect components
    ecs_require_(ecs, sys3, pos_t);
    ecs_require_(ecs, sys3, vel_t);
    ecs_require_(ecs, sys3, rect_t);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    ecs = ecs_new(32, NULL);

    ecs_register_component(ecs, pos_t, NULL);
    ecs_register_component(ecs, vel_t, NULL);
    ecs_register_component(ecs, rect_t, NULL);

    register_systems(ecs);

    // Create three entities
    ecs_entity_t e1 = ecs_create(ecs);
    ecs_entity_t e2 = ecs_create(ecs);
    ecs_entity_t e3 = ecs_create(ecs);

    // Add components to entities
    printf("---------------------------------------------------------------\n");
    printf("Created entities: %lu, %lu, %lu\n", e1.id, e2.id, e3.id);
    printf("---------------------------------------------------------------\n");

    printf("pos_t added to: %lu\n", e1.id);
    ecs_add_(ecs, e1, pos_t);

    printf("---------------------------------------------------------------\n");
    printf("pos_t added to: %lu\n", e2.id);
    printf("vel_t added to: %lu\n", e2.id);

    ecs_add_(ecs, e2, pos_t);
    ecs_add_(ecs, e2, vel_t);

    printf("---------------------------------------------------------------\n");
    printf("pos_t added to: %lu\n",  e3.id);
    printf("vel_t added to: %lu\n",  e3.id);
    printf("rect_t added to: %lu\n", e3.id);

    ecs_add_(ecs, e3, pos_t);
    ecs_add_(ecs, e3, vel_t);
    ecs_add_(ecs, e3, rect_t);

    printf("---------------------------------------------------------------\n");

    // Manually execute the systems
    printf("Executing system 1\n");
    ecs_run_system(ecs, sys1, 0); // Output: e1 e2 e3

    printf("Executing system 2\n");
    ecs_run_system(ecs, sys2, 0); // Output: e2 e3

    printf("Executing system 3\n");
    ecs_run_system(ecs, sys3, 0); // Output: e3

    printf("---------------------------------------------------------------\n");

    ecs_free(ecs);

    return 0;
}