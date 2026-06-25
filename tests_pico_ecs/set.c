#include "common.h"

// --- Helpers ---

static bool set_callback_called = false;

static void comp_on_set(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* udata)
{
    (void)ecs;
    (void)entity;
    (void)comp;
    (void)udata;
    set_callback_called = true;
}

static comp_t deferred_set_data = { .used = true };

static ecs_ret_t set_system(ecs_t* ecs,
                            ecs_entity_t* entities,
                            size_t entity_count,
                            void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_set(ecs, entities[i], comp1, &deferred_set_data);
    }

    return 0;
}

static bool deferred_cb_called = false;

static void on_set_deferred(ecs_t* ecs,
                            ecs_entity_t entity,
                            ecs_comp_t comp,
                            void* udata)
{
    (void)ecs;
    (void)entity;
    (void)comp;
    (void)udata;
    deferred_cb_called = true;
}

typedef struct { ecs_comp_t comp; comp_t data; } set_args_t;

static ecs_ret_t set_system_with_args(ecs_t* ecs,
                                      ecs_entity_t* entities,
                                      size_t entity_count,
                                      void* udata)
{
    set_args_t* args = (set_args_t*)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_set(ecs, entities[i], args->comp, &args->data);
    }

    return 0;
}

// =============================================================
// suite_set: ecs_set data copy, no-op, callback, and deferred
// =============================================================

TEST_CASE(test_set_copies_data)
{
    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);

    comp_t* comp = ecs_get(ecs, entity, comp1);
    REQUIRE(!comp->used);

    comp_t data = { .used = true };
    ecs_set(ecs, entity, comp1, &data);

    REQUIRE(comp->used);

    return true;
}

TEST_CASE(test_set_without_component)
{
    ecs_entity_t entity = ecs_create(ecs);

    // Entity does not have comp1 - ecs_set should add the component
    REQUIRE(!ecs_has(ecs, entity, comp1));

    comp_t data = { .used = true };
    ecs_set(ecs, entity, comp1, &data);

    // Component should not be absent
    REQUIRE(ecs_has(ecs, entity, comp1));

    return true;
}

TEST_CASE(test_set_on_set_callback)
{
    set_callback_called = false;

    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), NULL);
    ecs_on_set(ecs, comp_type, comp_on_set, false);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type, NULL);

    comp_t data = { .used = true };
    ecs_set(ecs, entity, comp_type, &data);

    // The data is copied synchronously, but on_set is delivered on dispatch
    REQUIRE(((comp_t*)ecs_get(ecs, entity, comp_type))->used);
    REQUIRE(!set_callback_called);

    ecs_dispatch(ecs);

    REQUIRE(set_callback_called);

    return true;
}

TEST_CASE(test_set_deferred)
{
    sys1 = ecs_define_system(ecs, set_system, NULL);
    ecs_require(ecs, sys1, comp1);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);

    REQUIRE(!((comp_t*)ecs_get(ecs, entity, comp1))->used);

    ecs_run_system(ecs, sys1, 0);

    // Deferred ecs_set should have been applied after the system completed
    REQUIRE(((comp_t*)ecs_get(ecs, entity, comp1))->used);

    return true;
}

TEST_CASE(test_set_isolated)
{
    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);
    ecs_add(ecs, entity, comp2, NULL);

    comp_t data = { .used = true };
    ecs_set(ecs, entity, comp1, &data);

    // Only comp1 should be updated
    REQUIRE(((comp_t*)ecs_get(ecs, entity, comp1))->used);
    REQUIRE(!((comp_t*)ecs_get(ecs, entity, comp2))->used);

    return true;
}

TEST_CASE(test_set_deferred_fires_callback)
{
    deferred_cb_called = false;

    ecs_comp_t comp_cb = ecs_define_component(ecs, sizeof(comp_t), NULL);
    ecs_on_set(ecs, comp_cb, on_set_deferred, false);

    set_args_t args = { .comp = comp_cb, .data = { .used = true } };

    sys1 = ecs_define_system(ecs, set_system_with_args, &(ecs_sys_desc_t)
    {
        .udata = &args
    });

    ecs_require(ecs, sys1, comp_cb);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_cb, NULL);

    ecs_run_system(ecs, sys1, 0);

    // The deferred set is applied at flush; its on_set fires on dispatch
    REQUIRE(((comp_t*)ecs_get(ecs, entity, comp_cb))->used);
    REQUIRE(!deferred_cb_called);

    ecs_dispatch(ecs);

    REQUIRE(deferred_cb_called);

    return true;
}

TEST_SUITE(suite_set)
{
    RUN_TEST_CASE(test_set_copies_data);
    RUN_TEST_CASE(test_set_without_component);
    RUN_TEST_CASE(test_set_on_set_callback);
    RUN_TEST_CASE(test_set_deferred);
    RUN_TEST_CASE(test_set_isolated);
    RUN_TEST_CASE(test_set_deferred_fires_callback);
}
