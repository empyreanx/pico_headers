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
 * Passing messages: events vs. temporary components
 * --------------------------------------------------
 *
 * Companion to events.c. That example resolved a collision entirely inside the
 * emitter -- a cascade of listener callbacks. This one is about the two ways
 * decoupled *systems* hand work to each other, and when to reach for each:
 *
 *   1. EVENTS (the queued emitter) are broadcast notifications. "This happened."
 *      Nobody is addressed; zero or more listeners react. Nothing survives the
 *      flush. Use them for fire-and-forget facts -- a death occurred, the score
 *      changed -- where the sender does not care who (if anyone) is listening.
 *
 *   2. TEMPORARY COMPONENTS are directed messages. A message component is
 *      attached to an entity, sits in the store as data, and is later drained
 *      and *removed* by a consuming system on its own scheduled pass. Use them
 *      when a specific system must process the message as part of its normal
 *      data-parallel iteration -- i.e. when the "handler" is a system, not a
 *      callback. The component is the envelope; its lifecycle is the message:
 *
 *          POST     ecs_set(e, Msg, &m)     attach the envelope (add if absent)
 *          DELIVER  system requires Msg     e enters the system only if posted
 *          CONSUME  ecs_get(e, Msg) ...     read it
 *          CLEAR    ecs_remove(e, Msg)      envelope gone; message delivered once
 *
 * Why both? The emitter dispatches to callbacks (emitter_listener_fn), never to
 * systems (ecs_system_fn) -- a system cannot "subscribe". So an event and a
 * temporary component are complementary: a thin *bridge* listener catches a
 * broadcast event and turns it into a directed message component that the right
 * system will drain next pass. The event is the bus; the component is the inbox.
 *
 *     collision_system --emit EVT_CONTACT--> on_contact (bridge)
 *                                                | posts message components
 *                                                v
 *                        DamageMsg  --> damage_system   (drains + removes it)
 *                        PickupMsg  --> pickup_system   (drains + removes it)
 *
 *     damage_system    --emit EVT_DEATH----> on_death (bridge)
 *                                                | emit EVT_SCORE  (broadcast)
 *                                                | post LootMsg    (directed)
 *                                                v
 *                        LootMsg    --> loot_system     (drains + removes it)
 *
 * This example shows three flavors of "temporary component as a message":
 *
 *   - DamageMsg  a message to an entity that already exists. Posted onto the
 *                combatant, drained by damage_system, then removed. Two hits in
 *                one frame accumulate into the one envelope before it is drained.
 *   - PickupMsg  a message whose consumer destroys the carrier. pickup_system
 *                deletes the item outright, so the envelope dies with it -- no
 *                explicit remove needed.
 *   - LootMsg    a message about nothing that exists yet. on_death has no entity
 *                to attach it to (the dead one is about to vanish), so it spawns
 *                a throwaway *carrier* entity holding only the message, which
 *                loot_system consumes and then destroys.
 *
 * Timing is controlled by where the queue is flushed. Each frame is staged so
 * that a consumer placed after a flush sees this frame's messages:
 *
 *   movement_system
 *   collision_system        emit EVT_CONTACT
 *   queued_emitter_flush    <-- barrier: bridge posts DamageMsg / PickupMsg
 *   damage_system           drain DamageMsg (may emit EVT_DEATH)
 *   pickup_system           drain PickupMsg (emits EVT_SCORE)
 *   queued_emitter_flush    <-- barrier: deaths post LootMsg, score resolves
 *   loot_system             drain LootMsg
 *   report_system           present
 */

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#define PICO_EMITTER_IMPLEMENTATION
#include "../pico_emitter.h"

#include <stdio.h>

/* --------------------------------------------------------------------------
 * Events: broadcast notifications carried on the bus.
 *
 * These are not addressed to anyone. They exist only between an enqueue and the
 * next flush; nothing about them persists in the ECS store.
 * -------------------------------------------------------------------------- */

typedef enum
{
    EVT_CONTACT,   // collision_system -> on_contact bridge
    EVT_DEATH,     // damage_system    -> on_death bridge
    EVT_SCORE,     // any system       -> on_score (updates global state)
    EVT_COUNT
} game_event_t;

typedef struct { ecs_entity_t a, b;  } contact_evt_t;
typedef struct { ecs_entity_t entity; } death_evt_t;
typedef struct { int delta;          } score_evt_t;

/* --------------------------------------------------------------------------
 * Components
 * -------------------------------------------------------------------------- */

typedef enum { KIND_PLAYER, KIND_ENEMY, KIND_ITEM } kind_t;

