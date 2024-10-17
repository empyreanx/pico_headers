#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <assert.h>
#include <stdio.h>

#define MIN_ENTITIES (1 * 1024)
#define MAX_ENTITIES (8 * 1024)

ecs_t* ecs = NULL;
ecs_id_t comp1_id;
ecs_id_t comp2_id;

ecs_id_t system1_id;
ecs_id_t system2_id;

typedef struct
{
    bool used;
} comp_t;

void setup()
{
    ecs = ecs_new(MIN_ENTITIES, NULL);

    comp1_id = ecs_register_component(ecs, sizeof(comp_t), NULL, NULL);
    comp2_id = ecs_register_component(ecs, sizeof(comp_t), NULL, NULL);
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
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, comp1_id, NULL);
        ecs_add(ecs, id, comp2_id, NULL);
        REQUIRE(ecs_is_ready(ecs, id));
    }

    ecs_reset(ecs);

    return true;
}

static struct
{
    ecs_id_t eid;
    size_t count;
} exclude_sys_state;

static ecs_ret_t exclude_system(ecs_t* ecs,
                                ecs_id_t* entities,
                                int entity_count,
                                ecs_dt_t dt,
                                void* udata)
{
    (void)ecs;
    (void)entity_count;
    (void)dt;
    (void)udata;

    exclude_sys_state.count = entity_count;

    if (entity_count > 0)
    {
        exclude_sys_state.eid = entities[0];
    }

    return 0;
}

TEST_CASE(test_exclude)
{
    ecs_id_t system_id = ecs_register_system(ecs, exclude_system, NULL, NULL, NULL);

    ecs_require_component(ecs, system_id, comp2_id);
    ecs_exclude_component(ecs, system_id, comp1_id);

    ecs_id_t eid1 = ecs_create(ecs);
    ecs_add(ecs, eid1, comp1_id, NULL);
    ecs_add(ecs, eid1, comp2_id, NULL);

    ecs_id_t eid2 = ecs_create(ecs);
    ecs_add(ecs, eid2, comp2_id, NULL);

    ecs_update_system(ecs, system_id, 0.0);

    REQUIRE(exclude_sys_state.count == 1);
    REQUIRE(exclude_sys_state.eid == eid2);

    return true;
}

typedef struct
{
    bool used;
} test_args_t;

void constructor(ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)
{
    (void)ecs;
    (void)entity_id;
    (void)args;

    test_args_t* test_args = args;

    comp_t* comp = ptr;
    comp->used = test_args->used;
}

TEST_CASE(test_constructor)
{
    ecs_id_t comp_id = ecs_register_component(ecs, sizeof(comp_t), constructor, NULL);

    ecs_id_t entity_id = ecs_create(ecs);
    comp_t* comp = ecs_add(ecs, entity_id, comp_id, &(test_args_t){ true });

    REQUIRE(comp->used);

    return true;
}

void destructor(ecs_t* ecs, ecs_id_t entity_id, void* ptr)
{
    (void)ecs;
    (void)entity_id;

    comp_t* comp = ptr;
    comp->used = false;
}

TEST_CASE(test_destructor_remove)
{
    ecs_id_t comp_id = ecs_register_component(ecs, sizeof(comp_t), constructor, destructor);

    ecs_id_t entity_id = ecs_create(ecs);
    comp_t* comp = ecs_add(ecs, entity_id, comp_id, &(test_args_t){ true });
    ecs_remove(ecs, entity_id, comp_id);

    REQUIRE(!comp->used);

    return true;
}

TEST_CASE(test_destructor_destroy)
{
    ecs_id_t comp_id = ecs_register_component(ecs, sizeof(comp_t), constructor, destructor);

    ecs_id_t entity_id = ecs_create(ecs);
    comp_t* comp = ecs_add(ecs, entity_id, comp_id, &(test_args_t){ true });
    ecs_destroy(ecs, entity_id);

    REQUIRE(!ecs_is_ready(ecs, entity_id));

    //WARNING: We assume memory has no been reclaimed
    REQUIRE(!comp->used);

    return true;
}

