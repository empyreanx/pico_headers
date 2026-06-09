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

// Helper macros for referring to pico_ecs components by their C type name
// rather than passing ecs_comp_t handles.

#ifndef MACROS_H
#define MACROS_H

// Name of the global handle for a given component type.
#define ECS_TYPE(type) (comp_##type)

// Declare / define the component handle
#define ECS_DECLARE_COMPONENT(type) extern ecs_comp_t ECS_TYPE(type)
#define ECS_DEFINE_COMPONENT(type) ecs_comp_t ECS_TYPE(type)

// Register a components
#define ecs_register_component(ecs, type, desc) \
    ECS_TYPE(type) = ecs_define_component((ecs), sizeof(type), (desc))

// Constrain a system's entity match by component type name.
#define ecs_require_(ecs, sys, type) \
    ecs_require((ecs), sys, ECS_TYPE(type))

#define ecs_exclude_(ecs, sys, type) \
    ecs_exclude((ecs), ECS_TYPE(type))

// Per-entity component operations, addressed by component type name.
#define ecs_add_(ecs, entity, type) \
    ecs_add((ecs), (entity), ECS_TYPE(type))

 #define ecs_remove_(ecs, entity, type) \
    ecs_remove((ecs), (entity), ECS_TYPE(type))

#define ecs_has_(ecs, entity, type) \
    ecs_has((ecs), (entity), ECS_TYPE(type))

#define ecs_set_(ecs, entity, type, data) \
    ecs_set((ecs), (entity), ECS_TYPE(type), (data))

// Returns a typed (type*) pointer to the entity's component data.
#define ecs_get_(ecs, entity, type) \
    ((type*)ecs_get((ecs), (entity), ECS_TYPE(type))

#endif // MACROS_H