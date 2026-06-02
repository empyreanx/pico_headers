#include "common.h"

// --- Helpers (suite_systems) ---

static ecs_ret_t dummy_system(ecs_t* ecs,
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

// Turns on the `used` flag on the components of matching entities
static ecs_ret_t comp_system(ecs_t* ecs,
                             ecs_entity_t* entities,
                             size_t entity_count,
                             void* udata)
{
	(void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_entity_t entity = entities[i];

        if (ecs_has(ecs, entity, comp1))
        {
            comp_t* comp = ecs_get(ecs, entity, comp1);
            comp->used = true;
        }

        if (ecs_has(ecs, entity, comp2))
        {
            comp_t* comp = ecs_get(ecs, entity, comp2);
            comp->used = true;
        }
    }

    return 0;
}

static ecs_ret_t empty_system(ecs_t* ecs,
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

static bool added = false;
static bool removed = false;

static void on_add(ecs_t* ecs, ecs_entity_t entity, void* udata)
{
    (void)ecs;
    (void)entity;
    (void)udata;
    added = true;
}

static void on_remove(ecs_t* ecs, ecs_entity_t entity, void* udata)
{
    (void)ecs;
    (void)entity;
    (void)udata;
    removed = true;
}

static ecs_ret_t mask_test_system(ecs_t* ecs,
                                  ecs_entity_t* entities,
                                  size_t entity_count,
                                  void* udata)
{
    (void)ecs;
    (void)entities;
    (void)entity_count;

    bool* run = udata;
    *run = true;

    return 0;
}

static ecs_ret_t ret_system(ecs_t* ecs,
                            ecs_entity_t* entities,
                            size_t entity_count,
                            void* udata)
{
    (void)ecs;
    (void)entities;
    (void)entity_count;
    (void)udata;
    return 42;
}

static ecs_ret_t alt_ret_system(ecs_t* ecs,
                                ecs_entity_t* entities,
                                size_t entity_count,
                                void* udata)
{
    (void)ecs;
    (void)entities;
    (void)entity_count;
    (void)udata;
    return 24;
}

// --- Helpers (suite_exclude) ---

typedef struct
{
    ecs_entity_t entity;
    size_t count;
    size_t add_count;
    size_t remove_count;
} exclude_sys_state_t;

static ecs_ret_t exclude_system(ecs_t* ecs,
                                ecs_entity_t* entities,
                                size_t entity_count,
                                void* udata)
{
    (void)ecs;
    (void)entity_count;

    exclude_sys_state_t* state = (exclude_sys_state_t*)udata;

    state->count = entity_count;

    if (entity_count > 0)
    {
        state->entity = entities[0];
    }

    return 0;
}

static void exclude_add_cb(ecs_t* ecs,
                           ecs_entity_t entity,
                           void* udata)
{
    (void)ecs;
    (void)entity;

    exclude_sys_state_t* state = (exclude_sys_state_t*)udata;

    state->add_count++;
}

static void exclude_remove_cb(ecs_t* ecs,
                              ecs_entity_t entity,
                              void* udata)
{
    (void)ecs;
    (void)entity;

    exclude_sys_state_t* state = (exclude_sys_state_t*)udata;

    state->remove_count++;
}

static exclude_sys_state_t state1;
static exclude_sys_state_t state2;

static ecs_ret_t remove_exclude_system(ecs_t* ecs,
                                       ecs_entity_t* entities,
                                       size_t entity_count,
                                       void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_remove(ecs, entities[i], comp1);
    }

    return 0;
}

static ecs_ret_t add_exclude_system(ecs_t* ecs,
                                    ecs_entity_t* entities,
                                    size_t entity_count,
                                    void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_add(ecs, entities[i], comp1);
    }

    return 0;
}

// =============================================================
// suite_systems: system entity tracking, enable/disable, mask,
//                callbacks, and udata
// =============================================================

// Tests adding components to systems
TEST_CASE(test_add_systems)
{
    // Set up systems
    sys1 = ecs_define_system(ecs, 0, dummy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);

    sys2 = ecs_define_system(ecs, 0, dummy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys2, comp1);
    ecs_require_component(ecs, sys2, comp2);

    // Create a couple entities
    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_entity_t entity2 = ecs_create(ecs);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 0);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 0);

    // Add a component to entity 1
    ecs_add(ecs, entity1, comp1);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 1);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 0);

    // Add components to entity 2
    ecs_add(ecs, entity2, comp1);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 2);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 0);

    ecs_add(ecs, entity2, comp2);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 2);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 1);

    return true;
}