TEST_CASE(test_create_destroy)
{
    // Create an entity
    ecs_id_t id = ecs_create(ecs);

    // Verify the entity is alive
    REQUIRE(ecs_is_ready(ecs, id));

    // Destroy the component
    ecs_destroy(ecs, id);

    // Verify that the component is no longer alive
    REQUIRE(!ecs_is_ready(ecs, id));

    return true;
}

TEST_CASE(test_add_remove)
{
    ecs_id_t id = ecs_create(ecs);

    // Initially entity should have no components
    REQUIRE(!ecs_has(ecs, id, comp1_id));
    REQUIRE(!ecs_has(ecs, id, comp2_id));

    // Add a component
    ecs_add(ecs, id, comp1_id, NULL);

    // Verify that the entity has one component and not the other
    REQUIRE(ecs_has(ecs, id, comp1_id));
    REQUIRE(!ecs_has(ecs, id, comp2_id));

    // Add a second component to the entity
    ecs_add(ecs, id, comp2_id, NULL);

    // Verify that the entity has both components
    REQUIRE(ecs_has(ecs, id, comp1_id));
    REQUIRE(ecs_has(ecs, id, comp2_id));

    // Remove the first component
    ecs_remove(ecs, id, comp1_id);

    // Verify that the first component has been removed
    REQUIRE(!ecs_has(ecs, id, comp1_id));
    REQUIRE(ecs_has(ecs, id, comp2_id));

    // Remove the second componentt
    ecs_remove(ecs, id, comp2_id);

    // Verify that both components have been removed
    REQUIRE(!ecs_has(ecs, id, comp1_id));
    REQUIRE(!ecs_has(ecs, id, comp2_id));

    return true;
}

static struct
{
    ecs_id_t eid;
    size_t count;
} remove_sys_state;

static ecs_ret_t remove_comp_system(ecs_t* ecs,
                                   ecs_id_t* entities,
                                   int entity_count,
                                   ecs_dt_t dt,
                                   void* udata)
{
    (void)ecs;
    (void)entities;
    (void)entity_count;
    (void)dt;
    (void)udata;

    remove_sys_state.count = entity_count;

    return 0;
}

TEST_CASE(test_remove_comp_system)
{
    ecs_id_t system_id = ecs_register_system(ecs, remove_comp_system, NULL, NULL, NULL);

    ecs_require_component(ecs, system_id, comp2_id);
    ecs_require_component(ecs, system_id, comp1_id);

    ecs_id_t eid1 = ecs_create(ecs);
    ecs_add(ecs, eid1, comp1_id, NULL);
    ecs_add(ecs, eid1, comp2_id, NULL);

    ecs_remove(ecs, eid1, comp1_id);

    ecs_update_system(ecs, system_id, 0.0);

    REQUIRE(remove_sys_state.count == 1);

    return true;
}

// Turns on the `used` flag on the components of matching entities
static ecs_ret_t comp_system(ecs_t* ecs,
                             ecs_id_t* entities,
                             int entity_count,
                             ecs_dt_t dt,
                             void* udata)
{
	(void)dt;
	(void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_id_t id = entities[i];

        if (ecs_has(ecs, id, comp1_id))
        {
            comp_t* comp = ecs_get(ecs, id, comp1_id);
            comp->used = true;
        }

        if (ecs_has(ecs, id, comp2_id))
        {
            comp_t* comp = ecs_get(ecs, id, comp2_id);
            comp->used = true;
        }

    }

    return 0;
}

TEST_CASE(test_add_systems)
{
    // Set up systems
    system1_id = ecs_register_system(ecs,  comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id);

    system2_id = ecs_register_system(ecs,  comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system2_id, comp1_id);
    ecs_require_component(ecs, system2_id, comp2_id);

    // Create a couple entities
    ecs_id_t id1 = ecs_create(ecs);
    ecs_id_t id2 = ecs_create(ecs);

    // Add a component to entity 1
    comp_t* comp1 = ecs_add(ecs, id1, comp1_id, NULL);
    comp1->used = false;

    // Run system1_id
    ecs_update_system(ecs, system1_id, 0.0);

    // Confirm that entity 1 was process by the system
    REQUIRE(comp1->used);

    // Add entities to entity 2
    comp1 = ecs_add(ecs, id2, comp1_id, NULL);
    comp1->used = false;

    comp_t* comp2 = ecs_add(ecs, id2, comp2_id, NULL);
    comp2->used = false;

    // Run Sys2
    ecs_update_system(ecs, system2_id, 0.0);

    // Confirm that both entities we processed by the system
    REQUIRE(comp1->used);
    REQUIRE(comp2->used);

    return true;
}

