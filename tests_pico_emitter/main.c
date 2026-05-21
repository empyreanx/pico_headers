#define EVENTEMITTER_IMPLEMENTATION
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

/* Tests -------------------------------------------------------------------- */

TEST_CASE(test_create_free)
{
    emitter_t* em = emitter_create(4);
    REQUIRE(em != NULL);
    emitter_free(em);
    return true;
}

TEST_CASE(test_emitter_count_starts_at_zero)
{
    emitter_t* em = emitter_create(3);
    REQUIRE(emitter_count(em, 0) == 0);
    REQUIRE(emitter_count(em, 1) == 0);
    REQUIRE(emitter_count(em, 2) == 0);
    emitter_free(em);
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
    emitter_free(em);
    return true;
}

TEST_CASE(test_emit_calls_listener)
{
    reset_counters();
    emitter_t* em = emitter_create(1);
    emitter_on(em, 0, listener_inc, NULL);
    emitter_emit(em, 0, NULL);
    REQUIRE(g_call_count == 1);
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(em);
    return true;
}

TEST_CASE(test_emit_no_listeners_is_safe)
{
    emitter_t* em = emitter_create(2);
    emitter_emit(em, 0, NULL);
    emitter_emit(em, 1, NULL);
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(em);
    return true;
}

TEST_CASE(test_emitter_off_unregistered_is_safe)
{
    emitter_t* em = emitter_create(1);
    /* Should not assert/crash when listener is not registered. */
    emitter_off(em, 0, listener_inc);
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(em);
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
    emitter_free(g_emitter_safe);
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
    emitter_free(g_emitter_safe);
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
    emitter_free(g_emitter_safe);
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
    emitter_free(g_emitter_safe);
    g_emitter_safe = NULL;
    return true;
}

/* Test suite --------------------------------------------------------------- */

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
}

int main(void)
{
    pu_display_colors(true);
    RUN_TEST_SUITE(suite_emitter);
    pu_print_stats();
    return pu_test_failed();
}
