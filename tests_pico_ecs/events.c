#include "common.h"

// --- Payloads ---

typedef struct { int value; } evt_payload_t;

// --- Shared listener state (reset at the start of each test case) ---

static int    g_call_count;
static int    g_sum;
static int    g_order[32];
static int    g_order_count;
static void*  g_last_udata;
static size_t g_last_size;
static bool   g_payload_was_null;

static void reset_globals(void)
{
    g_call_count       = 0;
    g_sum              = 0;
    g_order_count      = 0;
    g_last_udata       = NULL;
    g_last_size        = 0;
    g_payload_was_null = false;
}

// --- Listeners ---

static void on_accumulate(ecs_t* ecs,
                          ecs_event_t event,
                          const void* payload,
                          size_t payload_size,
                          void* udata)
{
    (void)ecs;
    (void)event;

    g_call_count++;
    g_last_udata = udata;
    g_last_size  = payload_size;

    if (payload)
    {
        const evt_payload_t* p = (const evt_payload_t*)payload;
        g_sum += p->value;
        g_order[g_order_count++] = p->value;
    }
    else
    {
        g_payload_was_null = true;
    }
}

// Counts raw subscriptions to a built-in component event. The payload struct is
// internal to the library, so the listener only records that it fired and that a
// payload was delivered.
static int  g_comp_event_count;
static bool g_comp_event_had_payload;

static void on_comp_event(ecs_t* ecs,
                          ecs_event_t event,
                          const void* payload,
                          size_t payload_size,
                          void* udata)
{
    (void)ecs;
    (void)event;
    (void)payload_size;
    (void)udata;

    g_comp_event_had_payload = (payload != NULL);
    g_comp_event_count++;
}

// Listener that re-emits another event while dispatch is running
static ecs_event_t g_chain_event;

static void on_trigger(ecs_t* ecs,
                       ecs_event_t event,
                       const void* payload,
                       size_t payload_size,
                       void* udata)
{
    (void)event;
    (void)payload;
    (void)payload_size;
    (void)udata;

    evt_payload_t p = { .value = 100 };
    ecs_enqueue(ecs, g_chain_event, &p);
}

// System that emits an event for each entity it iterates
static ecs_event_t g_system_event;

static ecs_ret_t emit_system(ecs_t* ecs,
                             ecs_entity_t* entities,
                             size_t entity_count,
                             void* udata)
{
    (void)entities;
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        evt_payload_t p = { .value = 1 };
        ecs_enqueue(ecs, g_system_event, &p);
    }

    return 0;
}

// =============================================================
// suite_events: define/subscribe/emit/dispatch/unsubscribe
// =============================================================

TEST_CASE(test_event_emit_and_dispatch)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    evt_payload_t p = { .value = 42 };
    ecs_enqueue(ecs, event, &p);

    // Nothing is delivered until dispatch
    REQUIRE(g_call_count == 0);

    ecs_dispatch(ecs);

    REQUIRE(g_call_count == 1);
    REQUIRE(g_sum == 42);
    REQUIRE(g_last_size == sizeof(evt_payload_t));

    return true;
}

TEST_CASE(test_event_dispatch_drains_queue)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    evt_payload_t p = { .value = 7 };
    ecs_enqueue(ecs, event, &p);

    ecs_dispatch(ecs);
    REQUIRE(g_call_count == 1);

    // The queue is empty now, so a second dispatch delivers nothing
    ecs_dispatch(ecs);
    REQUIRE(g_call_count == 1);

    return true;
}

TEST_CASE(test_event_multiple_listeners)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);
    ecs_subscribe(ecs, event, on_accumulate, NULL);
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    evt_payload_t p = { .value = 5 };
    ecs_enqueue(ecs, event, &p);
    ecs_dispatch(ecs);

    // All three listeners fire for the single event
    REQUIRE(g_call_count == 3);
    REQUIRE(g_sum == 15);

    return true;
}

TEST_CASE(test_event_emission_order)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    for (int i = 1; i <= 4; i++)
    {
        evt_payload_t p = { .value = i };
        ecs_enqueue(ecs, event, &p);
    }

    ecs_dispatch(ecs);

    // Events are delivered in the order they were emitted
    REQUIRE(g_order_count == 4);
    REQUIRE(g_order[0] == 1);
    REQUIRE(g_order[1] == 2);
    REQUIRE(g_order[2] == 3);
    REQUIRE(g_order[3] == 4);

    return true;
}

TEST_CASE(test_event_payload_is_copied)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    evt_payload_t p = { .value = 11 };
    ecs_enqueue(ecs, event, &p);

    // Mutating the source after emit must not affect the queued payload
    p.value = 999;

    ecs_dispatch(ecs);

    REQUIRE(g_sum == 11);

    return true;
}

TEST_CASE(test_event_no_payload)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, 0);
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    ecs_enqueue(ecs, event, NULL);
    ecs_enqueue(ecs, event, NULL);
    ecs_dispatch(ecs);

    REQUIRE(g_call_count == 2);
    REQUIRE(g_payload_was_null);
    REQUIRE(g_last_size == 0);

    return true;
}

TEST_CASE(test_event_listener_udata)
{
    reset_globals();

    int marker = 0;

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, &marker);

    evt_payload_t p = { .value = 1 };
    ecs_enqueue(ecs, event, &p);
    ecs_dispatch(ecs);

    REQUIRE(g_last_udata == &marker);

    return true;
}

TEST_CASE(test_event_unsubscribe)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);
    ecs_id_t sub = ecs_subscribe(ecs, event, on_accumulate, NULL);

    ecs_unsubscribe(ecs, event, sub);

    evt_payload_t p = { .value = 3 };
    ecs_enqueue(ecs, event, &p);
    ecs_dispatch(ecs);

    // Only the remaining listener fires
    REQUIRE(g_call_count == 1);

    return true;
}

