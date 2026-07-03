#define PICO_EMITTER_IMPLEMENTATION
#include "../pico_emitter.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

/* Helpers ------------------------------------------------------------------ */

static int g_call_count  = 0;
static int g_call_order[16];
static int g_order_count = 0;

static void reset_counters(void)
{
    g_call_count  = 0;
    g_order_count = 0;
}

static void listener_inc(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    g_call_count++;
}

static void listener_store_udata(const void* data, void* udata)
{
    (void)data;
    int* dst = (int*)udata;
    (*dst)++;
}

static void listener_record_order(const void* data, void* udata)
{
    (void)data;
    int id = *(int*)udata;
    g_call_order[g_order_count++] = id;
}

static void listener_read_data(const void* data, void* udata)
{
    int* dst = (int*)udata;
    *dst = *(int*)data;
}

typedef struct { int x; int y; float z; } test_vec3_t;

static void listener_read_vec3(const void* data, void* udata)
{
    test_vec3_t* dst = (test_vec3_t*)udata;
    *dst = *(const test_vec3_t*)data;
}

static int g_read_results[16];
static int g_read_count = 0;

static void listener_append_data(const void* data, void* udata)
{
    (void)udata;
    g_read_results[g_read_count++] = *(const int*)data;
}

/* Tests -------------------------------------------------------------------- */

TEST_CASE(test_create_free)
{
    emitter_t* em = emitter_create(4);
    REQUIRE(em != NULL);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_count_starts_at_zero)
{
    emitter_t* em = emitter_create(3);
    REQUIRE(emitter_count(em, 0) == 0);
    REQUIRE(emitter_count(em, 1) == 0);
    REQUIRE(emitter_count(em, 2) == 0);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_on_increases_count)
{
    emitter_t* em = emitter_create(2);
    emitter_on(em, 0, listener_inc, NULL);
    REQUIRE(emitter_count(em, 0) == 1);
    emitter_on(em, 0, listener_inc, NULL);
    REQUIRE(emitter_count(em, 0) == 2);
    REQUIRE(emitter_count(em, 1) == 0);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emit_calls_listener)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 1);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emit_calls_multiple_listeners)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 3);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emit_passes_data)
{
    int result = 0;
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_read_data, &result);
    int payload = 99;
    emitter_emit(em, 0, &payload);
    REQUIRE(result == 99);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emit_passes_udata)
{
    int counter = 0;
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_store_udata, &counter);
    emitter_on(em, 0, listener_store_udata, &counter);
    emitter_emit(em, 0, NULL);
    REQUIRE(counter == 2);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emit_fires_in_order)
{
    reset_counters();
    int ids[3] = { 10, 20, 30 };
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_record_order, &ids[0]);
    emitter_on(em, 0, listener_record_order, &ids[1]);
    emitter_on(em, 0, listener_record_order, &ids[2]);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_order_count   == 3);
    REQUIRE(g_call_order[0] == 10);
    REQUIRE(g_call_order[1] == 20);
    REQUIRE(g_call_order[2] == 30);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emit_no_listeners_is_safe)
{
    emitter_t* em = emitter_create(2);
    emitter_emit(em, 0, NULL);
    emitter_emit(em, 1, NULL);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_events_are_independent)
{
    reset_counters();
    emitter_t* em = emitter_create(3);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_on(em, 2, listener_inc, NULL);
    emitter_on(em, 2, listener_inc, NULL);
    emitter_emit(em, 1, NULL); /* no listeners */
    REQUIRE(g_call_count == 0);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 1);
    emitter_emit(em, 2, NULL);
    REQUIRE(g_call_count == 3);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_off_removes_listener)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_off(em, 0, listener_inc);
    REQUIRE(emitter_count(em, 0) == 0);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 0);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_off_removes_first_match_only)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_off(em, 0, listener_inc);
    REQUIRE(emitter_count(em, 0) == 1);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 1);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_off_unregistered_is_safe)
{
    emitter_t* em = emitter_create(1);
    /* Should not assert/crash when listener is not registered. */
    emitter_off(em, 0, listener_inc);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_off_all_removes_all)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_off_all(em, 0);
    REQUIRE(emitter_count(em, 0) == 0);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 0);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_once_fires_once)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_once(em, 0, listener_inc, NULL);
    emitter_emit(em, 0, NULL);
    emitter_emit(em, 0, NULL);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 1);
    REQUIRE(emitter_count(em, 0) == 0);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_once_and_on_together)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_once(em, 0, listener_inc, NULL);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_emit(em, 0, NULL); /* both fire */
    REQUIRE(g_call_count == 2);
    emitter_emit(em, 0, NULL); /* only persistent fires */
    REQUIRE(g_call_count == 3);
    emitter_destroy(em);
    return true;
}

