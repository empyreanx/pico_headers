#include "common.h"

// --- Helpers ---

static void comp_on_add(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* udata)
{
    (void)udata;

    comp_t* comp_ptr = ecs_get(ecs, entity, comp);
    comp_ptr->used = true;
}

static void comp_on_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* udata)
{
    (void)udata;

    comp_t* comp_ptr = ecs_get(ecs, entity, comp);
    comp_ptr->used = false;
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
    ecs_add(ecs, entity, comp1);

    // Verify that the entity has one component and not the other
    REQUIRE(ecs_has(ecs, entity, comp1));
    REQUIRE(!ecs_has(ecs, entity, comp2));

    // Add a second component to the entity
    ecs_add(ecs, entity, comp2);

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
        ecs_add(ecs, entities[i], comp1);
        ecs_add(ecs, entities[i], comp2);
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
    ecs_add(ecs, entity, comp1);

    REQUIRE(ecs_has(ecs, entity, comp1));
    REQUIRE(ecs_get_entity_count(ecs, sys1) == 1);

    // Double-add should be a no-op
    ecs_add(ecs, entity, comp1);

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
    ecs_add(ecs, entity, comp1);
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
    ecs_add(ecs, entity, comp1);

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
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), &(ecs_comp_desc_t)
    {
        .on_add_cb = comp_on_add
    });

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type);
    comp_t* comp = ecs_get(ecs, entity, comp_type);

    REQUIRE(comp->used);

    return true;
}

TEST_CASE(test_on_remove)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), &(ecs_comp_desc_t)
    {
        .on_add_cb = comp_on_add,
        .on_remove_cb = comp_on_remove
    });

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type);
    comp_t* comp = ecs_get(ecs, entity, comp_type);
    ecs_remove(ecs, entity, comp_type);

    REQUIRE(!comp->used);

    return true;
}

TEST_CASE(test_destructor_destroy)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), &(ecs_comp_desc_t)
    {
        .on_add_cb = comp_on_add,
        .on_remove_cb = comp_on_remove
    });

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp_type);
    comp_t* comp = ecs_get(ecs, entity, comp_type);
    ecs_destroy(ecs, entity);

    REQUIRE(!ecs_is_ready(ecs, entity));

    //WARNING: We assume memory has not been reclaimed
    REQUIRE(!comp->used);

    return true;
}

TEST_SUITE(suite_components)
{
    RUN_TEST_CASE(test_on_add);
    RUN_TEST_CASE(test_on_remove);
    RUN_TEST_CASE(test_destructor_destroy);
}