TEST_CASE(test_event_subscribe_reuses_slot)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);
    ecs_id_t sub = ecs_subscribe(ecs, event, on_accumulate, NULL);

    ecs_unsubscribe(ecs, event, sub);

    // A new subscription should reuse the freed slot
    ecs_id_t sub2 = ecs_subscribe(ecs, event, on_accumulate, NULL);
    REQUIRE(sub2 == sub);

    evt_payload_t p = { .value = 1 };
    ecs_enqueue(ecs, event, &p);
    ecs_dispatch(ecs);

    REQUIRE(g_call_count == 2);

    return true;
}

TEST_CASE(test_event_chained_emit_during_dispatch)
{
    reset_globals();

    ecs_event_t trigger = ecs_define_event(ecs, 0);
    g_chain_event       = ecs_define_event(ecs, sizeof(evt_payload_t));

    ecs_subscribe(ecs, trigger, on_trigger, NULL);
    ecs_subscribe(ecs, g_chain_event, on_accumulate, NULL);

    ecs_enqueue(ecs, trigger, NULL);
    ecs_dispatch(ecs);

    // The event emitted by the listener is delivered in the same dispatch pass
    REQUIRE(g_call_count == 1);
    REQUIRE(g_sum == 100);

    return true;
}

TEST_CASE(test_event_emit_from_system)
{
    reset_globals();

    g_system_event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, g_system_event, on_accumulate, NULL);

    sys1 = ecs_define_system(ecs, emit_system, NULL);
    ecs_require(ecs, sys1, comp1);

    const int entity_count = 5;
    for (int i = 0; i < entity_count; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1, NULL);
    }

    ecs_run_system(ecs, sys1, 0);

    // Emitting inside a running system only queues; nothing delivered yet
    REQUIRE(g_call_count == 0);

    ecs_dispatch(ecs);

    REQUIRE(g_call_count == entity_count);
    REQUIRE(g_sum == entity_count);

    return true;
}

TEST_CASE(test_event_reset_discards_pending)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    evt_payload_t p = { .value = 1 };
    ecs_enqueue(ecs, event, &p);

    // Reset must discard the pending event but keep the subscription
    ecs_reset(ecs);

    ecs_dispatch(ecs);
    REQUIRE(g_call_count == 0);

    // The subscription survives, so a fresh event is still delivered
    ecs_enqueue(ecs, event, &p);
    ecs_dispatch(ecs);
    REQUIRE(g_call_count == 1);

    return true;
}

TEST_CASE(test_event_emit_synchronous)
{
    reset_globals();

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe(ecs, event, on_accumulate, NULL);

    evt_payload_t p = { .value = 42 };
    ecs_emit(ecs, event, &p);

    // Synchronous emit delivers immediately, without ecs_dispatch
    REQUIRE(g_call_count == 1);
    REQUIRE(g_sum == 42);

    // Nothing was queued, so a dispatch delivers nothing further
    ecs_dispatch(ecs);
    REQUIRE(g_call_count == 1);

    return true;
}

TEST_CASE(test_event_emit_from_synchronous)
{
    reset_globals();

    const ecs_id_t source = 5;

    ecs_event_t event = ecs_define_event(ecs, sizeof(evt_payload_t));
    ecs_subscribe_to(ecs, event, source, on_accumulate, NULL); // scoped
    ecs_subscribe(ecs, event, on_accumulate, NULL);            // unscoped

    evt_payload_t p = { .value = 1 };

    // Matching source: both the scoped and unscoped listeners fire
    ecs_emit_from(ecs, event, source, &p);
    REQUIRE(g_call_count == 2);

    // Different source: only the unscoped listener fires
    ecs_emit_from(ecs, event, source + 1, &p);
    REQUIRE(g_call_count == 3);

    // No source: only the unscoped listener fires
    ecs_emit(ecs, event, &p);
    REQUIRE(g_call_count == 4);

    return true;
}

TEST_CASE(test_event_raw_component_subscription)
{
    g_comp_event_count       = 0;
    g_comp_event_had_payload = false;

    // comp1 has no ecs_on_add callback; a raw subscriber to the built-in add
    // event still observes the add (the payload type is internal to the library)
    ecs_subscribe(ecs, ecs_get_add_event(ecs), on_comp_event, NULL);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);

    // The lifecycle events are enqueued, so nothing is delivered until dispatch
    REQUIRE(g_comp_event_count == 0);
    ecs_dispatch(ecs);

    REQUIRE(g_comp_event_count == 1);
    REQUIRE(g_comp_event_had_payload);

    return true;
}

TEST_SUITE(suite_events)
{
    RUN_TEST_CASE(test_event_emit_and_dispatch);
    RUN_TEST_CASE(test_event_dispatch_drains_queue);
    RUN_TEST_CASE(test_event_multiple_listeners);
    RUN_TEST_CASE(test_event_emission_order);
    RUN_TEST_CASE(test_event_payload_is_copied);
    RUN_TEST_CASE(test_event_no_payload);
    RUN_TEST_CASE(test_event_listener_udata);
    RUN_TEST_CASE(test_event_unsubscribe);
    RUN_TEST_CASE(test_event_subscribe_reuses_slot);
    RUN_TEST_CASE(test_event_chained_emit_during_dispatch);
    RUN_TEST_CASE(test_event_emit_from_system);
    RUN_TEST_CASE(test_event_reset_discards_pending);
    RUN_TEST_CASE(test_event_emit_synchronous);
    RUN_TEST_CASE(test_event_emit_from_synchronous);
    RUN_TEST_CASE(test_event_raw_component_subscription);
}