/* State components: the persistent makeup of an entity. */
typedef struct { float x, y;   } pos_t;
typedef struct { float vx, vy; } vel_t;
typedef struct { float radius; } collider_t;
typedef struct { int hp;       } health_t;
typedef struct { kind_t kind; int power; int value; } tag_t;

/* Message components: temporary envelopes. A bridge posts one; a system drains
 * and removes it. An entity only carries one while a message is in flight. */
typedef struct { int amount;             } damage_msg_t;
typedef struct { ecs_entity_t collector; } pickup_msg_t;
typedef struct { float x, y; int value;  } loot_msg_t;

ecs_comp_t PosComp;
ecs_comp_t VelComp;
ecs_comp_t ColliderComp;
ecs_comp_t HealthComp;
ecs_comp_t TagComp;
ecs_comp_t DamageMsgComp;
ecs_comp_t PickupMsgComp;
ecs_comp_t LootMsgComp;

ecs_system_t MovementSystem;
ecs_system_t CollisionSystem;
ecs_system_t DamageSystem;
ecs_system_t PickupSystem;
ecs_system_t LootSystem;
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

// Defined in the setup section; forward-declared so loot_system can spawn.
static ecs_entity_t spawn(ecs_t* ecs, kind_t kind, float x, float y,
                          float vx, float vy, float radius,
                          int hp, int power, int value);

/* --------------------------------------------------------------------------
 * Producer system: movement (no messages, just state)
 * -------------------------------------------------------------------------- */

ecs_ret_t movement_system(ecs_t* ecs, ecs_entity_t* entities,
                          size_t entity_count, void* udata)
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
 * Producer system: collision
 *
 * Detects overlaps and broadcasts them as EVT_CONTACT. It does not know what a
 * contact means -- it never touches health, items, or score, and never names
 * damage_system or pickup_system. An event is the right tool here: a plain
 * notification with no addressee.
 * -------------------------------------------------------------------------- */

