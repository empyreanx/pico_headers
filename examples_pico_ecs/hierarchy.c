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

// This example implements parent/child entity hierarchies on top of the
// public pico_ecs API, using an ordinary component. No engine support is
// required.
//
// The hierarchy is stored as parent/first-child/sibling entity links inside a
// node component. The children of an entity form a doubly-linked list
// threaded through the children themselves, so attaching (node_child_of),
// detaching (node_detach), and destroying entities are all O(1) and require
// no additional allocations.
//
// The component's on_remove callback detaches the entity and orphans its
// children, so a plain ecs_destroy leaves the hierarchy consistent: the
// destroyed entity's children simply become root entities. To destroy an
// entire subtree, use node_destroy_hierarchy.
//
// One caveat: destroys issued from inside a system are deferred, and entities
// queued for destruction may not be accessed until the destroy is applied.
// Destroying entities with node_destroy/node_destroy_hierarchy handles this:
// they unlink the entity up front, while its parent, children, and siblings
// are all still alive, so no hierarchy operation ever needs to touch a queued
// entity. So the rule is simply: destroy entities that have a node component
// with node_destroy (or node_destroy_hierarchy for a subtree) rather than
// ecs_destroy. The on_remove callback remains as a safety net for direct
// ecs_destroy calls made outside of systems.

// Pull in the ECS implementation
#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <stdio.h>

/*=============================================================================
 * Components
 *============================================================================*/

// Hierarchy links. All links are initialized to an invalid handle by the
// component's default value
typedef struct
{
    ecs_entity_t parent;
    ecs_entity_t first_child;
    ecs_entity_t next_sibling;
    ecs_entity_t prev_sibling;
} node_t;

// A human readable name, so the demo can print the tree
typedef struct
{
    const char* str;
} name_t;

// A position relative to the entity's parent
typedef struct
{
    float x, y;
} pos_t;

ecs_comp_t NodeComp;
ecs_comp_t NameComp;
ecs_comp_t PosComp;

/*=============================================================================
 * Hierarchy functions
 *============================================================================*/

static node_t* node_get(ecs_t* ecs, ecs_entity_t entity)
{
    return ecs_get(ecs, entity, NodeComp);
}

ecs_entity_t node_get_parent(ecs_t* ecs, ecs_entity_t entity)
{
    return node_get(ecs, entity)->parent;
}

ecs_entity_t node_get_first_child(ecs_t* ecs, ecs_entity_t entity)
{
    return node_get(ecs, entity)->first_child;
}

ecs_entity_t node_get_next_sibling(ecs_t* ecs, ecs_entity_t entity)
{
    return node_get(ecs, entity)->next_sibling;
}

// Detaches an entity from its parent, making it a root entity. Does nothing
// if the entity has no parent
void node_detach(ecs_t* ecs, ecs_entity_t entity)
{
    node_t* node = node_get(ecs, entity);

    if (ECS_IS_INVALID(node->parent))
        return;

    node_t* parent = node_get(ecs, node->parent);

    // Unlink the entity from its parent's child list
    if (parent->first_child.id == entity.id)
        parent->first_child = node->next_sibling;

    if (!ECS_IS_INVALID(node->prev_sibling))
        node_get(ecs, node->prev_sibling)->next_sibling = node->next_sibling;

    if (!ECS_IS_INVALID(node->next_sibling))
        node_get(ecs, node->next_sibling)->prev_sibling = node->prev_sibling;

    node->parent       = ECS_INVALID_ENTITY;
    node->next_sibling = ECS_INVALID_ENTITY;
    node->prev_sibling = ECS_INVALID_ENTITY;
}

