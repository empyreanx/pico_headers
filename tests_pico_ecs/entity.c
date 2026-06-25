#include "common.h"

// --- Helpers ---

static void comp_on_add(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, const void* args, void* udata)
{
    (void)args;
    (void)udata;

    comp_t* comp_ptr = ecs_get(ecs, entity, comp);
    comp_ptr->used = true;
}

// Constructor that initializes the component from the args passed to ecs_add
static void comp_on_add_args(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, const void* args, void* udata)
{
    (void)udata;

    comp_t* comp_ptr = ecs_get(ecs, entity, comp);

    if (args)
        *comp_ptr = *(const comp_t*)args;
}

static void comp_on_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* udata)
{
    (void)udata;

    comp_t* comp_ptr = ecs_get(ecs, entity, comp);
    comp_ptr->used = false;
}

// Destructor used by the destroy test. Because on_remove is now delivered on
// ecs_dispatch, an entity destroyed via ecs_destroy is no longer live when this
// runs, so it must not dereference the entity; it just records that it fired.
static bool destructor_ran = false;

static void comp_on_remove_flag(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* udata)
{
    (void)ecs;
    (void)entity;
    (void)comp;
    (void)udata;

    destructor_ran = true;
}

static ecs_ret_t entity_dummy_system(ecs_t* ecs,
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

// =============================================================
// suite_entity: entity lifecycle and component add/remove
// =============================================================

TEST_CASE(test_create_destroy)
{
    // Create an entity
    ecs_entity_t entity = ecs_create(ecs);

    // Verify the entity is alive
    REQUIRE(ecs_is_ready(ecs, entity));

    // Destroy the component
    ecs_destroy(ecs, entity);

    // Verify that the component is no longer alive
    REQUIRE(!ecs_is_ready(ecs, entity));

    return true;
}

TEST_CASE(test_add_remove)
{
    ecs_entity_t entity = ecs_create(ecs);

    // Initially entity should have no components
    REQUIRE(!ecs_has(ecs, entity, comp1));
    REQUIRE(!ecs_has(ecs, entity, comp2));

    // Add a component
    ecs_add(ecs, entity, comp1, NULL);

    // Verify that the entity has one component and not the other
    REQUIRE(ecs_has(ecs, entity, comp1));
    REQUIRE(!ecs_has(ecs, entity, comp2));

    // Add a second component to the entity
    ecs_add(ecs, entity, comp2, NULL);

    // Verify that the entity has both components
    REQUIRE(ecs_has(ecs, entity, comp1));
    REQUIRE(ecs_has(ecs, entity, comp2));

    // Remove the first component
    ecs_remove(ecs, entity, comp1);

    // Verify that the first component has been removed
    REQUIRE(!ecs_has(ecs, entity, comp1));
    REQUIRE(ecs_has(ecs, entity, comp2));

    // Remove the second component
    ecs_remove(ecs, entity, comp2);

    // Verify that both components have been removed
    REQUIRE(!ecs_has(ecs, entity, comp1));
    REQUIRE(!ecs_has(ecs, entity, comp2));

    return true;
}

TEST_CASE(test_reset)
{
    sys1 = ecs_define_system(ecs, entity_dummy_system, NULL);
    ecs_require(ecs, sys1, comp1);

    ecs_entity_t entities[MAX_ENTITIES];
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i] = ecs_create(ecs);
        ecs_add(ecs, entities[i], comp1, NULL);
        ecs_add(ecs, entities[i], comp2, NULL);
        REQUIRE(ecs_is_ready(ecs, entities[i]));
    }

    REQUIRE(ecs_get_entity_count(ecs, sys1) == MAX_ENTITIES);

    ecs_reset(ecs);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        REQUIRE(!ecs_is_ready(ecs, entities[i]));
    }

    REQUIRE(ecs_get_entity_count(ecs, sys1) == 0);

    return true;
}

TEST_CASE(test_add_idempotent)
{
    sys1 = ecs_define_system(ecs, entity_dummy_system, NULL);
    ecs_require(ecs, sys1, comp1);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);

    REQUIRE(ecs_has(ecs, entity, comp1));
    REQUIRE(ecs_get_entity_count(ecs, sys1) == 1);

    // Double-add should be a no-op
    ecs_add(ecs, entity, comp1, NULL);

    REQUIRE(ecs_has(ecs, entity, comp1));
    REQUIRE(ecs_get_entity_count(ecs, sys1) == 1);

    return true;
}

TEST_CASE(test_remove_idempotent)
{
    ecs_entity_t entity = ecs_create(ecs);

    // Removing a component the entity doesn't have is a no-op
    REQUIRE(!ecs_has(ecs, entity, comp1));
    ecs_remove(ecs, entity, comp1);
    REQUIRE(!ecs_has(ecs, entity, comp1));

    // Add then double-remove
    ecs_add(ecs, entity, comp1, NULL);
    REQUIRE(ecs_has(ecs, entity, comp1));
    ecs_remove(ecs, entity, comp1);
    REQUIRE(!ecs_has(ecs, entity, comp1));
    ecs_remove(ecs, entity, comp1);
    REQUIRE(!ecs_has(ecs, entity, comp1));

    return true;
}

