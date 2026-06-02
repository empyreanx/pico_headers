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

// =============================================================
// suite_set: ecs_set data copy, no-op, callback, and deferred
// =============================================================

TEST_CASE(test_set_copies_data)
{
    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1);

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

    // Entity does not have comp1 - ecs_set should be a no-op
    REQUIRE(!ecs_has(ecs, entity, comp1));

    comp_t data = { .used = true };
    ecs_set(ecs, entity, comp1, &data);

    // Component should still be absent
    REQUIRE(!ecs_has(ecs, entity, comp1));

    return true;
}

TEST_CASE(test_set_on_set_callback)
{
    set_callback_called = false;

    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), NULL, NULL, comp_on_set, NULL);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type);

    comp_t data = { .used = true };
    ecs_set(ecs, entity, comp_type, &data);

    REQUIRE(set_callback_called);
    REQUIRE(((comp_t*)ecs_get(ecs, entity, comp_type))->used);

    return true;
}

TEST_CASE(test_set_deferred)
{
    sys1 = ecs_define_system(ecs, 0, set_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1);

    REQUIRE(!((comp_t*)ecs_get(ecs, entity, comp1))->used);

    ecs_run_system(ecs, sys1, 0);

    // Deferred ecs_set should have been applied after the system completed
    REQUIRE(((comp_t*)ecs_get(ecs, entity, comp1))->used);

    return true;
}

TEST_SUITE(suite_set)
{
    RUN_TEST_CASE(test_set_copies_data);
    RUN_TEST_CASE(test_set_without_component);
    RUN_TEST_CASE(test_set_on_set_callback);
    RUN_TEST_CASE(test_set_deferred);
}