/* --------------------------------------------------------------------------
 * Safe-during-emit tests
 * -------------------------------------------------------------------------- */

static emitter_t* g_emitter_safe = NULL;

static void listener_calls_emitter_off(const void* data, void* udata)
{
    (void)data;
    emitter_off(g_emitter_safe, 0, listener_calls_emitter_off);
    g_call_count++;
    (void)udata;
}

TEST_CASE(test_emitter_off_during_emit)
{
    reset_counters();
    g_emitter_safe = emitter_create(1);
    /* Register same callback twice. */
    emitter_on(g_emitter_safe, 0, listener_calls_emitter_off, NULL);
    emitter_on(g_emitter_safe, 0, listener_inc,          NULL);
    emitter_emit(g_emitter_safe, 0, NULL);
    /* Both should have been called. */
    REQUIRE(g_call_count == 2);
    /* First listener removed itself; only the second remains. */
    REQUIRE(emitter_count(g_emitter_safe, 0) == 1);
    emitter_destroy(g_emitter_safe);
    g_emitter_safe = NULL;
    return true;
}

static void listener_calls_emitter_off_all(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    emitter_off_all(g_emitter_safe, 0);
    g_call_count++;
}

TEST_CASE(test_emitter_off_all_during_emit)
{
    reset_counters();
    g_emitter_safe = emitter_create(1);
    emitter_on(g_emitter_safe, 0, listener_calls_emitter_off_all, NULL);
    emitter_on(g_emitter_safe, 0, listener_inc,              NULL);
    emitter_on(g_emitter_safe, 0, listener_inc,              NULL);
    emitter_emit(g_emitter_safe, 0, NULL);
    /* First callback fires and calls emitter_off_all; remaining two are skipped. */
    REQUIRE(g_call_count == 1);
    REQUIRE(emitter_count(g_emitter_safe, 0) == 0);
    emitter_destroy(g_emitter_safe);
    g_emitter_safe = NULL;
    return true;
}

static void listener_calls_emitter_on(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    emitter_on(g_emitter_safe, 0, listener_inc, NULL);
    g_call_count++;
}

TEST_CASE(test_emitter_on_during_emit_not_called)
{
    reset_counters();
    g_emitter_safe = emitter_create(1);
    emitter_on(g_emitter_safe, 0, listener_calls_emitter_on, NULL);
    emitter_emit(g_emitter_safe, 0, NULL);
    /* Only the first listener fires; the newly registered one does not. */
    REQUIRE(g_call_count == 1);
    /* Next emit fires both. */
    emitter_emit(g_emitter_safe, 0, NULL);
    REQUIRE(g_call_count == 3);
    emitter_destroy(g_emitter_safe);
    g_emitter_safe = NULL;
    return true;
}

static void listener_reregisters_once(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    emitter_once(g_emitter_safe, 0, listener_reregisters_once, NULL);
    g_call_count++;
}

TEST_CASE(test_emitter_once_reregister_during_emit)
{
    reset_counters();
    g_emitter_safe = emitter_create(1);
    emitter_once(g_emitter_safe, 0, listener_reregisters_once, NULL);
    emitter_emit(g_emitter_safe, 0, NULL); /* fires, re-registers */
    REQUIRE(g_call_count == 1);
    REQUIRE(emitter_count(g_emitter_safe, 0) == 1);
    emitter_emit(g_emitter_safe, 0, NULL); /* fires again */
    REQUIRE(g_call_count == 2);
    emitter_destroy(g_emitter_safe);
    g_emitter_safe = NULL;
    return true;
}

