#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <assert.h>
#include <stdio.h>

#define MIN_ENTITIES (1 * 1024)
#define MAX_ENTITIES (8 * 1024)

ecs_t* ecs = NULL;
ecs_comp_t comp1;
ecs_comp_t comp2;

ecs_system_t sys1;
ecs_system_t sys2;

typedef struct
{
    bool used;
} comp_t;

void setup()
{
    ecs = ecs_new(MIN_ENTITIES, NULL);
    comp1 = ecs_define_component(ecs, sizeof(comp_t), NULL, NULL);
    comp2 = ecs_define_component(ecs, sizeof(comp_t), NULL, NULL);
}

void teardown()
{
    ecs_free(ecs);
    ecs = NULL;
}

TEST_CASE(test_reset)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1, NULL);
        ecs_add(ecs, entity, comp2, NULL);
        REQUIRE(ecs_is_ready(ecs, entity));
    }

    ecs_reset(ecs);

    return true;
}

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

exclude_sys_state_t state1;
exclude_sys_state_t state2;

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
    ecs_add(ecs, entity1, comp1, NULL);
    ecs_add(ecs, entity1, comp2, NULL);

    ecs_entity_t entity2 = ecs_create(ecs);
    ecs_add(ecs, entity2, comp2, NULL);

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

    // Removing comp1 from entity1 causes it to be added to the system
    ecs_remove(ecs, entity1, comp1);

    ecs_run_system(ecs, sys1, 0);
    ecs_run_system(ecs, sys2, 0);

    REQUIRE(state1.count == 2);
    REQUIRE(state1.entity.id == entity2.id);
    REQUIRE(state1.add_count == 2);
    REQUIRE(state1.remove_count == 0);

    REQUIRE(state2.count == 2);
    REQUIRE(state2.entity.id == entity1.id);
    REQUIRE(state2.add_count == 2);
    REQUIRE(state2.remove_count == 0);

    // Adding comp1 to entity2 causes it to be removed from the system
    ecs_add(ecs, entity2, comp1, NULL);

    ecs_run_system(ecs, sys1, 0);
    ecs_run_system(ecs, sys2, 0);

    REQUIRE(state1.count == 1);
    REQUIRE(state1.entity.id == entity1.id);
    REQUIRE(state1.add_count == 2);
    REQUIRE(state1.remove_count == 1);

    REQUIRE(state2.count == 2);
    REQUIRE(state2.entity.id == entity1.id);
    REQUIRE(state2.add_count == 2);
    REQUIRE(state2.remove_count == 0);

    return true;
}

typedef struct
{
    bool used;
} test_args_t;

void constructor(ecs_t* ecs, ecs_entity_t entity, void* ptr, void* args)
{
    (void)ecs;
    (void)entity;
    (void)args;

    test_args_t* test_args = args;

    comp_t* comp = ptr;
    comp->used = test_args->used;
}

TEST_CASE(test_constructor)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), constructor, NULL);

    ecs_entity_t entity = ecs_create(ecs);
    comp_t* comp = ecs_add(ecs, entity, comp_type, &(test_args_t){ true });

    REQUIRE(comp->used);

    return true;
}

void destructor(ecs_t* ecs, ecs_entity_t entity, void* ptr)
{
    (void)ecs;
    (void)entity;

    comp_t* comp = ptr;
    comp->used = false;
}

TEST_CASE(test_destructor_remove)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), constructor, destructor);

    ecs_entity_t entity = ecs_create(ecs);
    comp_t* comp = ecs_add(ecs, entity, comp_type, &(test_args_t){ true });
    ecs_remove(ecs, entity, comp_type);

    REQUIRE(!comp->used);

    return true;
}

TEST_CASE(test_destructor_destroy)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), constructor, destructor);

    ecs_entity_t entity = ecs_create(ecs);
    comp_t* comp = ecs_add(ecs, entity, comp_type, &(test_args_t){ true });
    ecs_destroy(ecs, entity);

    REQUIRE(!ecs_is_ready(ecs, entity));

    //WARNING: We assume memory has not been reclaimed
    REQUIRE(!comp->used);

    return true;
}

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

static struct
{
    ecs_entity_t entity;
    size_t count;
} remove_sys_state;

// Copies the entity_count, should be 1
static ecs_ret_t remove_comp_system(ecs_t* ecs,
                                    ecs_entity_t* entities,
                                    size_t entity_count,
                                    void* udata)
{
    (void)ecs;
    (void)entities;
    (void)entity_count;
    (void)udata;

    remove_sys_state.count = entity_count;

    return 0;
}

// Test
TEST_CASE(test_remove_comp_system)
{
    ecs_system_t sys =ecs_define_system(ecs, 0, remove_comp_system, NULL, NULL, NULL);

    ecs_require_component(ecs, sys, comp2);
    ecs_require_component(ecs, sys, comp1);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);
    ecs_add(ecs, entity, comp2, NULL);

    ecs_remove(ecs, entity, comp1);

    ecs_run_system(ecs, sys, 0);

    REQUIRE(remove_sys_state.count == 1);

    return true;
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