TEST_CASE(test_remove)
{
    // Set up system
    system1_id = ecs_register_system(ecs, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id); // Entity must have at least
    ecs_require_component(ecs, system1_id, comp2_id); // component 1 and 2 to match

    // Create an entity
    ecs_id_t id = ecs_create(ecs);

    // Add components to the entity
    comp_t* comp1 = ecs_add(ecs, id, comp1_id, NULL);
    comp_t* comp2 = ecs_add(ecs, id, comp2_id, NULL);

    comp1->used = false;
    comp2->used = false;

    // Run system
    ecs_update_system(ecs, system1_id, 0.0);

    // Verify that the system processed the entity
    REQUIRE(comp1->used);
    REQUIRE(comp2->used);

    // Reset components
    comp1->used = false;
    comp2->used = false;

    // Remove compoent 2
    ecs_remove(ecs, id, comp2_id);

    // Run system again
    ecs_update_system(ecs, system1_id, 0.0);

    // Verify that the entity was remove by the system
    REQUIRE(comp1->used);
    REQUIRE(!comp2->used);

    return true;
}

TEST_CASE(test_destroy)
{
    // Set up system
    system1_id = ecs_register_system(ecs, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id);
    ecs_require_component(ecs, system1_id, comp2_id);

    // Create an entity
    ecs_id_t id = ecs_create(ecs);

    // Add components to entity
    comp_t* comp1 = ecs_add(ecs, id, comp1_id, NULL);
    comp_t* comp2 = ecs_add(ecs, id, comp2_id, NULL);

    comp1->used = false;
    comp2->used = false;

    // Run system
    ecs_update_system(ecs, system1_id, 0.0);

    // Verify that the entity was processed by the system
    REQUIRE(comp1->used);
    REQUIRE(comp2->used);

    // Destroy entity
    ecs_destroy(ecs, id);

    // Reset components
    comp1->used = false;
    comp2->used = false;

    // Run system again
    ecs_update_system(ecs, system1_id, 0.0);

    // Verify that entity was not processed by the system
    REQUIRE(!comp1->used);
    REQUIRE(!comp2->used);

    // Verify entity is inactive
    REQUIRE(!ecs_is_ready(ecs, id));

    return true;
}

static ecs_ret_t destroy_system(ecs_t* ecs,
                                ecs_id_t* entities,
                                int entity_count,
                                ecs_dt_t dt,
                                void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)dt;
    (void)udata;

    while (entity_count > 0)
    {
        ecs_id_t entity_id = entities[0];

        ecs_destroy(ecs, entity_id);
        entity_count--;

        if (ecs_is_ready(ecs, entity_id))
            return -1;
    }

    return 0;
}

TEST_CASE(test_destroy_system)
{
    system1_id = ecs_register_system(ecs, destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id);
    ecs_require_component(ecs, system1_id, comp2_id);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, comp1_id, NULL);
        ecs_add(ecs, id, comp2_id, NULL);
        REQUIRE(ecs_is_ready(ecs, id));
    }

    // Run system again
    ecs_ret_t ret = ecs_update_system(ecs, system1_id, 0.0);

    REQUIRE(ret == 0);

    return true;
}

static ecs_ret_t remove_system(ecs_t* ecs,
                               ecs_id_t* entities,
                               int entity_count,
                               ecs_dt_t dt,
                               void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)dt;
    (void)udata;

    while (entity_count > 0)
    {
        ecs_id_t entity_id = entities[0];

        ecs_remove(ecs, entity_id, comp1_id);
        entity_count--;

        if (ecs_has(ecs, entity_id, comp1_id))
            return -1;
    }

    return 0;
}

TEST_CASE(test_remove_system)
{
    system1_id = ecs_register_system(ecs, remove_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id);
    ecs_require_component(ecs, system1_id, comp2_id);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, comp1_id, NULL);
        ecs_add(ecs, id, comp2_id, NULL);
        REQUIRE(ecs_is_ready(ecs, id));
    }

    // Run system again
    ecs_ret_t ret = ecs_update_system(ecs, system1_id, 0.0);

    REQUIRE(ret == 0);

    return true;
}