TEST_CASE(test_remove)
{
    // Set up system
    sys1 = ecs_define_system(ecs, 0, dummy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1); // Entity must have at least
    ecs_require_component(ecs, sys1, comp2); // component 1 and 2 to match

    // Create an entity
    ecs_entity_t entity1 = ecs_create(ecs);

    // Add components to the entity
    ecs_add(ecs, entity1, comp1);
    ecs_add(ecs, entity1, comp2);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 1);

    // Remove component
    REQUIRE(ecs_has(ecs, entity1, comp2));
    ecs_remove(ecs, entity1, comp2);
    REQUIRE(!ecs_has(ecs, entity1, comp2));

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 0);

    ecs_entity_t entity2 = ecs_create(ecs);

    ecs_add(ecs, entity2, comp1);
    ecs_add(ecs, entity2, comp2);
    ecs_add(ecs, entity2, comp3);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 1);

    REQUIRE(ecs_has(ecs, entity2, comp3));
    ecs_remove(ecs, entity2, comp3);
    REQUIRE(!ecs_has(ecs, entity2, comp3));

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 1);

    return true;
}

TEST_CASE(test_destroy)
{
    // Set up system
    sys1 = ecs_define_system(ecs, 0, dummy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);
    ecs_require_component(ecs, sys1, comp2);

    // Create an entity
    ecs_entity_t entity = ecs_create(ecs);

    // Add components to entity
    ecs_add(ecs, entity, comp1);
    ecs_add(ecs, entity, comp2);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 1);

    // Destroy entity
    ecs_destroy(ecs, entity);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 0);

    // Verify entity is inactive
    REQUIRE(!ecs_is_ready(ecs, entity));

    return true;
}

// Tests system enable/disable
TEST_CASE(test_enable_disable)
{
    // Set up system
    sys1 = ecs_define_system(ecs, 0, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);

    // Create entity
    ecs_entity_t entity = ecs_create(ecs);

    // Add component to entity
    ecs_add(ecs, entity, comp1);
    comp_t* comp = ecs_get(ecs, entity, comp1);
    comp->used = false;

    // Run system
    ecs_run_system(ecs, sys1, 0);

    // Verify that entity was processed by system
    REQUIRE(comp->used);

    // Reset component
    comp->used = false;

    // Disable system
    ecs_disable_system(ecs, sys1);

    // Run system
    ecs_run_system(ecs, sys1, 0);

    // Verify that the entity was not processed by the system
    REQUIRE(!comp->used);

    // Enable system
    ecs_enable_system(ecs, sys1);

    // Run system again
    ecs_run_system(ecs, sys1, 0);

    // Verify that entity was processed by the system
    REQUIRE(comp->used);

    return true;
}

TEST_CASE(test_system_mask)
{
    bool run = false;

    sys1 = ecs_define_system(ecs, (1 << 0) | (1 << 1), mask_test_system, NULL, NULL, &run);

    ecs_run_system(ecs, sys1, 0);

    REQUIRE(!run);

    ecs_run_system(ecs, sys1, (1 << 3));

    REQUIRE(!run);

    ecs_run_system(ecs, sys1, (1 << 1));

    REQUIRE(run);

    run = false;

    ecs_run_system(ecs, sys1, (1 << 0) | (1 << 1));

    REQUIRE(run);

    run = false;

    ecs_run_system(ecs, sys1, (ecs_mask_t)-1);

    REQUIRE(run);

    return true;
}

// Tests system add/remove callbacks
TEST_CASE(test_add_remove_callbacks)
{
    added = false;
    removed = false;

    sys1 = ecs_define_system(ecs, 0, empty_system, on_add, on_remove, NULL);

    ecs_require_component(ecs, sys1, comp1);

    ecs_run_system(ecs, sys1, 0);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1);
    ecs_destroy(ecs, entity);

    REQUIRE(added);
    REQUIRE(removed);

    return true;
}

