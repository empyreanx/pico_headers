/*=============================================================================
 * This is free and unencumbered software released into the public domain.
 *
 * For more information, please refer to <http://unlicense.org/>
 *============================================================================*/

// Demonstrates the built-in lifecycle callbacks, which are thin wrappers over
// the event system:
//
//   Component events: ecs_on_add / ecs_on_set / ecs_on_remove
//   System events:    ecs_on_join / ecs_on_leave
//
// Each registration takes a `sync` flag:
//   sync = true   -> the callback runs immediately, inline with the mutation
//   sync = false  -> the callback is deferred and runs on ecs_dispatch
//
// A synchronous on_add behaves like a constructor (it can initialize the
// component from the args passed to ecs_add); a synchronous on_remove behaves
// like a destructor (the entity is still alive when it runs).

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <stdio.h>

// ---------------------------------------------------------------------------
// Components
// ---------------------------------------------------------------------------
typedef struct
{
    float x, y;
} pos_t;

typedef struct
{
    int   fd;       // pretend this owns a resource that must be released
    char  name[16];
} handle_t;

ecs_comp_t PosComp;
ecs_comp_t HandleComp;
ecs_system_t MoveSystem;

// ---------------------------------------------------------------------------
// Component callbacks
// ---------------------------------------------------------------------------

// Constructor: initialize the component from the args forwarded by ecs_add.
// Registered with sync = true so it runs during ecs_add and the component is
// usable immediately.
void handle_on_add(ecs_t* ecs,
                   ecs_entity_t entity,
                   ecs_comp_t comp,
                   const void* args,
                   void* udata)
{
    (void)udata;

    handle_t* h = ecs_get(ecs, entity, comp);

    if (args)
        *h = *(const handle_t*)args;

    printf("  [on_add]    entity %lu acquired handle '%s' (fd=%d)\n",
           entity.id, h->name, h->fd);
}

// Destructor: release the owned resource. Registered with sync = true so the
// entity is still alive and the component still readable when it runs.
void handle_on_remove(ecs_t* ecs,
                      ecs_entity_t entity,
                      ecs_comp_t comp,
                      void* udata)
{
    (void)udata;

    handle_t* h = ecs_get(ecs, entity, comp);

    printf("  [on_remove] entity %lu releasing handle '%s' (fd=%d)\n",
           entity.id, h->name, h->fd);
}

// Reacts to ecs_set. Registered deferred (sync = false), so it fires on
// ecs_dispatch rather than inline with ecs_set.
void pos_on_set(ecs_t* ecs,
                ecs_entity_t entity,
                ecs_comp_t comp,
                void* udata)
{
    (void)udata;

    pos_t* p = ecs_get(ecs, entity, comp);

    printf("  [on_set]    entity %lu position is now (%.0f, %.0f)\n",
           entity.id, p->x, p->y);
}

// ---------------------------------------------------------------------------
// System callbacks
// ---------------------------------------------------------------------------

// An entity joins a system the moment it satisfies the system's requirements.
// Deferred (sync = false): fires on ecs_dispatch.
void move_on_join(ecs_t* ecs, ecs_entity_t entity, void* udata)
{
    (void)ecs;
    (void)udata;
    printf("  [on_join]   entity %lu joined MoveSystem\n", entity.id);
}

// An entity leaves when it stops matching the system or is destroyed.
void move_on_leave(ecs_t* ecs, ecs_entity_t entity, void* udata)
{
    (void)ecs;
    (void)udata;
    printf("  [on_leave]  entity %lu left MoveSystem\n", entity.id);
}

ecs_ret_t move_update(ecs_t* ecs,
                      ecs_entity_t* entities,
                      size_t entity_count,
                      void* udata)
{
    (void)ecs;
    (void)entities;
    (void)udata;

    printf("  [update]    MoveSystem ran over %zu entit%s\n",
           entity_count, entity_count == 1 ? "y" : "ies");
    return 0;
}

int main()
{
    ecs_t* ecs = ecs_new(1024, NULL);

    // PosComp has no constructor args; HandleComp forwards a handle_t as args.
    PosComp = ecs_define_component(ecs, sizeof(pos_t), NULL);

    HandleComp = ecs_define_component(ecs, sizeof(handle_t), &(ecs_comp_desc_t)
    {
        .args_size = sizeof(handle_t)
    });

    MoveSystem = ecs_define_system(ecs, move_update, NULL);
    ecs_require(ecs, MoveSystem, PosComp);

    // Register the lifecycle callbacks.
    ecs_on_add   (ecs, HandleComp, handle_on_add,    true);   // constructor
    ecs_on_remove(ecs, HandleComp, handle_on_remove, true);   // destructor
    ecs_on_set   (ecs, PosComp,    pos_on_set,       false);  // deferred
    ecs_on_join  (ecs, MoveSystem, move_on_join,     false);  // deferred
    ecs_on_leave (ecs, MoveSystem, move_on_leave,    false);  // deferred

    // -----------------------------------------------------------------------
    printf("--- ecs_add: synchronous on_add (constructor) ----------------\n");
    ecs_entity_t entity = ecs_create(ecs);

    handle_t res = { .fd = 7, .name = "socket" };
    ecs_add(ecs, entity, HandleComp, &res);   // handle_on_add runs right here

    // -----------------------------------------------------------------------
    printf("--- ecs_add + ecs_set: deferred join/set ---------------------\n");
    // Adding PosComp makes the entity match MoveSystem, so it joins. The join
    // callback is deferred, so nothing prints until ecs_dispatch below.
    ecs_add(ecs, entity, PosComp, NULL);

    pos_t p = { .x = 3.0f, .y = 4.0f };
    ecs_set(ecs, entity, PosComp, &p);        // pos_on_set is deferred too

    printf("  ...join/set callbacks are queued; calling ecs_dispatch:\n");
    ecs_dispatch(ecs);                   // on_join, then on_set, fire here

    // -----------------------------------------------------------------------
    printf("--- ecs_run_system -------------------------------------------\n");
    ecs_run_system(ecs, MoveSystem, 0);

    // -----------------------------------------------------------------------
    printf("--- ecs_destroy: synchronous on_remove + deferred on_leave ---\n");
    // Destroying the entity removes its components and pulls it out of every
    // system. The synchronous on_remove (destructor) runs immediately; the
    // deferred on_leave waits for ecs_dispatch.
    ecs_destroy(ecs, entity);

    printf("  ...leave callback is queued; calling ecs_dispatch:\n");
    ecs_dispatch(ecs);                   // move_on_leave fires here

    printf("--------------------------------------------------------------\n");

    ecs_free(ecs);
    return 0;
}