TEST_CASE(test_component_zeroed)
{
    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);

    comp_t* comp = ecs_get(ecs, entity, comp1);
    REQUIRE(!comp->used);

    return true;
}

TEST_SUITE(suite_entity)
{
    RUN_TEST_CASE(test_create_destroy);
    RUN_TEST_CASE(test_add_remove);
    RUN_TEST_CASE(test_reset);
    RUN_TEST_CASE(test_add_idempotent);
    RUN_TEST_CASE(test_remove_idempotent);
    RUN_TEST_CASE(test_component_zeroed);
}

// =============================================================
// suite_components: component add/remove callbacks and destructor
// =============================================================

TEST_CASE(test_on_add)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), NULL);
    ecs_on_add(ecs, comp_type, comp_on_add, false);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type, NULL);

    // on_add is delivered as an event; it runs on dispatch, not during ecs_add
    REQUIRE(!((comp_t*)ecs_get(ecs, entity, comp_type))->used);
    ecs_dispatch(ecs);

    comp_t* comp = ecs_get(ecs, entity, comp_type);
    REQUIRE(comp->used);

    return true;
}

TEST_CASE(test_on_add_args)
{
    // args_size is required so the args are copied and survive until dispatch
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), &(ecs_comp_desc_t)
    {
        .args_size = sizeof(comp_t)
    });
    ecs_on_add(ecs, comp_type, comp_on_add_args, false);

    ecs_entity_t entity = ecs_create(ecs);

    // The args passed to ecs_add are forwarded to the on_add constructor
    comp_t init = { .used = true };
    ecs_add(ecs, entity, comp_type, &init);
    ecs_dispatch(ecs);

    comp_t* comp = ecs_get(ecs, entity, comp_type);
    REQUIRE(comp->used);

    return true;
}

TEST_CASE(test_on_remove)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), NULL);
    ecs_on_add(ecs, comp_type, comp_on_add, false);
    ecs_on_remove(ecs, comp_type, comp_on_remove, false);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type, NULL);
    comp_t* comp = ecs_get(ecs, entity, comp_type);
    ecs_remove(ecs, entity, comp_type);

    // on_add then on_remove are delivered in order on dispatch
    ecs_dispatch(ecs);

    REQUIRE(!comp->used);

    return true;
}

TEST_CASE(test_destructor_destroy)
{
    destructor_ran = false;

    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), NULL);
    ecs_on_remove(ecs, comp_type, comp_on_remove_flag, false);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type, NULL);

    ecs_destroy(ecs, entity);

    // The destructor is queued by ecs_destroy and fires on dispatch
    ecs_dispatch(ecs);

    REQUIRE(!ecs_is_ready(ecs, entity));
    REQUIRE(destructor_ran);

    return true;
}

TEST_CASE(test_default_value)
{
    // A component with no default value is zeroed on add
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), &(ecs_comp_desc_t)
    {
        .default_value = &(comp_t){ .used = true }
    });

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type, NULL);
    comp_t* comp = ecs_get(ecs, entity, comp_type);

    // The component should be initialized to the default rather than zeroed
    REQUIRE(comp->used);

    return true;
}

TEST_CASE(test_default_value_per_add)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), &(ecs_comp_desc_t)
    {
        .default_value = &(comp_t){ .used = true }
    });

    // First entity gets the default, then we mutate its component
    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_add(ecs, entity1, comp_type, NULL);
    comp_t* comp1_ptr = ecs_get(ecs, entity1, comp_type);
    REQUIRE(comp1_ptr->used);
    comp1_ptr->used = false;

    // A second entity should still receive a fresh copy of the default,
    // proving the stored default is copied and not shared/mutated
    ecs_entity_t entity2 = ecs_create(ecs);
    ecs_add(ecs, entity2, comp_type, NULL);
    comp_t* comp2_ptr = ecs_get(ecs, entity2, comp_type);
    REQUIRE(comp2_ptr->used);

    return true;
}

// A synchronous on_add runs during ecs_add, acting as a real constructor
TEST_CASE(test_on_add_sync)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), NULL);
    ecs_on_add(ecs, comp_type, comp_on_add, true); // synchronous

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type, NULL);

    // The callback already ran (no dispatch), so the component is initialized
    REQUIRE(((comp_t*)ecs_get(ecs, entity, comp_type))->used);

    return true;
}

// A synchronous on_remove runs during ecs_remove, while the entity is live
TEST_CASE(test_on_remove_sync)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), NULL);
    ecs_on_remove(ecs, comp_type, comp_on_remove, true); // synchronous

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type, NULL);
    comp_t* comp = ecs_get(ecs, entity, comp_type);
    comp->used = true;

    ecs_remove(ecs, entity, comp_type);

    // The callback already ran (no dispatch) and cleared the component
    REQUIRE(!comp->used);

    return true;
}

TEST_SUITE(suite_components)
{
    RUN_TEST_CASE(test_on_add);
    RUN_TEST_CASE(test_on_add_args);
    RUN_TEST_CASE(test_on_remove);
    RUN_TEST_CASE(test_destructor_destroy);
    RUN_TEST_CASE(test_on_add_sync);
    RUN_TEST_CASE(test_on_remove_sync);
    RUN_TEST_CASE(test_default_value);
    RUN_TEST_CASE(test_default_value_per_add);
}
