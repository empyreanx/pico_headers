/*=============================================================================
 * This is free and unencumbered software released into the public domain.
 *
 * For more information, please refer to <http://unlicense.org/>
 *============================================================================*/

// Demonstrates the publish/subscribe event system that is decoupled from
// components and entities:
//
//   1. Defining a custom event with a payload
//   2. Subscribing listeners and emitting synchronously (ecs_emit)
//   3. Enqueuing events and draining them later (ecs_enqueue + ecs_dispatch)
//   4. Scoping a receiver to a single sender (ecs_emit_from + ecs_subscribe_to)

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <stdio.h>

// ---------------------------------------------------------------------------
// A custom event payload. Events may carry any fixed-size, plain-old-data
// struct (or no payload at all, by passing a size of 0 to ecs_define_event).
// ---------------------------------------------------------------------------
typedef struct
{
    int   amount;
    float x, y;
} damage_t;

// Event handle, filled in by ecs_define_event below.
ecs_event_t DamageEvent;

// ---------------------------------------------------------------------------
// Listeners. Every listener shares the ecs_listener_fn signature: it receives
// the event handle, an opaque payload pointer (cast it back to the type the
// event was defined with), the payload size, and the user data passed at
// subscription time.
// ---------------------------------------------------------------------------
void on_damage(ecs_t* ecs,
               ecs_event_t event,
               const void* payload,
               size_t payload_size,
               void* udata)
{
    (void)ecs;
    (void)event;
    (void)payload_size;
    (void)udata;

    const damage_t* dmg = (const damage_t*)payload;

    printf("  [health]  took %d damage at (%.0f, %.0f)\n",
           dmg->amount, dmg->x, dmg->y);
}

// A second, independent listener for the same event. Both fire for every
// emit; the user data lets one callback serve many subscriptions.
void on_damage_audio(ecs_t* ecs,
                     ecs_event_t event,
                     const void* payload,
                     size_t payload_size,
                     void* udata)
{
    (void)ecs;
    (void)event;
    (void)payload_size;

    const damage_t* dmg = (const damage_t*)payload;
    const char*     tag = (const char*)udata;

    printf("  [audio]   play '%s' (damage = %d)\n", tag, dmg->amount);
}

// A receiver scoped to a single sender via ecs_subscribe_to.
void on_player_damage(ecs_t* ecs,
                      ecs_event_t event,
                      const void* payload,
                      size_t payload_size,
                      void* udata)
{
    (void)ecs;
    (void)event;
    (void)payload_size;
    (void)udata;

    const damage_t* dmg = (const damage_t*)payload;

    printf("  [player]  HUD flash: -%d HP\n", dmg->amount);
}

int main()
{
    ecs_t* ecs = ecs_new(1024, NULL);

    // Declare the event, stating the size of its payload.
    DamageEvent = ecs_define_event(ecs, sizeof(damage_t));

    // -----------------------------------------------------------------------
    // 1. Synchronous delivery with ecs_emit.
    //
    // ecs_emit invokes every matching listener before it returns. The payload
    // is borrowed for the duration of the call, so a stack value is fine.
    // -----------------------------------------------------------------------
    ecs_subscribe(ecs, DamageEvent, on_damage, NULL);
    ecs_subscribe(ecs, DamageEvent, on_damage_audio, "hit.wav");

    printf("--- ecs_emit (synchronous) -----------------------------------\n");
    damage_t hit = { .amount = 12, .x = 4.0f, .y = 9.0f };
    ecs_emit(ecs, DamageEvent, &hit);   // both listeners run right here

    // -----------------------------------------------------------------------
    // 2. Deferred delivery with ecs_enqueue + ecs_dispatch.
    //
    // ecs_enqueue copies the payload into the event queue and returns
    // immediately; nothing is delivered until ecs_dispatch drains the queue.
    // This is the safe way to raise events from inside a running system.
    // -----------------------------------------------------------------------
    printf("--- ecs_enqueue (deferred) -----------------------------------\n");

    damage_t burst[3] =
    {
        { .amount = 1, .x = 0.0f, .y = 0.0f },
        { .amount = 2, .x = 1.0f, .y = 1.0f },
        { .amount = 3, .x = 2.0f, .y = 2.0f },
    };

    for (int i = 0; i < 3; i++)
    {
        printf("  enqueue damage %d\n", burst[i].amount);
        ecs_enqueue(ecs, DamageEvent, &burst[i]);
    }

    printf("  ...nothing delivered yet; draining the queue:\n");
    ecs_dispatch(ecs);   // the three queued events fire now, in order

    // -----------------------------------------------------------------------
    // 3. Sender/receiver scoping with ecs_emit_from + ecs_subscribe_to.
    //
    // An event can be tagged with a sender id. A receiver registered with
    // ecs_subscribe_to only hears a matching sender, while plain ecs_subscribe
    // receivers hear every sender (and untagged ecs_emit too).
    // -----------------------------------------------------------------------
    printf("--- ecs_emit_from (scoped to a sender) -----------------------\n");

    ecs_id_t player_id = 1;   // any application-defined id works as a sender
    ecs_id_t goblin_id = 2;

    // This receiver only cares about damage emitted by the player.
    ecs_subscribe_to(ecs, DamageEvent, player_id, on_player_damage, NULL);

    printf("  emit from goblin (player HUD should stay quiet):\n");
    damage_t from_goblin = { .amount = 5, .x = 7.0f, .y = 3.0f };
    ecs_emit_from(ecs, DamageEvent, goblin_id, &from_goblin);

    printf("  emit from player (player HUD reacts):\n");
    damage_t from_player = { .amount = 8, .x = 1.0f, .y = 2.0f };
    ecs_emit_from(ecs, DamageEvent, player_id, &from_player);

    printf("--------------------------------------------------------------\n");

    ecs_free(ecs);
    return 0;
}