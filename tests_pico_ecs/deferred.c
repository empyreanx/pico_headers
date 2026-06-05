#include "common.h"

// Destroys all entities starting with the front of the array
static ecs_ret_t destroy_system(ecs_t* ecs,
                                ecs_entity_t* entities,
                                size_t entity_count,
                                void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_destroy(ecs, entities[i]);

        if (ecs_is_ready(ecs, entities[i]))
            return -1;
    }

    return 0;
}

// TODO: make more elaborate
static ecs_ret_t remove_system(ecs_t* ecs,
                               ecs_entity_t* entities,
                               size_t entity_count,
                               void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_entity_t entity = entities[i];

        ecs_remove(ecs, entity, comp1);

        if (ecs_has(ecs, entity, comp1))
            return -1;
    }

    return 0;
}

static ecs_ret_t queue_add_system(ecs_t* ecs,
                                  ecs_entity_t* entities,
                                  size_t entity_count,
                                  void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)udata;

    for (size_t i = 0; i < MIN_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1);
    }

    return 0;
}

static ecs_ret_t queue_remove_system(ecs_t* ecs,
                                     ecs_entity_t* entities,
                                     size_t entity_count,
                                     void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)udata;

    for (size_t i  = 0; i < entity_count; i++)
    {
        ecs_entity_t entity = entities[i];
        ecs_remove(ecs, entity, comp1);
    }

    return ecs_get_system_entity_count(ecs, sys1);
}

static ecs_ret_t queue_destroy_system(ecs_t* ecs,
                                      ecs_entity_t* entities,
                                      size_t entity_count,
                                      void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)udata;

    for (size_t i  = 0; i < entity_count; i++)
    {
        ecs_entity_t entity = entities[i];
        ecs_destroy(ecs, entity);
    }

    return ecs_get_system_entity_count(ecs, sys1);
}

static comp_t set_then_destroy_data = { .used = true };
static bool   set_then_destroy_cb_called = false;

static void on_set_before_destroy(ecs_t* ecs,
                                  ecs_entity_t entity,
                                  ecs_comp_t comp,
                                  void* udata)
{
    (void)ecs;
    (void)entity;
    (void)comp;
    (void)udata;
    set_then_destroy_cb_called = true;
}

// Queues a set then a destroy for the same entity within one system tick.
// The deferred set must be skipped at flush time because the destroy marks
// the entity not-ready before the command queue is processed.
static ecs_ret_t set_then_destroy_system(ecs_t* ecs,
                                         ecs_entity_t* entities,
                                         size_t entity_count,
                                         void* udata)
{
    ecs_comp_t* comp_handle = (ecs_comp_t*)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        // Queue set first (entity is still ready at this point)
        ecs_set(ecs, entities[i], *comp_handle, &set_then_destroy_data);
        // Destroy immediately marks entity not-ready and queues CMD_DESTROY
        ecs_destroy(ecs, entities[i]);
    }

    return 0;
}

// =============================================================
// Deferred Suite: mutations performed during system iteration
// =============================================================

TEST_CASE(test_destroy_system)
{
    sys1 = ecs_define_system(ecs, destroy_system, NULL);
    ecs_require(ecs, sys1, comp1);
    ecs_require(ecs, sys1, comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1);
        ecs_add(ecs, entity, comp2);
        REQUIRE(ecs_is_ready(ecs, entity));
    }

    ecs_ret_t ret = ecs_run_system(ecs, sys1, 0);

    REQUIRE(ret == 0);

    return true;
}

TEST_CASE(test_remove_system)
{
    sys1 = ecs_define_system(ecs, remove_system, NULL);
    ecs_require(ecs, sys1, comp1);
    ecs_require(ecs, sys1, comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1);
        ecs_add(ecs, entity, comp2);
        REQUIRE(ecs_is_ready(ecs, entity));
    }

    ecs_ret_t ret = ecs_run_system(ecs, sys1, 0);

    REQUIRE(ret == 0);

    return true;
}

TEST_CASE(test_queue_add)
{
    sys1 = ecs_define_system(ecs, queue_add_system, NULL);
    ecs_require(ecs, sys1, comp1);

    ecs_run_system(ecs, sys1, 0);
    size_t count = ecs_get_system_entity_count(ecs, sys1);

    REQUIRE(count == MIN_ENTITIES);
    return true;
}

TEST_CASE(test_queue_remove)
{
    sys1 = ecs_define_system(ecs, queue_remove_system, NULL);
    ecs_require(ecs, sys1, comp1);

    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_add(ecs, ecs_create(ecs), comp1);
    }

    size_t inner_count = ecs_run_system(ecs, sys1, 0);
    size_t outer_count = ecs_get_system_entity_count(ecs, sys1);

    REQUIRE(inner_count == MAX_ENTITIES);
    REQUIRE(outer_count == 0);

    return true;
}

TEST_CASE(test_queue_destroy)
{
    sys1 = ecs_define_system(ecs, queue_destroy_system, NULL);
    ecs_require(ecs, sys1, comp1);

    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_add(ecs, ecs_create(ecs), comp1);
    }

    size_t inner_count = ecs_run_system(ecs, sys1, 0);
    size_t outer_count = ecs_get_system_entity_count(ecs, sys1);

    REQUIRE(inner_count == MAX_ENTITIES);
    REQUIRE(outer_count == 0);

    return true;
}

TEST_CASE(test_queue_set_dead_entity)
{
    set_then_destroy_cb_called = false;

    ecs_comp_t comp_cb = ecs_define_component(ecs, sizeof(comp_t), &(ecs_comp_desc_t)
    {
        .on_set_cb = on_set_before_destroy
    });
    sys1 = ecs_define_system(ecs, set_then_destroy_system, &(ecs_sys_desc_t)
    {
        .udata = &comp_cb
    });
    ecs_require(ecs, sys1, comp_cb);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_cb);

    ecs_run_system(ecs, sys1, 0);

    // The entity should have been destroyed
    REQUIRE(!ecs_is_ready(ecs, entity));

    // The deferred set was skipped because the entity was not ready at flush time
    REQUIRE(!set_then_destroy_cb_called);

    return true;
}

TEST_SUITE(suite_deferred)
{
    RUN_TEST_CASE(test_destroy_system);
    RUN_TEST_CASE(test_remove_system);
    RUN_TEST_CASE(test_queue_add);
    RUN_TEST_CASE(test_queue_remove);
    RUN_TEST_CASE(test_queue_destroy);
    RUN_TEST_CASE(test_queue_set_dead_entity);
}
