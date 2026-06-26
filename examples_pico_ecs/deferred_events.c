/*=============================================================================
 * This is free and unencumbered software released into the public domain.
 *
 * For more information, please refer to <http://unlicense.org/>
 *============================================================================*/

// Demonstrates the two *independent* layers of deferral in the ECS, and how
// they compose:
//
//   1. Deferred commands. While a system is running, ecs_add / ecs_remove /
//      ecs_set / ecs_destroy are queued instead of applied. The queue is
//      flushed when the system callback returns (inside ecs_run_system).
//
//   2. Deferred events. The flush above is an ordinary mutation, so it raises
//      the built-in lifecycle events (on_add, on_join, on_remove, on_leave).
//      Registered without sync, those callbacks are enqueued and only fire on
//      ecs_dispatch.
//
// The upshot is a clean, three-stage timeline for work kicked off inside a
// system:
//
//      [A] inside the system body  -> structural changes are still pending
//      [B] ecs_run_system returns  -> commands flushed; world is updated,
//                                      but lifecycle callbacks have NOT run
//      [C] ecs_dispatch            -> the deferred callbacks finally fire
//
// A spawner system creates monsters mid-iteration; we observe the world at
// each stage to make the separation visible.

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <stdio.h>

// ---------------------------------------------------------------------------
// Components
// ---------------------------------------------------------------------------
typedef struct { int hp; } health_t;
typedef struct { int dummy; } spawner_t;   // a tag marking the spawner entity

ecs_comp_t HealthComp;
ecs_comp_t SpawnerComp;

ecs_system_t SpawnSystem;    // runs over spawner entities, creates monsters
ecs_system_t MonsterSystem;  // every entity with HealthComp belongs here

// How many monsters the spawner should request this tick.
#define SPAWN_COUNT 3

// ---------------------------------------------------------------------------
// Counters, bumped only when the deferred callbacks actually run. Watching
// these lets us prove *when* delivery happens.
// ---------------------------------------------------------------------------
int g_on_add_fired  = 0;
int g_on_join_fired = 0;

// ---------------------------------------------------------------------------
// Deferred lifecycle callbacks (registered with sync = false).
// ---------------------------------------------------------------------------
void health_on_add(ecs_t* ecs,
                   ecs_entity_t entity,
                   ecs_comp_t comp,
                   const void* args,
                   void* udata)
{
    (void)args;
    (void)udata;

    // The component already exists by the time any on_add runs; give it a
    // starting value here, the way a constructor would.
    health_t* health = ecs_get(ecs, entity, comp);
    health->hp = 100;

    g_on_add_fired++;
    printf("      [on_add]   entity %lu HealthComp constructed (hp=%d)\n",
           entity.id, health->hp);
}

void monster_on_join(ecs_t* ecs, ecs_entity_t entity, void* udata)
{
    (void)ecs;
    (void)udata;

    g_on_join_fired++;
    printf("      [on_join]  entity %lu joined MonsterSystem\n", entity.id);
}

// ---------------------------------------------------------------------------
// The spawner system. Each call requests SPAWN_COUNT new monsters. ecs_create
// hands back a live entity immediately, but the ecs_add that gives it a
// HealthComp is deferred until this callback returns.
// ---------------------------------------------------------------------------
ecs_ret_t spawn_update(ecs_t* ecs,
                       ecs_entity_t* entities,
                       size_t entity_count,
                       void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)udata;

    printf("    [stage A] inside spawn_update: requesting %d monsters\n",
           SPAWN_COUNT);

    for (int i = 0; i < SPAWN_COUNT; i++)
    {
        ecs_entity_t monster = ecs_create(ecs);   // immediate
        ecs_add(ecs, monster, HealthComp, NULL);  // DEFERRED (system_active)
    }

    // The adds have not been applied yet, so the new monsters are not members
    // of MonsterSystem at this point in time.
    printf("    [stage A] MonsterSystem membership right now: %zu"
           " (adds still queued)\n",
           ecs_get_entity_count(ecs, MonsterSystem));

    return 0;
}

// MonsterSystem exists only so entities can join it; it does no work. We never
// run it, but a system still needs a (non-NULL) update callback.
ecs_ret_t monster_update(ecs_t* ecs,
                         ecs_entity_t* entities,
                         size_t entity_count,
                         void* udata)
{
    (void)ecs;
    (void)entities;
    (void)entity_count;
    (void)udata;
    return 0;
}

int main()
{
    ecs_t* ecs = ecs_new(1024, NULL);

    HealthComp  = ecs_define_component(ecs, sizeof(health_t),  NULL);
    SpawnerComp = ecs_define_component(ecs, sizeof(spawner_t), NULL);

    SpawnSystem = ecs_define_system(ecs, spawn_update, NULL);
    ecs_require(ecs, SpawnSystem, SpawnerComp);

    // MonsterSystem exists so we can watch entities join it; its update is a
    // no-op and we never run it.
    MonsterSystem = ecs_define_system(ecs, monster_update, NULL);
    ecs_require(ecs, MonsterSystem, HealthComp);

    // Deferred lifecycle callbacks: enqueued, fired on ecs_dispatch.
    ecs_on_add (ecs, HealthComp,   health_on_add,   false);
    ecs_on_join(ecs, MonsterSystem, monster_on_join, false);

    // The single spawner entity that drives SpawnSystem.
    ecs_entity_t spawner = ecs_create(ecs);
    ecs_add(ecs, spawner, SpawnerComp, NULL);

    printf("=== Running the spawner system ===============================\n");
    ecs_run_system(ecs, SpawnSystem, 0);

    // -----------------------------------------------------------------------
    // Stage B: ecs_run_system has returned. The command queue was flushed on
    // the way out, so the monsters now own HealthComp and belong to
    // MonsterSystem. But the flush only *enqueued* the on_add / on_join
    // events: not one callback has run.
    // -----------------------------------------------------------------------
    printf("    [stage B] after ecs_run_system:\n");
    printf("    [stage B]   MonsterSystem membership: %zu (commands flushed)\n",
           ecs_get_entity_count(ecs, MonsterSystem));
    printf("    [stage B]   on_add fired:  %d\n", g_on_add_fired);
    printf("    [stage B]   on_join fired: %d   (events still queued)\n",
           g_on_join_fired);

    // -----------------------------------------------------------------------
    // Stage C: drain the event queue. Now the deferred callbacks run, in the
    // order the flush enqueued them.
    // -----------------------------------------------------------------------
    printf("    [stage C] calling ecs_dispatch:\n");
    ecs_dispatch(ecs);

    printf("    [stage C]   on_add fired:  %d\n", g_on_add_fired);
    printf("    [stage C]   on_join fired: %d\n", g_on_join_fired);

    printf("==============================================================\n");
    printf("Two deferrals, two triggers: ecs_run_system flushed the command\n");
    printf("queue; ecs_dispatch flushed the event queue it produced.\n");

    ecs_free(ecs);
    return 0;
}