// Tests adding components to systems
TEST_CASE(test_add_systems)
{
    // Set up systems
    sys1 =ecs_define_system(ecs, 0,  comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);

    sys2 =ecs_define_system(ecs, 0,  comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys2, comp1);
    ecs_require_component(ecs, sys2, comp2);

    // Create a couple entities
    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_entity_t entity2 = ecs_create(ecs);

    // Add a component to entity 1
    comp_t* comp_one = ecs_add(ecs, entity1, comp1, NULL);
    comp_one->used = false;

    // Run sys1
    ecs_run_system(ecs, sys1, 0);

    // Confirm that entity 1 was process by the system
    REQUIRE(comp_one->used);

    // Add components to entity 2
    comp_one = ecs_add(ecs, entity2, comp1, NULL);
    comp_one->used = false;

    comp_t* comp_two = ecs_add(ecs, entity2, comp2, NULL);
    comp_two->used = false;

    // Run sys2
    ecs_run_system(ecs, sys2, 0);

    // Confirm that both entities we processed by the system
    REQUIRE(comp_one->used);
    REQUIRE(comp_two->used);

    return true;
}

TEST_CASE(test_remove)
{
    // Set up system
    sys1 =ecs_define_system(ecs, 0, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1); // Entity must have at least
    ecs_require_component(ecs, sys1, comp2); // component 1 and 2 to match

    // Create an entity
    ecs_entity_t entity = ecs_create(ecs);

    // Add components to the entity
    comp_t* comp_one = ecs_add(ecs, entity, comp1, NULL);
    comp_t* comp_two = ecs_add(ecs, entity, comp2, NULL);

    comp_one->used = false;
    comp_two->used = false;

    // Run system
    ecs_run_system(ecs, sys1, 0);

    // Verify that the system processed the entity
    REQUIRE(comp_one->used);
    REQUIRE(comp_two->used);

    // Reset components
    comp_one->used = false;
    comp_two->used = false;

    // Remove component 2
    ecs_remove(ecs, entity, comp2);

    // Run system again
    ecs_run_system(ecs, sys1, 0);

    // Verify that the entity was remove by the system
    REQUIRE(comp_one->used);
    REQUIRE(!comp_two->used);

    return true;
}

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

TEST_CASE(test_destroy_system)
{
    sys1 = ecs_define_system(ecs, 0, destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);
    ecs_require_component(ecs, sys1, comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1, NULL);
        ecs_add(ecs, entity, comp2, NULL);
        REQUIRE(ecs_is_ready(ecs, entity));
    }

    ecs_ret_t ret = ecs_run_system(ecs, sys1, 0);

    REQUIRE(ret == 0);

    return true;
}

TEST_CASE(test_destroy)
{
    // Set up system
    sys1 =ecs_define_system(ecs, 0,comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);
    ecs_require_component(ecs, sys1, comp2);

    // Create an entity
    ecs_entity_t entity = ecs_create(ecs);

    // Add components to entity
    comp_t* comp_one = ecs_add(ecs, entity, comp1, NULL);
    comp_t* comp_two = ecs_add(ecs, entity, comp2, NULL);

    comp_one->used = false;
    comp_two->used = false;

    // Run system
    ecs_run_system(ecs, sys1, 0);

    // Verify that the entity was processed by the system
    REQUIRE(comp_one->used);
    REQUIRE(comp_two->used);

    // Destroy entity
    ecs_destroy(ecs, entity);

    // Reset components
    comp_one->used = false;
    comp_two->used = false;

    // Run system again
    ecs_run_system(ecs, sys1, 0);

    // Verify that entity was not processed by the system
    REQUIRE(!comp_one->used);
    REQUIRE(!comp_two->used);

    // Verify entity is inactive
    REQUIRE(!ecs_is_ready(ecs, entity));

    return true;
}

// Removes components starting at the front of the array
static ecs_ret_t remove_system(ecs_t* ecs,
                               ecs_entity_t* entities,
                               size_t entity_count,
                               void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)udata;

    while (entity_count > 0)
    {
        ecs_entity_t entity = entities[0];

        ecs_remove(ecs, entity, comp1);
        entity_count--;

        if (ecs_has(ecs, entity, comp1))
            return -1;

    }
    return 0;
}

TEST_CASE(test_remove_system)
{
    sys1 =ecs_define_system(ecs, 0, remove_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);
    ecs_require_component(ecs, sys1, comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1, NULL);
        ecs_add(ecs, entity, comp2, NULL);
        REQUIRE(ecs_is_ready(ecs, entity));
    }

    ecs_ret_t ret = ecs_run_system(ecs, sys1, 0);

    REQUIRE(ret == 0);

    return true;
}

// System that enqueues entities for destruction
static ecs_ret_t queue_destroy_system(ecs_t* ecs,
                                      ecs_entity_t* entities,
                                      size_t entity_count,
                                      void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_entity_t entity = entities[i];

        REQUIRE(ecs_is_ready(ecs, entity));
        ecs_destroy(ecs, entity);
        REQUIRE(!ecs_is_ready(ecs, entity));
    }

    return 0;
}