TEST_CASE(test_emitter_resize_grow_adds_events)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_inc, NULL);
    REQUIRE(emitter_resize(em, 3)); /* expand num_events from 1 to 3 */
    /* Existing listener is preserved. */
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 1);
    /* New event slots are usable and start empty. */
    REQUIRE(emitter_count(em, 2) == 0);
    emitter_on(em, 2, listener_inc, NULL);
    emitter_emit(em, 2, NULL);
    REQUIRE(g_call_count == 2);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_resize_preserves_listeners)
{
    int a = 0;
    int b = 0;
    emitter_t* em = emitter_create(2);
    emitter_on(em, 0, listener_store_udata, &a);
    emitter_on(em, 1, listener_store_udata, &b);
    REQUIRE(emitter_resize(em, 5));
    emitter_emit(em, 0, NULL);
    emitter_emit(em, 1, NULL);
    REQUIRE(a == 1);
    REQUIRE(b == 1);
    emitter_destroy(em);
    return true;
}

TEST_CASE(test_emitter_resize_shrink_drops_events)
{
    reset_counters();
    emitter_t* em = emitter_create(3);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_on(em, 2, listener_inc, NULL); /* listener on a slot to be dropped */
    REQUIRE(emitter_resize(em, 1)); /* drop events 1 and 2, freeing their slots */
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 1);
    emitter_destroy(em); /* must not leak or double-free the dropped slot */
    return true;
}

TEST_CASE(test_emitter_resize_same_size_is_noop)
{
    reset_counters();
    emitter_t* em = emitter_create(2);
    emitter_on(em, 1, listener_inc, NULL);
    REQUIRE(emitter_resize(em, 2));
    emitter_emit(em, 1, NULL);
    REQUIRE(g_call_count == 1);
    emitter_destroy(em);
    return true;
}

/* Test suite (emitter) ----------------------------------------------------- */

TEST_SUITE(suite_emitter)
{
    RUN_TEST_CASE(test_create_free);
    RUN_TEST_CASE(test_emitter_count_starts_at_zero);
    RUN_TEST_CASE(test_emitter_on_increases_count);
    RUN_TEST_CASE(test_emit_calls_listener);
    RUN_TEST_CASE(test_emit_calls_multiple_listeners);
    RUN_TEST_CASE(test_emit_passes_data);
    RUN_TEST_CASE(test_emit_passes_udata);
    RUN_TEST_CASE(test_emit_fires_in_order);
    RUN_TEST_CASE(test_emit_no_listeners_is_safe);
    RUN_TEST_CASE(test_events_are_independent);
    RUN_TEST_CASE(test_emitter_off_removes_listener);
    RUN_TEST_CASE(test_emitter_off_removes_first_match_only);
    RUN_TEST_CASE(test_emitter_off_unregistered_is_safe);
    RUN_TEST_CASE(test_emitter_off_all_removes_all);
    RUN_TEST_CASE(test_emitter_once_fires_once);
    RUN_TEST_CASE(test_emitter_once_and_on_together);
    RUN_TEST_CASE(test_emitter_off_during_emit);
    RUN_TEST_CASE(test_emitter_off_all_during_emit);
    RUN_TEST_CASE(test_emitter_on_during_emit_not_called);
    RUN_TEST_CASE(test_emitter_once_reregister_during_emit);
    RUN_TEST_CASE(test_emitter_resize_grow_adds_events);
    RUN_TEST_CASE(test_emitter_resize_preserves_listeners);
    RUN_TEST_CASE(test_emitter_resize_shrink_drops_events);
    RUN_TEST_CASE(test_emitter_resize_same_size_is_noop);
}

/* --------------------------------------------------------------------------
 * Queued emitter tests
 * -------------------------------------------------------------------------- */