// Makes one entity a child of another, first detaching the child from its
// current parent, if it has one. The child must not be an ancestor of the
// parent
void node_child_of(ecs_t* ecs, ecs_entity_t child, ecs_entity_t parent)
{
    node_detach(ecs, child);

    node_t* child_node  = node_get(ecs, child);
    node_t* parent_node = node_get(ecs, parent);

    // Push the child onto the front of the parent's child list
    child_node->parent       = parent;
    child_node->prev_sibling = ECS_INVALID_ENTITY;
    child_node->next_sibling = parent_node->first_child;

    if (!ECS_IS_INVALID(parent_node->first_child))
        node_get(ecs, parent_node->first_child)->prev_sibling = child;

    parent_node->first_child = child;
}

// Orphans the entity's children (they become root entities). Only the upward
// and sibling links are cleared; each child keeps its own children
static void node_orphan_children(ecs_t* ecs, ecs_entity_t entity)
{
    node_t* node = node_get(ecs, entity);

    ecs_entity_t child = node->first_child;

    while (!ECS_IS_INVALID(child))
    {
        node_t* child_node = node_get(ecs, child);
        ecs_entity_t next = child_node->next_sibling;

        child_node->parent       = ECS_INVALID_ENTITY;
        child_node->next_sibling = ECS_INVALID_ENTITY;
        child_node->prev_sibling = ECS_INVALID_ENTITY;

        child = next;
    }

    node->first_child = ECS_INVALID_ENTITY;
}

// Destroys an entity, unlinking it from the hierarchy first: the entity is
// detached from its parent, and its children are orphaned, becoming root
// entities
//
// Unlinking before destroying matters when the destroy is issued from inside
// a system: ecs_destroy is deferred until the system completes, and entities
// queued for destruction may not be accessed in the meantime. By unlinking up
// front, while the entity's parent, children, and siblings are all still
// alive, the entity carries no links into the deferred destroy, so nothing
// ever needs to touch a queued entity
void node_destroy(ecs_t* ecs, ecs_entity_t entity)
{
    node_detach(ecs, entity);
    node_orphan_children(ecs, entity);
    ecs_destroy(ecs, entity);
}

// Destroys an entity and all of its descendants, children first
void node_destroy_hierarchy(ecs_t* ecs, ecs_entity_t entity)
{
    ecs_entity_t child = node_get_first_child(ecs, entity);

    while (!ECS_IS_INVALID(child))
    {
        // Save the next sibling now: destroying the child unlinks it from
        // the child list
        ecs_entity_t next = node_get_next_sibling(ecs, child);

        node_destroy_hierarchy(ecs, child);

        child = next;
    }

    node_destroy(ecs, entity);
}

// Called by the ECS when a node component is removed, which includes entity
// destruction. Detaching here keeps the hierarchy consistent no matter how
// the entity is destroyed
static void node_on_remove(ecs_t* ecs,
                           ecs_entity_t entity,
                           ecs_comp_t comp,
                           void* udata)
{
    (void)comp;
    (void)udata;

    node_detach(ecs, entity);
    node_orphan_children(ecs, entity);
}

/*=============================================================================
 * Demo
 *============================================================================*/

// Register components
void register_components(ecs_t* ecs)
{
    // All hierarchy links start out invalid
    node_t default_node =
    {
        .parent       = { ECS_INVALID_ID },
        .first_child  = { ECS_INVALID_ID },
        .next_sibling = { ECS_INVALID_ID },
        .prev_sibling = { ECS_INVALID_ID }
    };

    ecs_comp_desc_t desc =
    {
        .on_remove_cb  = node_on_remove,
        .default_value = &default_node
    };

    NodeComp = ecs_define_component(ecs, sizeof(node_t), &desc);
    NameComp = ecs_define_component(ecs, sizeof(name_t), NULL);
    PosComp  = ecs_define_component(ecs, sizeof(pos_t),  NULL);
}

