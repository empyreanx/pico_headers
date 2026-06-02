#pragma once

#include "../pico_ecs.h"
#include "../pico_unit.h"

#define MIN_ENTITIES (1 * 1024)
#define MAX_ENTITIES (8 * 1024)

typedef struct
{
    bool used;
} comp_t;

extern ecs_t* ecs;
extern ecs_comp_t comp1;
extern ecs_comp_t comp2;
extern ecs_comp_t comp3;

extern ecs_system_t sys1;
extern ecs_system_t sys2;