// Tests replacing system callbacks on the fly
TEST_CASE(test_set_system_callbacks)
{
    added = false;
    removed = false;

    sys1 = ecs_define_system(ecs, 0, ret_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);

    // Test initial system callback works
    ecs_ret_t ret = ecs_run_system(ecs, sys1, 0);
    REQUIRE(ret == 42);

    // Change system callback and test
    ecs_set_system_callbacks(ecs, sys1, alt_ret_system, on_add, on_remove);
    ret = ecs_run_system(ecs, sys1, 0);
    REQUIRE(ret == 24);

    // Create entity to trigger callbacks
    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1);
    REQUIRE(added);

    ecs_destroy(ecs, entity);
    REQUIRE(removed);

    return true;
}

TEST_CASE(test_system_udata)
{
    int test_value = 42;
    sys1 = ecs_define_system(ecs, 0, empty_system, NULL, NULL, NULL);

    // Test setting udata
    ecs_set_system_udata(ecs, sys1, &test_value);

    // Test getting udata
    void* retrieved_udata = ecs_get_system_udata(ecs, sys1);
    REQUIRE(retrieved_udata == &test_value);
    REQUIRE(*(int*)retrieved_udata == 42);

    // Test overwriting udata
    int new_value = 24;
    ecs_set_system_udata(ecs, sys1, &new_value);
    retrieved_udata = ecs_get_system_udata(ecs, sys1);
    REQUIRE(retrieved_udata == &new_value);
    REQUIRE(*(int*)retrieved_udata == 24);

    return true;
}

TEST_CASE(test_run_systems)
{
    sys1 = ecs_define_system(ecs, 0, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);

    sys2 = ecs_define_system(ecs, 0, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys2, comp2);

    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_add(ecs, entity1, comp1);

    ecs_entity_t entity2 = ecs_create(ecs);
    ecs_add(ecs, entity2, comp2);

    comp_t* c1 = ecs_get(ecs, entity1, comp1);
    comp_t* c2 = ecs_get(ecs, entity2, comp2);

    ecs_run_systems(ecs, 0);

    REQUIRE(c1->used);
    REQUIRE(c2->used);

    return true;
}

TEST_CASE(test_get_set_system_mask)
{
    sys1 = ecs_define_system(ecs, (1 << 2), dummy_system, NULL, NULL, NULL);

    REQUIRE(ecs_get_system_mask(ecs, sys1) == (ecs_mask_t)(1 << 2));

    ecs_set_system_mask(ecs, sys1, (1 << 3) | (1 << 4));

    REQUIRE(ecs_get_system_mask(ecs, sys1) == (ecs_mask_t)((1 << 3) | (1 << 4)));

    // Zero mask matches all categories
    ecs_set_system_mask(ecs, sys1, 0);

    REQUIRE(ecs_get_system_mask(ecs, sys1) == 0);

    return true;
}

TEST_SUITE(suite_systems)
{
    RUN_TEST_CASE(test_add_systems);
    RUN_TEST_CASE(test_remove);
    RUN_TEST_CASE(test_destroy);
    RUN_TEST_CASE(test_enable_disable);
    RUN_TEST_CASE(test_system_mask);
    RUN_TEST_CASE(test_add_remove_callbacks);
    RUN_TEST_CASE(test_set_system_callbacks);
    RUN_TEST_CASE(test_system_udata);
    RUN_TEST_CASE(test_run_systems);
    RUN_TEST_CASE(test_get_set_system_mask);
}

// =============================================================
// suite_exclude: component exclusion filters on systems
// =============================================================

