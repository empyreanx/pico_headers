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

/*
 * Event-driven ECS example
 * ------------------------
 *
 * This example wires pico_emitter.h into pico_ecs.h to show how gameplay
 * logic can be decomposed into successively finer layers of event handlers.
 * An ECS *system* is good at answering broad, data-parallel questions ("which
 * pairs of colliders overlap?"), but it is a poor place to bury the dozens of
 * special-case rules that a collision can trigger. Instead, the system emits
 * one coarse event and lets a cascade of listeners refine it.
 *
 * Each frame is split into three phases, which is how a real game loop tends
 * to be organized: advance the simulation, resolve the events it produced,
 * then present the result.
 *
 *   1. simulation   movement_system   integrate position from velocity
 *                   collision_system  detect overlaps -> ENQUEUE EVT_COLLISION
 *   2. resolve      queued_emitter_flush  drain the frame's events
 *   3. present      report_system     print entity state
 *
 * The emitter is a *queued* emitter, which is the realistic choice: a system
 * should not have listeners firing in the middle of its own iteration, nor
 * recursing onto its call stack. collision_system therefore only *enqueues*
 * EVT_COLLISION; nothing is dispatched until queued_emitter_flush is called.
 * Flush drains in FIFO waves -- the EVT_COLLISION events first, then the
 * EVT_DAMAGE / EVT_PICKUP they spawn, then EVT_DEATH / EVT_SCORE_CHANGED, and
 * so on -- so the cascade still happens, but at one predictable drain point.
 * Payloads are copied into the emitter's arena, so it is safe to enqueue a
 * stack-local event struct (see the queued_emitter_enqueue_typed calls below).
 *
 * The cascade refines one coarse collision into progressively finer events:
 *
 *   Layer 0  collision_system  (geometry only) -----> EVT_COLLISION
 *   Layer 1  on_collision      (what kind of hit?) -> EVT_DAMAGE / EVT_PICKUP
 *   Layer 2  on_damage         (apply to entity) ----> EVT_DEATH
 *            on_pickup         (consume item) -------> EVT_SCORE_CHANGED
 *   Layer 3  on_death          (final bookkeeping) --> EVT_SCORE_CHANGED
 *            on_score          (terminal)
 *
 * Each layer knows a little more about the game than the one above it, and
 * each only depends on the event it subscribes to -- not on its callers. The
 * movement and report systems are entirely unaware of the emitter; the
 * collision system never mentions health, items, or score; the score handler
 * never mentions collisions. New rules can be slotted in by subscribing to an
 * existing event, without editing the layers above.
 */

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#define PICO_EMITTER_IMPLEMENTATION
#include "../pico_emitter.h"

#include <stdio.h>

/* --------------------------------------------------------------------------
 * Event IDs, ordered from coarsest (top) to finest (bottom)
 * -------------------------------------------------------------------------- */

typedef enum
{
    EVT_COLLISION,      // two colliders overlapped (no gameplay meaning yet)
    EVT_DAMAGE,         // an entity should lose health
    EVT_PICKUP,         // a player collected an item
    EVT_DEATH,          // an entity's health reached zero
    EVT_SCORE_CHANGED,  // the running score changed
    EVT_COUNT
} game_event_t;

/* Event payloads. Each is forwarded by-pointer to its listeners. */

typedef struct { ecs_entity_t a, b;          } collision_evt_t;
typedef struct { ecs_entity_t target; int amount; } damage_evt_t;
typedef struct { ecs_entity_t player, item;  } pickup_evt_t;
typedef struct { ecs_entity_t entity;        } death_evt_t;
typedef struct { int delta;                  } score_evt_t;

/* --------------------------------------------------------------------------
 * Components
 * -------------------------------------------------------------------------- */

typedef enum { KIND_PLAYER, KIND_ENEMY, KIND_ITEM } kind_t;

typedef struct { float x, y;   } pos_t;
typedef struct { float vx, vy; } vel_t;
typedef struct { float radius; } collider_t;
typedef struct { int hp;       } health_t;

typedef struct
{
    kind_t kind;
    int    power; // damage dealt on contact (enemies/players)
    int    value; // score awarded when collected (items) or killed (enemies)
} tag_t;

ecs_comp_t PosComp;
ecs_comp_t VelComp;
ecs_comp_t ColliderComp;
ecs_comp_t HealthComp;
ecs_comp_t TagComp;