TEST_CASE(test_queued_create_destroy)
{
    queued_emitter_t* qe = queued_emitter_create(4);
    REQUIRE(qe != NULL);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_emit_deferred)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    REQUIRE(g_call_count == 0); /* not fired yet */
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_emit_immediate)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_emit(qe, 0, NULL); /* alias for immediate dispatch */
    REQUIRE(g_call_count == 1); /* fired synchronously, not deferred */
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 1); /* nothing was queued */
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_emit_immediate_passes_data)
{
    int result = 0;
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_read_data, &result);
    int payload = 91;
    queued_emitter_emit(qe, 0, &payload); /* payload forwarded, not copied */
    REQUIRE(result == 91);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_flush_dispatches)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 1);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_flush_order)
{
    reset_counters();
    int ids[3] = { 10, 20, 30 };
    queued_emitter_t* qe = queued_emitter_create(3);
    queued_emitter_on(qe, 0, listener_record_order, &ids[0]);
    queued_emitter_on(qe, 1, listener_record_order, &ids[1]);
    queued_emitter_on(qe, 2, listener_record_order, &ids[2]);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_enqueue_raw(qe, 1, NULL, 0);
    queued_emitter_enqueue_raw(qe, 2, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_order_count   == 3);
    REQUIRE(g_call_order[0] == 10);
    REQUIRE(g_call_order[1] == 20);
    REQUIRE(g_call_order[2] == 30);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_flush_clears_queue)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 1);
    queued_emitter_flush(qe); /* second flush does nothing */
    REQUIRE(g_call_count == 1);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_passes_data)
{
    int result = 0;
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_read_data, &result);
    int payload = 77;
    queued_emitter_enqueue_raw(qe, 0, &payload, sizeof(payload));
    queued_emitter_flush(qe);
    REQUIRE(result == 77);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_once)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_once(qe, 0, listener_inc, NULL);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 1);
    REQUIRE(queued_emitter_count(qe, 0) == 0);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_off)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_off(qe, 0, listener_inc);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 0);
    queued_emitter_destroy(qe);
    return true;
}

TEST_CASE(test_queued_off_all)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_off_all(qe, 0);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 0);
    queued_emitter_destroy(qe);
    return true;
}

static queued_emitter_t* g_qe_safe = NULL;

/* Bound on the number of events the re-enqueuing listener will chain. */
#define QUEUED_ENQUEUE_CHAIN 5

static void queued_listener_enqueues(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    /* Re-enqueue until the chain bound is reached; flush drains the queue to
     * completion, so an unconditional re-enqueue would loop forever. */
    if (g_call_count < QUEUED_ENQUEUE_CHAIN - 1)
        queued_emitter_enqueue_raw(g_qe_safe, 0, NULL, 0);
    g_call_count++;
}

TEST_CASE(test_queued_emit_during_flush_dispatched)
{
    reset_counters();
    g_qe_safe = queued_emitter_create(1);
    queued_emitter_on(g_qe_safe, 0, queued_listener_enqueues, NULL);
    queued_emitter_enqueue_raw(g_qe_safe, 0, NULL, 0);
    /* A single flush dispatches the original event and every event the
     * listener enqueues during the flush, draining the queue to completion. */
    queued_emitter_flush(g_qe_safe);
    REQUIRE(g_call_count == QUEUED_ENQUEUE_CHAIN);
    queued_emitter_destroy(g_qe_safe);
    g_qe_safe = NULL;
    return true;
}

/* A listener on event 0 enqueues event 1 during flush; both events must be
 * dispatched within the same flush, fanning out across event types. */
static void queued_listener_chains_event1(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    queued_emitter_enqueue_raw(g_qe_safe, 1, NULL, 0);
    g_call_order[g_order_count++] = 0;
}

static void queued_listener_records_event1(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    g_call_order[g_order_count++] = 1;
}

TEST_CASE(test_queued_nested_cross_event_dispatch)
{
    reset_counters();
    g_qe_safe = queued_emitter_create(2);
    queued_emitter_on(g_qe_safe, 0, queued_listener_chains_event1, NULL);
    queued_emitter_on(g_qe_safe, 1, queued_listener_records_event1, NULL);
    queued_emitter_enqueue_raw(g_qe_safe, 0, NULL, 0);
    /* One flush dispatches event 0, which enqueues event 1, which is then
     * dispatched in the same flush. */
    queued_emitter_flush(g_qe_safe);
    REQUIRE(g_order_count   == 2);
    REQUIRE(g_call_order[0] == 0); /* event 0 handled first */
    REQUIRE(g_call_order[1] == 1); /* event 1 handled after, same flush */
    queued_emitter_destroy(g_qe_safe);
    g_qe_safe = NULL;
    return true;
}