// Creates a named entity at a position relative to its parent
ecs_entity_t spawn(ecs_t* ecs, const char* name, float x, float y)
{
    ecs_entity_t entity = ecs_create(ecs);

    ecs_add(ecs, entity, NodeComp, NULL);
    ecs_add(ecs, entity, NameComp, NULL);
    ecs_add(ecs, entity, PosComp,  NULL);

    name_t* name_comp = ecs_get(ecs, entity, NameComp);
    name_comp->str = name;

    pos_t* pos = ecs_get(ecs, entity, PosComp);
    pos->x = x;
    pos->y = y;

    return entity;
}

// Recursively prints an entity and its descendants. World positions are
// computed by accumulating local positions down the tree, which is the
// typical use case for a hierarchy (e.g. a scene graph)
void print_tree(ecs_t* ecs, ecs_entity_t entity, float world_x, float world_y, int depth)
{
    name_t* name = ecs_get(ecs, entity, NameComp);
    pos_t*  pos  = ecs_get(ecs, entity, PosComp);

    world_x += pos->x;
    world_y += pos->y;

    printf("%*s%s (local %.0f,%.0f world %.0f,%.0f)\n",
           depth * 4, "", name->str, pos->x, pos->y, world_x, world_y);

    ecs_entity_t child = node_get_first_child(ecs, entity);

    while (!ECS_IS_INVALID(child))
    {
        print_tree(ecs, child, world_x, world_y, depth + 1);
        child = node_get_next_sibling(ecs, child);
    }
}

int main()
{
    // Creates concrete ECS instance
    ecs_t* ecs = ecs_new(1024, NULL);

    register_components(ecs);

    // Build a small scene graph:
    //
    // ship
    //   engine
    //   turret
    //     barrel
    ecs_entity_t ship   = spawn(ecs, "ship",   100.0f, 50.0f);
    ecs_entity_t engine = spawn(ecs, "engine", -10.0f,  0.0f);
    ecs_entity_t turret = spawn(ecs, "turret",   5.0f,  2.0f);
    ecs_entity_t barrel = spawn(ecs, "barrel",   3.0f,  0.0f);

    (void)ship;
    (void)engine;
    (void)turret;
    (void)barrel;

    node_child_of(ecs, engine, ship);
    node_child_of(ecs, turret, ship);
    node_child_of(ecs, barrel, turret);

    printf("---------------------------------------------------------------\n");
    printf("Initial hierarchy:\n");
    print_tree(ecs, ship, 0.0f, 0.0f, 0);

    // Moving the ship moves everything attached to it
    pos_t* ship_pos = ecs_get(ecs, ship, PosComp);
    ship_pos->x += 25.0f;

    printf("---------------------------------------------------------------\n");
    printf("After moving the ship:\n");
    print_tree(ecs, ship, 0.0f, 0.0f, 0);

    // Reparenting: move the barrel from the turret to the engine
    node_child_of(ecs, barrel, engine);

    printf("---------------------------------------------------------------\n");
    printf("After reparenting the barrel to the engine:\n");
    print_tree(ecs, ship, 0.0f, 0.0f, 0);

    // Destroying an entity orphans its children: the barrel becomes a root
    // entity
    node_destroy(ecs, engine);

    printf("---------------------------------------------------------------\n");
    printf("After destroying the engine (the barrel is orphaned):\n");
    print_tree(ecs, ship, 0.0f, 0.0f, 0);
    print_tree(ecs, barrel, 0.0f, 0.0f, 0);

    // Destroying a subtree destroys the entity and all of its descendants
    node_destroy_hierarchy(ecs, ship);

    printf("---------------------------------------------------------------\n");
    printf("After destroying the ship hierarchy:\n");
    printf("ship exists: %s\n",   ecs_is_ready(ecs, ship)   ? "yes" : "no");
    printf("turret exists: %s\n", ecs_is_ready(ecs, turret) ? "yes" : "no");
    printf("barrel exists: %s\n", ecs_is_ready(ecs, barrel) ? "yes" : "no");
    printf("---------------------------------------------------------------\n");

    ecs_free(ecs);

    return 0;
}