ecs_system_t MovementSystem;
ecs_system_t CollisionSystem;
ecs_system_t ReportSystem;

/* --------------------------------------------------------------------------
 * Shared context handed to every system and listener as user data
 * -------------------------------------------------------------------------- */

typedef struct
{
    ecs_t*            ecs;
    queued_emitter_t* qe;
    float             dt;
    int               score;
} game_t;

static const char* kind_name(kind_t k)
{
    switch (k)
    {
        case KIND_PLAYER: return "player";
        case KIND_ENEMY:  return "enemy";
        case KIND_ITEM:   return "item";
        default:          return "?";
    }
}

/* --------------------------------------------------------------------------
 * Movement system: integrate position from velocity
 *
 * A plain data-parallel system with no events at all. It is what makes the
 * collisions below emergent -- entities drift together over successive frames
 * instead of being placed in contact up front.
 * -------------------------------------------------------------------------- */

ecs_ret_t movement_system(ecs_t* ecs,
                          ecs_entity_t* entities,
                          size_t entity_count,
                          void* udata)
{
    game_t* game = (game_t*)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        pos_t* p = ecs_get(ecs, entities[i], PosComp);
        vel_t* v = ecs_get(ecs, entities[i], VelComp);

        p->x += v->vx * game->dt;
        p->y += v->vy * game->dt;
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Layer 0: the ECS collision system
 *
 * Pure geometry. It walks every pair of entities that have a position and a
 * collider and, for each overlapping pair, emits a single coarse EVT_COLLISION.
 * It deliberately knows nothing about health, items, or score.
 * -------------------------------------------------------------------------- */

ecs_ret_t collision_system(ecs_t* ecs,
                           ecs_entity_t* entities,
                           size_t entity_count,
                           void* udata)
{
    game_t* game = (game_t*)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        for (size_t j = i + 1; j < entity_count; j++)
        {
            ecs_entity_t ea = entities[i];
            ecs_entity_t eb = entities[j];

            // A listener earlier in this frame may have scheduled one of these
            // entities for destruction; skip stale entities.
            if (!ecs_is_ready(ecs, ea) || !ecs_is_ready(ecs, eb))
                continue;

            pos_t*      pa = ecs_get(ecs, ea, PosComp);
            pos_t*      pb = ecs_get(ecs, eb, PosComp);
            collider_t* ca = ecs_get(ecs, ea, ColliderComp);
            collider_t* cb = ecs_get(ecs, eb, ColliderComp);

            float dx = pa->x - pb->x;
            float dy = pa->y - pb->y;
            float r  = ca->radius + cb->radius;

            if (dx * dx + dy * dy <= r * r)
            {
                collision_evt_t ev = { ea, eb };
                queued_emitter_enqueue_typed(game->qe, EVT_COLLISION, &ev);
            }
        }
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Layer 1: interpret a raw collision as gameplay intent
 *
 * Classifies the pair by kind and re-emits a more specific event. This is the
 * only handler that has to reason about pairs; everything below it deals with
 * a single entity at a time.
 * -------------------------------------------------------------------------- */

// Resolve the pair into (player/attacker, other) regardless of ordering.
static bool find_kind(game_t* g, collision_evt_t ev, kind_t want,
                      ecs_entity_t* match, ecs_entity_t* other)
{
    tag_t* ta = ecs_get(g->ecs, ev.a, TagComp);
    tag_t* tb = ecs_get(g->ecs, ev.b, TagComp);

    if (ta->kind == want) { *match = ev.a; *other = ev.b; return true; }
    if (tb->kind == want) { *match = ev.b; *other = ev.a; return true; }
    return false;
}

static void on_collision(const void* data, void* udata)
{
    const collision_evt_t* ev = (const collision_evt_t*)data;
    game_t* game = (game_t*)udata;

    if (!ecs_is_ready(game->ecs, ev->a) || !ecs_is_ready(game->ecs, ev->b))
        return;

    ecs_entity_t player, other;

    if (find_kind(game, *ev, KIND_PLAYER, &player, &other))
    {
        tag_t* ot = ecs_get(game->ecs, other, TagComp);

        if (ot->kind == KIND_ITEM)
        {
            pickup_evt_t pe = { player, other };
            queued_emitter_enqueue_typed(game->qe, EVT_PICKUP, &pe);
            return;
        }

        if (ot->kind == KIND_ENEMY)
        {
            // A clash hurts both parties: fan one collision out into two
            // damage events, each addressed to a single entity.
            tag_t* pt = ecs_get(game->ecs, player, TagComp);

            damage_evt_t to_enemy  = { other,  pt->power };
            damage_evt_t to_player = { player, ot->power };

            queued_emitter_enqueue_typed(game->qe, EVT_DAMAGE, &to_enemy);
            queued_emitter_enqueue_typed(game->qe, EVT_DAMAGE, &to_player);
        }
    }
}

/* --------------------------------------------------------------------------
 * Layer 2: apply effects to a single entity
 * -------------------------------------------------------------------------- */

static void on_damage(const void* data, void* udata)
{
    const damage_evt_t* ev = (const damage_evt_t*)data;
    game_t* game = (game_t*)udata;

    if (!ecs_is_ready(game->ecs, ev->target))
        return;

    health_t* h = ecs_get(game->ecs, ev->target, HealthComp);
    tag_t*    t = ecs_get(game->ecs, ev->target, TagComp);

    h->hp -= ev->amount;

    printf("    [damage] %s takes %d  (hp: %d)\n",
           kind_name(t->kind), ev->amount, h->hp);

    if (h->hp <= 0)
    {
        death_evt_t de = { ev->target };
        queued_emitter_enqueue_typed(game->qe, EVT_DEATH, &de);
    }
}

static void on_pickup(const void* data, void* udata)
{
    const pickup_evt_t* ev = (const pickup_evt_t*)data;
    game_t* game = (game_t*)udata;

    if (!ecs_is_ready(game->ecs, ev->item))
        return;

    tag_t* t = ecs_get(game->ecs, ev->item, TagComp);
    int    value = t->value;

    printf("    [pickup] player collects item worth %d\n", value);

    // Deferred while a system is running; the entity is marked not-ready now,
    // so the guards above keep other handlers from acting on it this frame.
    ecs_destroy(game->ecs, ev->item);

    score_evt_t se = { value };
    queued_emitter_enqueue_typed(game->qe, EVT_SCORE_CHANGED, &se);
}

/* --------------------------------------------------------------------------
 * Layer 3: terminal bookkeeping
 * -------------------------------------------------------------------------- */

static void on_death(const void* data, void* udata)
{
    const death_evt_t* ev = (const death_evt_t*)data;
    game_t* game = (game_t*)udata;

    if (!ecs_is_ready(game->ecs, ev->entity))
        return;

    tag_t* t = ecs_get(game->ecs, ev->entity, TagComp);

    printf("    [death] %s dies\n", kind_name(t->kind));

    // Killing an enemy is worth points; refine the death into a score change.
    if (t->kind == KIND_ENEMY)
    {
        score_evt_t se = { t->value };
        queued_emitter_enqueue_typed(game->qe, EVT_SCORE_CHANGED, &se);
    }

    ecs_destroy(game->ecs, ev->entity);
}

static void on_score(const void* data, void* udata)
{
    const score_evt_t* ev = (const score_evt_t*)data;
    game_t* game = (game_t*)udata;

    game->score += ev->delta;

    printf("    [score] +%d  (total: %d)\n", ev->delta, game->score);
}

/* --------------------------------------------------------------------------
 * Report system: print the surviving entities at the end of the frame
 *
 * Runs last so it sees the state left behind once the collision cascade and
 * the deferred destroys for the frame have been applied.
 * -------------------------------------------------------------------------- */

ecs_ret_t report_system(ecs_t* ecs,
                        ecs_entity_t* entities,
                        size_t entity_count,
                        void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        pos_t*    p = ecs_get(ecs, entities[i], PosComp);
        health_t* h = ecs_get(ecs, entities[i], HealthComp);
        tag_t*    t = ecs_get(ecs, entities[i], TagComp);

        printf("    [report] %-6s @ (%.1f, %.1f)  hp: %d\n",
               kind_name(t->kind), p->x, p->y, h->hp);
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Setup
 * -------------------------------------------------------------------------- */

static void register_components(ecs_t* ecs)
{
    PosComp      = ecs_define_component(ecs, sizeof(pos_t),      NULL);
    VelComp      = ecs_define_component(ecs, sizeof(vel_t),      NULL);
    ColliderComp = ecs_define_component(ecs, sizeof(collider_t), NULL);
    HealthComp   = ecs_define_component(ecs, sizeof(health_t),   NULL);
    TagComp      = ecs_define_component(ecs, sizeof(tag_t),      NULL);
}

static void register_systems(ecs_t* ecs, game_t* game)
{
    ecs_sys_desc_t desc = { .udata = game };

    // ecs_run_systems dispatches systems in definition order, so the order of
    // these calls is the per-frame pipeline: move, then collide, then report.
    MovementSystem = ecs_define_system(ecs, movement_system, &desc);
    ecs_require(ecs, MovementSystem, PosComp);
    ecs_require(ecs, MovementSystem, VelComp);

    CollisionSystem = ecs_define_system(ecs, collision_system, &desc);
    ecs_require(ecs, CollisionSystem, PosComp);
    ecs_require(ecs, CollisionSystem, ColliderComp);

    ReportSystem = ecs_define_system(ecs, report_system, &desc);
    ecs_require(ecs, ReportSystem, PosComp);
    ecs_require(ecs, ReportSystem, TagComp);
}

static void register_listeners(queued_emitter_t* qe, game_t* game)
{
    // Each subscription connects one layer to the next. The cascade is built
    // entirely out of these edges; no layer calls another directly.
    queued_emitter_on(qe, EVT_COLLISION,     on_collision, game);
    queued_emitter_on(qe, EVT_DAMAGE,        on_damage,    game);
    queued_emitter_on(qe, EVT_PICKUP,        on_pickup,    game);
    queued_emitter_on(qe, EVT_DEATH,         on_death,     game);
    queued_emitter_on(qe, EVT_SCORE_CHANGED, on_score,     game);
}

static ecs_entity_t spawn(ecs_t* ecs, kind_t kind, float x, float y,
                          float vx, float vy, float radius,
                          int hp, int power, int value)
{
    ecs_entity_t e = ecs_create(ecs);

    pos_t      p = { x, y };
    vel_t      v = { vx, vy };
    collider_t c = { radius };
    health_t   h = { hp };
    tag_t      t = { kind, power, value };

    // ecs_set adds the component if absent and copies the value in. (ecs_add's
    // "args" are forwarded to an optional constructor, not stored directly.)
    ecs_set(ecs, e, PosComp,      &p);
    ecs_set(ecs, e, VelComp,      &v);
    ecs_set(ecs, e, ColliderComp, &c);
    ecs_set(ecs, e, HealthComp,   &h);
    ecs_set(ecs, e, TagComp,      &t);

    return e;
}

int main(void)
{
    ecs_t*            ecs = ecs_new(1024, NULL);
    queued_emitter_t* qe  = queued_emitter_create(EVT_COUNT);

    game_t game = { ecs, qe, 1.0f, 0 };

    register_components(ecs);
    register_systems(ecs, &game);
    register_listeners(qe, &game);

    // The player drifts right into a stationary item, then into an enemy that
    // is closing from the right. A second enemy sits far away and never joins.
    //                 kind          x      y     vx     vy   rad  hp  power value
    spawn(ecs, KIND_PLAYER,   0.0f,  0.0f,  1.0f,  0.0f, 1.0f, 100,  20,    0);
    spawn(ecs, KIND_ITEM,     3.0f,  0.0f,  0.0f,  0.0f, 1.0f,   1,   0,  100);
    spawn(ecs, KIND_ENEMY,    8.0f,  0.0f, -2.0f,  0.0f, 1.0f,  30,  10,   50);
    spawn(ecs, KIND_ENEMY,   50.0f, 50.0f,  0.0f,  0.0f, 1.0f,  30,  10,   50); // out of range

    for (int frame = 1; frame <= 4; frame++)
    {
        printf("---------------------------------------------------------------\n");
        printf("Frame %d\n", frame);
        printf("---------------------------------------------------------------\n");

        // Phase 1 - simulation: movement_system integrates positions and
        // collision_system enqueues EVT_COLLISION events. No listener has run
        // yet; the queue just fills up.
        ecs_run_system(ecs, MovementSystem,  0);
        ecs_run_system(ecs, CollisionSystem, 0);

        // Phase 2 - resolve: drain the frame's events. The whole cascade runs
        // here, in FIFO waves, off the systems' call stacks.
        queued_emitter_flush(qe);

        // Phase 3 - present: report the state the cascade left behind.
        ecs_run_system(ecs, ReportSystem, 0);
    }

    printf("---------------------------------------------------------------\n");
    printf("Final score: %d\n", game.score);
    printf("---------------------------------------------------------------\n");

    queued_emitter_destroy(qe);
    ecs_free(ecs);

    return 0;
}