/* A payload enqueued by a listener during flush must survive the arena
 * ping-pong and be delivered intact to the nested event's listener. */
static int g_nested_result = 0;

static void queued_listener_enqueues_payload(const void* data, void* udata)
{
    (void)data;
    (void)udata;
    int nested = 1234;
    queued_emitter_enqueue_raw(g_qe_safe, 1, &nested, sizeof(nested));
}

static void queued_listener_store_nested(const void* data, void* udata)
{
    (void)udata;
    g_nested_result = *(const int*)data;
}

TEST_CASE(test_queued_nested_payload_delivered)
{
    reset_counters();
    g_nested_result = 0;
    g_qe_safe = queued_emitter_create(2);
    queued_emitter_on(g_qe_safe, 0, queued_listener_enqueues_payload, NULL);
    queued_emitter_on(g_qe_safe, 1, queued_listener_store_nested, NULL);
    queued_emitter_enqueue_raw(g_qe_safe, 0, NULL, 0);
    /* The payload is copied into the flipped write arena during flush; it must
     * remain valid when the nested event is dispatched. */
    queued_emitter_flush(g_qe_safe);
    REQUIRE(g_nested_result == 1234);
    queued_emitter_destroy(g_qe_safe);
    g_qe_safe = NULL;
    return true;
}

/* Proves copy semantics: mutating the source after enqueue must not affect the
 * value received by the listener. */
TEST_CASE(test_queued_data_copied_not_referenced)
{
    int result = 0;
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_read_data, &result);
    int payload = 42;
    queued_emitter_enqueue_raw(qe, 0, &payload, sizeof(payload));
    payload = 999; /* mutate original after enqueue */
    queued_emitter_flush(qe);
    REQUIRE(result == 42); /* must see the copied value, not 999 */
    queued_emitter_destroy(qe);
    return true;
}

/* Larger struct payload: exercises arena alignment and multi-field copy. */
TEST_CASE(test_queued_large_payload)
{
    test_vec3_t result = { 0, 0, 0.0f };
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_read_vec3, &result);
    test_vec3_t payload = { 10, 20, 3.5f };
    queued_emitter_enqueue_raw(qe, 0, &payload, sizeof(payload));
    queued_emitter_flush(qe);
    REQUIRE(result.x == 10);
    REQUIRE(result.y == 20);
    REQUIRE(result.z == 3.5f);
    queued_emitter_destroy(qe);
    return true;
}

/* Queue more than EMITTER_INIT_CAPACITY events to force arena grow. */
TEST_CASE(test_queued_many_events_arena_grow)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    for (int i = 0; i < 24; i++)
        queued_emitter_enqueue_raw(qe, 0, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 24);
    queued_emitter_destroy(qe);
    return true;
}

/* Arena reset and reuse: flush, re-emit, flush again. */
TEST_CASE(test_queued_arena_reuse_after_flush)
{
    int result = 0;
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_read_data, &result);
    int payload = 11;
    queued_emitter_enqueue_raw(qe, 0, &payload, sizeof(payload));
    queued_emitter_flush(qe);
    REQUIRE(result == 11);
    payload = 22;
    result  = 0;
    queued_emitter_enqueue_raw(qe, 0, &payload, sizeof(payload));
    queued_emitter_flush(qe);
    REQUIRE(result == 22);
    queued_emitter_destroy(qe);
    return true;
}

/* queued_emitter_enqueue deduces payload size from the pointer type; the
 * listener must receive the copied value even after the source is mutated. */