static ecs_ret_t queue_destroy_system(ecs_t* ecs,
                                      ecs_id_t* entities,
                                      int entity_count,
                                      ecs_dt_t dt,
                                      void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)dt;
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_queue_destroy(ecs, entities[i]);
    }

    return 0;
}

TEST_CASE(test_queue_destroy_system)
{
    system1_id = ecs_register_system(ecs, queue_destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id);
    ecs_require_component(ecs, system1_id, comp2_id);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, comp1_id, NULL);
        ecs_add(ecs, id, comp2_id, NULL);
    }

    // Run system again
    ecs_update_system(ecs, system1_id, 0.0);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        REQUIRE(!ecs_is_ready(ecs, i));
    }

    return true;
}

static ecs_ret_t queue_remove_system(ecs_t* ecs,
                                     ecs_id_t* entities,
                                     int entity_count,
                                     ecs_dt_t dt,
                                     void* udata)
{
    (void)entities;
    (void)entity_count;
    (void)dt;
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_queue_remove(ecs, entities[i], comp1_id);
    }

    return 0;
}

TEST_CASE(test_queue_remove_system)
{
    system1_id = ecs_register_system(ecs, queue_remove_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id);
    ecs_require_component(ecs, system1_id, comp2_id);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, comp1_id, NULL);
        ecs_add(ecs, id, comp2_id, NULL);
    }

    // Run system again
    ecs_update_system(ecs, system1_id, 0.0);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (ecs_is_ready(ecs, i))
            REQUIRE(!ecs_has(ecs, i, comp1_id));
    }

    return true;
}

TEST_CASE(test_enable_disable)
{
    // Set up system
    system1_id = ecs_register_system(ecs, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, system1_id, comp1_id);

    // Create entity
    ecs_id_t id = ecs_create(ecs);

    // Add component to entity
    comp_t* comp = ecs_add(ecs, id, comp1_id, NULL);
    comp->used = false;

    // Run system
    ecs_update_system(ecs, system1_id, 0.0);

    // Verify that entity was processed by system
    REQUIRE(comp->used);

    // Reset component
    comp->used = false;

    // Disable system
    ecs_disable_system(ecs, system1_id);

    // Run system
    ecs_update_system(ecs, system1_id, 0.0);

    // Verify that the entity was not processed by the system
    REQUIRE(!comp->used);

    // Enable system
    ecs_enable_system(ecs, system1_id);

    // Run system again
    ecs_update_system(ecs, system1_id, 0.0);

    // Verify that entity was processed by the system
    REQUIRE(comp->used);

    return true;
}

static bool added = false;
static bool removed = false;

static ecs_ret_t empty_system(ecs_t* ecs,
                              ecs_id_t* entities,
                              int entity_count,
                              ecs_dt_t dt,
                              void* udata)
{
    (void)ecs;
    (void)entities;
    (void)entity_count;
    (void)dt;
    (void)udata;
    return 0;
}

static void on_add(ecs_t* ecs, ecs_id_t entity_id, void* udata)
{
    (void)ecs;
    (void)entity_id;
    (void)udata;
    added = true;
}

static void on_remove(ecs_t* ecs, ecs_id_t entity_id, void* udata)
{
    (void)ecs;
    (void)entity_id;
    (void)udata;
    removed = true;
}

TEST_CASE(test_add_remove_callbacks)
{
    system1_id = ecs_register_system(ecs, empty_system, on_add, on_remove, NULL);

    ecs_require_component(ecs, system1_id, comp1_id);

    ecs_update_system(ecs, system1_id, 0.0);

    ecs_id_t entity_id = ecs_create(ecs);
    ecs_add(ecs, entity_id, comp1_id, NULL);
    ecs_destroy(ecs, entity_id);

    REQUIRE(added);
    REQUIRE(removed);

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
    RUN_TEST_CASE(test_enable_disable);
    RUN_TEST_CASE(test_add_remove_callbacks);
}

int main ()
{
    pu_display_colors(true);
    pu_setup(setup, teardown);
    RUN_TEST_SUITE(suite_ecs);
    pu_print_stats();
    return pu_test_failed();
}