TEST_CASE(test_exclude)
{
    memset(&state1, 0, sizeof(exclude_sys_state_t));
    memset(&state2, 0, sizeof(exclude_sys_state_t));

    ecs_system_t sys1 = ecs_define_system(ecs, 0,
                                            exclude_system,
                                            exclude_add_cb,
                                            exclude_remove_cb,
                                            &state1);

    ecs_require_component(ecs, sys1, comp2);
    ecs_exclude_component(ecs, sys1, comp1);

    ecs_system_t sys2 = ecs_define_system(ecs, 0,
                                            exclude_system,
                                            exclude_add_cb,
                                            exclude_remove_cb,
                                            &state2);

    ecs_require_component(ecs, sys2, comp2);

    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_add(ecs, entity1, comp1);
    ecs_add(ecs, entity1, comp2);

    ecs_entity_t entity2 = ecs_create(ecs);
    ecs_add(ecs, entity2, comp2);

    ecs_run_system(ecs, sys1, 0);
    ecs_run_system(ecs, sys2, 0);

    REQUIRE(state1.count == 1);
    REQUIRE(state1.entity.id == entity2.id);
    REQUIRE(state1.add_count == 1);
    REQUIRE(state1.remove_count == 0);
    REQUIRE(state2.count == 2);
    REQUIRE(state2.entity.id == entity1.id);
    REQUIRE(state2.add_count == 2);
    REQUIRE(state2.remove_count == 0);

    memset(&state1, 0, sizeof(exclude_sys_state_t));
    memset(&state2, 0, sizeof(exclude_sys_state_t));

    // Removing comp1 from entity1 causes it to be added to the system
    ecs_remove(ecs, entity1, comp1);

    ecs_run_system(ecs, sys1, 0);
    ecs_run_system(ecs, sys2, 0);

    REQUIRE(state1.count == 2);
    REQUIRE(state1.entity.id == entity2.id);
    REQUIRE(state1.add_count == 1);
    REQUIRE(state1.remove_count == 0);

    REQUIRE(state2.count == 2);
    REQUIRE(state2.entity.id == entity1.id);
    REQUIRE(state2.add_count == 0);
    REQUIRE(state2.remove_count == 0);

    memset(&state1, 0, sizeof(exclude_sys_state_t));
    memset(&state2, 0, sizeof(exclude_sys_state_t));

    // Adding comp1 to entity2 causes it to be removed from the system
    ecs_add(ecs, entity2, comp1);

    ecs_run_system(ecs, sys1, 0);
    ecs_run_system(ecs, sys2, 0);

    REQUIRE(state1.count == 1);
    REQUIRE(state1.entity.id == entity1.id);
    REQUIRE(state1.add_count == 0);
    REQUIRE(state1.remove_count == 1);

    REQUIRE(state2.count == 2);
    REQUIRE(state2.entity.id == entity1.id);
    REQUIRE(state2.add_count == 0);
    REQUIRE(state2.remove_count == 0);

    return true;
}

TEST_CASE(test_exclude_remove_system)
{
    ecs_system_t sys1 = ecs_define_system(ecs, 0,
                                          remove_exclude_system,
                                          NULL,
                                          NULL,
                                          NULL);

    ecs_require_component(ecs, sys1, comp2);
    ecs_exclude_component(ecs, sys1, comp1);

    ecs_system_t sys2 = ecs_define_system(ecs, 0,
                                          remove_exclude_system,
                                          NULL,
                                          NULL,
                                          NULL);

    ecs_require_component(ecs, sys2, comp2);

    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_add(ecs, entity1, comp1);
    ecs_add(ecs, entity1, comp2);

    ecs_entity_t entity2 = ecs_create(ecs);
    ecs_add(ecs, entity2, comp2);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 1);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 2);

    ecs_run_system(ecs, sys2, 0);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 2);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 2);

    return true;
}

TEST_CASE(test_exclude_add_system)
{
    ecs_system_t sys1 = ecs_define_system(ecs, 0,
                                          add_exclude_system,
                                          NULL,
                                          NULL,
                                          NULL);

    ecs_require_component(ecs, sys1, comp2);
    ecs_exclude_component(ecs, sys1, comp1);

    ecs_system_t sys2 = ecs_define_system(ecs, 0,
                                          add_exclude_system,
                                          NULL,
                                          NULL,
                                          NULL);

    ecs_require_component(ecs, sys2, comp2);

    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_add(ecs, entity1, comp1);
    ecs_add(ecs, entity1, comp2);

    ecs_entity_t entity2 = ecs_create(ecs);
    ecs_add(ecs, entity2, comp2);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 1);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 2);

    ecs_run_system(ecs, sys2, 0);

    REQUIRE(ecs_get_system_entity_count(ecs, sys1) == 0);
    REQUIRE(ecs_get_system_entity_count(ecs, sys2) == 2);

    return true;
}

TEST_SUITE(suite_exclude)
{
    RUN_TEST_CASE(test_exclude);
    RUN_TEST_CASE(test_exclude_remove_system);
    RUN_TEST_CASE(test_exclude_add_system);
}