TEST_CASE(test_queued_typed_emit_deduces_size)
{
    int result = 0;
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_read_data, &result);
    int payload = 55;
    queued_emitter_enqueue(qe, 0, &payload);
    payload = 999; /* mutate after enqueue */
    queued_emitter_flush(qe);
    REQUIRE(result == 55); /* copied value, not 999 */
    queued_emitter_destroy(qe);
    return true;
}

/* Passing NULL to queued_emitter_enqueue must not crash; the listener
 * receives a NULL data pointer. */
TEST_CASE(test_queued_typed_emit_null_is_safe)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_enqueue(qe, 0, (int*)NULL);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 1);
    queued_emitter_destroy(qe);
    return true;
}

/* Multiple events with distinct payloads each get an independent arena copy. */
TEST_CASE(test_queued_multiple_payloads_independent)
{
    g_read_count = 0;
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_append_data, NULL);
    int vals[4] = { 1, 2, 3, 4 };
    for (int i = 0; i < 4; i++)
        queued_emitter_enqueue_raw(qe, 0, &vals[i], sizeof(int));
    for (int i = 0; i < 4; i++) /* mutate sources before flush */
        vals[i] = 0;
    queued_emitter_flush(qe);
    REQUIRE(g_read_count      == 4);
    REQUIRE(g_read_results[0] == 1);
    REQUIRE(g_read_results[1] == 2);
    REQUIRE(g_read_results[2] == 3);
    REQUIRE(g_read_results[3] == 4);
    queued_emitter_destroy(qe);
    return true;
}

/* Resizing the queued emitter grows the wrapped emitter's event slots; new
 * events become usable while queued events on existing slots still flush. */
TEST_CASE(test_queued_resize_grow_adds_events)
{
    reset_counters();
    queued_emitter_t* qe = queued_emitter_create(1);
    queued_emitter_on(qe, 0, listener_inc, NULL);
    queued_emitter_enqueue_raw(qe, 0, NULL, 0); /* queued before the resize */
    REQUIRE(queued_emitter_resize(qe, 3));  /* expand num_events from 1 to 3 */
    queued_emitter_on(qe, 2, listener_inc, NULL);
    queued_emitter_enqueue_raw(qe, 2, NULL, 0);
    queued_emitter_flush(qe);
    REQUIRE(g_call_count == 2); /* both the pre-resize and new-event fire */
    queued_emitter_destroy(qe);
    return true;
}

/* Test suite (queued emitter) ---------------------------------------------- */

TEST_SUITE(suite_queued_emitter)
{
    RUN_TEST_CASE(test_queued_create_destroy);
    RUN_TEST_CASE(test_queued_emit_deferred);
    RUN_TEST_CASE(test_queued_emit_immediate);
    RUN_TEST_CASE(test_queued_emit_immediate_passes_data);
    RUN_TEST_CASE(test_queued_flush_dispatches);
    RUN_TEST_CASE(test_queued_flush_order);
    RUN_TEST_CASE(test_queued_flush_clears_queue);
    RUN_TEST_CASE(test_queued_passes_data);
    RUN_TEST_CASE(test_queued_once);
    RUN_TEST_CASE(test_queued_off);
    RUN_TEST_CASE(test_queued_off_all);
    RUN_TEST_CASE(test_queued_emit_during_flush_dispatched);
    RUN_TEST_CASE(test_queued_nested_cross_event_dispatch);
    RUN_TEST_CASE(test_queued_nested_payload_delivered);
    RUN_TEST_CASE(test_queued_data_copied_not_referenced);
    RUN_TEST_CASE(test_queued_large_payload);
    RUN_TEST_CASE(test_queued_many_events_arena_grow);
    RUN_TEST_CASE(test_queued_arena_reuse_after_flush);
    RUN_TEST_CASE(test_queued_multiple_payloads_independent);
    RUN_TEST_CASE(test_queued_typed_emit_deduces_size);
    RUN_TEST_CASE(test_queued_typed_emit_null_is_safe);
    RUN_TEST_CASE(test_queued_resize_grow_adds_events);
}

int main(void)
{
    pu_display_colors(true);
    RUN_TEST_SUITE(suite_emitter);
    RUN_TEST_SUITE(suite_queued_emitter);
    pu_print_stats();
    return pu_test_failed();
}