ecs_ret_t collision_system(ecs_t* ecs, ecs_entity_t* entities,
                           size_t entity_count, void* udata)
{
    game_t* game = (game_t*)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        for (size_t j = i + 1; j < entity_count; j++)
        {
            ecs_entity_t ea = entities[i];
            ecs_entity_t eb = entities[j];

            pos_t*      pa = ecs_get(ecs, ea, PosComp);
            pos_t*      pb = ecs_get(ecs, eb, PosComp);
            collider_t* ca = ecs_get(ecs, ea, ColliderComp);
            collider_t* cb = ecs_get(ecs, eb, ColliderComp);

            float dx = pa->x - pb->x;
            float dy = pa->y - pb->y;
            float r  = ca->radius + cb->radius;

            if (dx * dx + dy * dy <= r * r)
            {
                contact_evt_t ev = { ea, eb };
                queued_emitter_enqueue_typed(game->qe, EVT_CONTACT, &ev);
            }
        }
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Bridge listeners: turn broadcast events into directed messages
 *
 * These run during flush, off any system's call stack. A bridge's only job is
 * to translate an event into either a message component (for a system to drain)
 * or another event (a pure broadcast, like the score change).
 * -------------------------------------------------------------------------- */

// Resolve a pair into (entity of the requested kind, the other) either way round.
static bool find_kind(game_t* g, contact_evt_t ev, kind_t want,
                      ecs_entity_t* match, ecs_entity_t* other)
{
    tag_t* ta = ecs_get(g->ecs, ev.a, TagComp);
    tag_t* tb = ecs_get(g->ecs, ev.b, TagComp);

    if (ta->kind == want) { *match = ev.a; *other = ev.b; return true; }
    if (tb->kind == want) { *match = ev.b; *other = ev.a; return true; }
    return false;
}

// POST a DamageMsg. The envelope accumulates: two hits in one frame stack into
// one component rather than overwriting, since the consumer only drains once.
static void post_damage(game_t* g, ecs_entity_t e, int amount)
{
    if (ecs_has(g->ecs, e, DamageMsgComp))
    {
        damage_msg_t* dm = ecs_get(g->ecs, e, DamageMsgComp);
        dm->amount += amount;
    }
    else
    {
        damage_msg_t dm = { amount };
        ecs_set(g->ecs, e, DamageMsgComp, &dm);
    }
}

static void on_contact(const void* data, void* udata)
{
    const contact_evt_t* ev = (const contact_evt_t*)data;
    game_t* game = (game_t*)udata;

    if (!ecs_is_ready(game->ecs, ev->a) || !ecs_is_ready(game->ecs, ev->b))
        return;

    ecs_entity_t player, other;

    if (!find_kind(game, *ev, KIND_PLAYER, &player, &other))
        return;

    tag_t* ot = ecs_get(game->ecs, other, TagComp);

    if (ot->kind == KIND_ITEM)
    {
        // POST a PickupMsg onto the item, addressed to its collector.
        pickup_msg_t pm = { player };
        ecs_set(game->ecs, other, PickupMsgComp, &pm);
        printf("    [bridge] contact -> PickupMsg on item\n");
    }
    else if (ot->kind == KIND_ENEMY)
    {
        // POST a DamageMsg to each combatant.
        tag_t* pt = ecs_get(game->ecs, player, TagComp);
        post_damage(game, other,  pt->power);
        post_damage(game, player, ot->power);
        printf("    [bridge] contact -> DamageMsg on enemy(%d) and player(%d)\n",
               pt->power, ot->power);
    }
}

static void on_death(const void* data, void* udata)
{
    const death_evt_t* ev = (const death_evt_t*)data;
    game_t* game = (game_t*)udata;

    if (!ecs_is_ready(game->ecs, ev->entity))
        return;

    tag_t* t = ecs_get(game->ecs, ev->entity, TagComp);
    pos_t* p = ecs_get(game->ecs, ev->entity, PosComp);

    printf("    [bridge] death of %s\n", kind_name(t->kind));

    if (t->kind == KIND_ENEMY)
    {
        // Broadcast the points (fire-and-forget; on_score just tallies it).
        score_evt_t se = { t->value };
        queued_emitter_enqueue_typed(game->qe, EVT_SCORE, &se);

        // POST a LootMsg. There is no existing entity to address -- the dying
        // enemy is about to be destroyed -- so the message rides on a throwaway
        // carrier entity that holds nothing but the envelope. loot_system will
        // consume it and destroy the carrier.
        ecs_entity_t carrier = ecs_create(game->ecs);
        loot_msg_t   lm      = { p->x, p->y, t->value / 2 };
        ecs_set(game->ecs, carrier, LootMsgComp, &lm);
        printf("    [bridge] death -> LootMsg (loot at %.1f, %.1f)\n", p->x, p->y);
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
 * Consumer system: damage
 *
 * DELIVER: it requires DamageMsg, so an entity only enters this system once a
 * bridge has posted one. It CONSUMES the message, then CLEARS it with
 * ecs_remove so the entity leaves the system again -- the envelope is delivered
 * exactly once. It never learns collision_system exists. When a hit is fatal it
 * turns producer and broadcasts EVT_DEATH.
 * -------------------------------------------------------------------------- */

ecs_ret_t damage_system(ecs_t* ecs, ecs_entity_t* entities,
                        size_t entity_count, void* udata)
{
    game_t* game = (game_t*)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_entity_t  e  = entities[i];
        health_t*     h  = ecs_get(ecs, e, HealthComp);
        damage_msg_t* dm = ecs_get(ecs, e, DamageMsgComp);
        tag_t*        t  = ecs_get(ecs, e, TagComp);

        h->hp -= dm->amount;
        printf("    [damage-sys] %s takes %d  (hp: %d)\n",
               kind_name(t->kind), dm->amount, h->hp);

        ecs_remove(ecs, e, DamageMsgComp); // CLEAR: message delivered

        if (h->hp <= 0)
        {
            death_evt_t de = { e };
            queued_emitter_enqueue_typed(game->qe, EVT_DEATH, &de);
        }
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Consumer system: pickup
 *
 * Drains PickupMsg and broadcasts the item's value via EVT_SCORE. Here the
 * consumer destroys the item outright, so the envelope dies with its carrier --
 * no explicit ecs_remove is needed. Again, no reference to collision_system.
 * -------------------------------------------------------------------------- */

ecs_ret_t pickup_system(ecs_t* ecs, ecs_entity_t* entities,
                        size_t entity_count, void* udata)
{
    game_t* game = (game_t*)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_entity_t item = entities[i];
        tag_t* t = ecs_get(ecs, item, TagComp);

        printf("    [pickup-sys] item worth %d collected\n", t->value);

        score_evt_t se = { t->value };
        queued_emitter_enqueue_typed(game->qe, EVT_SCORE, &se);

        ecs_destroy(ecs, item); // envelope goes with the entity
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Consumer system: loot
 *
 * Drains the LootMsg envelopes riding on throwaway carrier entities -- one per
 * enemy that died last barrier -- materializes a real loot item in the world,
 * and then destroys the spent carrier. The loot it drops re-enters the pipeline:
 * a later frame's collision_system can have the player walk into it.
 * -------------------------------------------------------------------------- */

ecs_ret_t loot_system(ecs_t* ecs, ecs_entity_t* entities,
                      size_t entity_count, void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_entity_t carrier = entities[i];
        loot_msg_t*  lm      = ecs_get(ecs, carrier, LootMsgComp);

        spawn(ecs, KIND_ITEM, lm->x, lm->y, 0.0f, 0.0f, 1.0f, 1, 0, lm->value);
        printf("    [loot-sys] dropped item worth %d at (%.1f, %.1f)\n",
               lm->value, lm->x, lm->y);

        ecs_destroy(ecs, carrier); // discard the spent carrier + its envelope
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Present system: report
 * -------------------------------------------------------------------------- */

ecs_ret_t report_system(ecs_t* ecs, ecs_entity_t* entities,
                        size_t entity_count, void* udata)
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
    PosComp       = ecs_define_component(ecs, sizeof(pos_t),        NULL);
    VelComp       = ecs_define_component(ecs, sizeof(vel_t),        NULL);
    ColliderComp  = ecs_define_component(ecs, sizeof(collider_t),   NULL);
    HealthComp    = ecs_define_component(ecs, sizeof(health_t),     NULL);
    TagComp       = ecs_define_component(ecs, sizeof(tag_t),        NULL);
    DamageMsgComp = ecs_define_component(ecs, sizeof(damage_msg_t), NULL);
    PickupMsgComp = ecs_define_component(ecs, sizeof(pickup_msg_t), NULL);
    LootMsgComp   = ecs_define_component(ecs, sizeof(loot_msg_t),   NULL);
}

static void register_systems(ecs_t* ecs, game_t* game)
{
    ecs_sys_desc_t desc = { .udata = game };

    MovementSystem = ecs_define_system(ecs, movement_system, &desc);
    ecs_require(ecs, MovementSystem, PosComp);
    ecs_require(ecs, MovementSystem, VelComp);

    CollisionSystem = ecs_define_system(ecs, collision_system, &desc);
    ecs_require(ecs, CollisionSystem, PosComp);
    ecs_require(ecs, CollisionSystem, ColliderComp);

    // Consumers require their message component, so an entity only enters the
    // system once a bridge has posted an envelope to it.
    DamageSystem = ecs_define_system(ecs, damage_system, &desc);
    ecs_require(ecs, DamageSystem, HealthComp);
    ecs_require(ecs, DamageSystem, DamageMsgComp);

    PickupSystem = ecs_define_system(ecs, pickup_system, &desc);
    ecs_require(ecs, PickupSystem, TagComp);
    ecs_require(ecs, PickupSystem, PickupMsgComp);

    LootSystem = ecs_define_system(ecs, loot_system, &desc);
    ecs_require(ecs, LootSystem, LootMsgComp);

    ReportSystem = ecs_define_system(ecs, report_system, &desc);
    ecs_require(ecs, ReportSystem, PosComp);
    ecs_require(ecs, ReportSystem, TagComp);
}

static void register_listeners(queued_emitter_t* qe, game_t* game)
{
    queued_emitter_on(qe, EVT_CONTACT, on_contact, game);
    queued_emitter_on(qe, EVT_DEATH,   on_death,   game);
    queued_emitter_on(qe, EVT_SCORE,   on_score,   game);
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

        // Produce: advance state and broadcast contacts.
        ecs_run_system(ecs, MovementSystem,  0);
        ecs_run_system(ecs, CollisionSystem, 0);

        // Barrier: bridge turns EVT_CONTACT into DamageMsg / PickupMsg envelopes.
        queued_emitter_flush(qe);

        // Consume: systems drain their envelopes (and may broadcast new events).
        ecs_run_system(ecs, DamageSystem, 0);
        ecs_run_system(ecs, PickupSystem, 0);

        // Barrier: deaths post LootMsg carriers and the score resolves.
        queued_emitter_flush(qe);

        // Consume: drain the LootMsg carriers those deaths produced.
        ecs_run_system(ecs, LootSystem, 0);

        // Present.
        ecs_run_system(ecs, ReportSystem, 0);
    }

    printf("---------------------------------------------------------------\n");
    printf("Final score: %d\n", game.score);
    printf("---------------------------------------------------------------\n");

    queued_emitter_destroy(qe);
    ecs_free(ecs);

    return 0;
}