TEST_CASE(test_queue_destroy_system)
{
    sys1 =ecs_define_system(ecs, 0, queue_destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);
    ecs_require_component(ecs, sys1, comp2);

    ecs_entity_t entities[MAX_ENTITIES];

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1, NULL);
        ecs_add(ecs, entity, comp2, NULL);
        REQUIRE(ecs_is_ready(ecs, entity));
        entities[i] = entity;
    }

    // Run system again
    ecs_run_system(ecs, sys1, 0);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        REQUIRE(!ecs_is_ready(ecs, entities[i]));
    }

    return true;
}

// System that enqueues components for removal
static ecs_ret_t queue_remove_system(ecs_t* ecs,
                                     ecs_entity_t* entities,
                                     size_t entity_count,
                                     void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
    {
        ecs_remove(ecs, entities[i], comp1);
    }

    return 0;
}

TEST_CASE(test_queue_remove_system)
{
    sys1 =ecs_define_system(ecs, 0, queue_remove_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);
    ecs_require_component(ecs, sys1, comp2);

    ecs_entity_t entities[MAX_ENTITIES];

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = ecs_create(ecs);
        ecs_add(ecs, entity, comp1, NULL);
        ecs_add(ecs, entity, comp2, NULL);
        REQUIRE(ecs_is_ready(ecs, entity));
        entities[i] = entity;
    }

    ecs_run_system(ecs, sys1, 0);

    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_entity_t entity = entities[i];

        if (ecs_is_ready(ecs, entity))
            REQUIRE(!ecs_has(ecs, entity, comp1));
    }

    return true;
}

// Tests destructors
TEST_CASE(test_queue_destroy_system_with_destructor)
{
    ecs_comp_t comp_type = ecs_define_component(ecs, sizeof(comp_t), constructor, destructor);

    sys1 =ecs_define_system(ecs, 0, queue_destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp_type);

    ecs_entity_t entity = ecs_create(ecs);
    comp_t* comp = ecs_add(ecs, entity, comp_type, &(test_args_t){ true });

    REQUIRE(comp->used);
    REQUIRE(ecs_is_ready(ecs, entity));

    // Run system to queue destruction
    ecs_run_system(ecs, sys1, 0);

    // Verify entity is destroyed
    REQUIRE(!ecs_is_ready(ecs, entity));

    // WARNING: We assume memory has not been reclaimed
    // Verify destructor was called
    REQUIRE(!comp->used);

    return true;
}

// Tests system enable/disable
TEST_CASE(test_enable_disable)
{
    // Set up system
    sys1 =ecs_define_system(ecs, 0, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, sys1, comp1);

    // Create entity
    ecs_entity_t entity = ecs_create(ecs);

    // Add component to entity
    comp_t* comp = ecs_add(ecs, entity, comp1, NULL);
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

static bool added = false;
static bool removed = false;

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

// Tests system add/remove callbacks
TEST_CASE(test_add_remove_callbacks)
{
    sys1 =ecs_define_system(ecs, 0, empty_system, on_add, on_remove, NULL);

    ecs_require_component(ecs, sys1, comp1);

    ecs_run_system(ecs, sys1, 0);

    ecs_entity_t entity = ecs_create(ecs);
    ecs_add(ecs, entity, comp1, NULL);
    ecs_destroy(ecs, entity);

    REQUIRE(added);
    REQUIRE(removed);

    return true;
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

// Tests replacing system callbacks on the fly
TEST_CASE(test_set_system_callbacks)
{
    added = false;
    removed = false;

    sys1 =ecs_define_system(ecs, 0, ret_system, NULL, NULL, NULL);
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
    ecs_add(ecs, entity, comp1, NULL);
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

static TEST_SUITE(suite_ecs)
{
    RUN_TEST_CASE(test_reset);
    RUN_TEST_CASE(test_exclude);
    RUN_TEST_CASE(test_constructor);
    RUN_TEST_CASE(test_destructor_remove);
    RUN_TEST_CASE(test_destructor_destroy);
    RUN_TEST_CASE(test_create_destroy);
    RUN_TEST_CASE(test_add_remove);
    RUN_TEST_CASE(test_add_systems);
    RUN_TEST_CASE(test_remove);
    RUN_TEST_CASE(test_remove_comp_system);
    RUN_TEST_CASE(test_destroy);
    RUN_TEST_CASE(test_destroy_system);
    RUN_TEST_CASE(test_remove_system);
    RUN_TEST_CASE(test_queue_destroy_system);
    RUN_TEST_CASE(test_queue_remove_system);
    RUN_TEST_CASE(test_queue_destroy_system_with_destructor);
    RUN_TEST_CASE(test_enable_disable);
    RUN_TEST_CASE(test_system_mask);
    RUN_TEST_CASE(test_add_remove_callbacks);
    RUN_TEST_CASE(test_set_system_callbacks);
    RUN_TEST_CASE(test_system_udata);
}

int main ()
{
    pu_display_colors(true);
    pu_setup(setup, teardown);
    RUN_TEST_SUITE(suite_ecs);
    pu_print_stats();
    return pu_test_failed();
}

