#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

#include <assert.h>
#include <stdio.h>

#define MAX_ENTITIES (8 * 1024)

ecs_t* ecs = NULL;

enum
{
    Comp1,
    Comp2
    //Comp3
};

enum
{
    Sys1,
    Sys2
};

typedef struct
{
    bool used;
} comp_t;

void setup()
{
    ecs = ecs_new(1024, NULL);

    ecs_register_component(ecs, Comp1, sizeof(comp_t));
    ecs_register_component(ecs, Comp2, sizeof(comp_t));
    //ecs_register_component(ecs, Comp3, sizeof(comp_t));
}

void teardown()
{
    ecs_free(ecs);
    ecs = NULL;
}

PU_TEST(test_create_destroy)
{
    // Create an entity
    ecs_id_t id = ecs_create(ecs);

    // Verify the entity is alive
    PU_ASSERT(ecs_is_ready(ecs, id));

    // Destroy the component
    ecs_destroy(ecs, id);

    // Verify that the component is no longer alive
    PU_ASSERT(!ecs_is_ready(ecs, id));

    return true;
}

PU_TEST(test_add_remove)
{
    ecs_id_t id = ecs_create(ecs);

    // Initially entity should have no components
    PU_ASSERT(!ecs_has(ecs, id, Comp1));
    PU_ASSERT(!ecs_has(ecs, id, Comp2));

    // Add a component
    ecs_add(ecs, id, Comp1);

    // Verify that the entity has one component and not the other
    PU_ASSERT(ecs_has(ecs, id, Comp1));
    PU_ASSERT(!ecs_has(ecs, id, Comp2));

    // Add a second component to the entity
    ecs_add(ecs, id, Comp2);

    // Verify that the entity has both components
    PU_ASSERT(ecs_has(ecs, id, Comp1));
    PU_ASSERT(ecs_has(ecs, id, Comp2));

    // Remove the first component
    ecs_remove(ecs, id, Comp1);

    // Verify that the first component has been removed
    PU_ASSERT(!ecs_has(ecs, id, Comp1));
    PU_ASSERT(ecs_has(ecs, id, Comp2));

    // Remove the second componentt
    ecs_remove(ecs, id, Comp2);

    // Verify that both components have been removed
    PU_ASSERT(!ecs_has(ecs, id, Comp1));
    PU_ASSERT(!ecs_has(ecs, id, Comp2));

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

        if (ecs_has(ecs, id, Comp1))
        {
            comp_t* comp = ecs_get(ecs, id, Comp1);
            comp->used = true;
        }

        if (ecs_has(ecs, id, Comp2))
        {
            comp_t* comp = ecs_get(ecs, id, Comp2);
            comp->used = true;
        }

        /*if (ecs_has(ecs, id, Comp3))
        {
            comp_t* comp = ecs_get(ecs, id, Comp3);
            comp->used = true;
        }*/
    }

    return 0;
}

PU_TEST(test_add_systems)
{
    // Set up systems
    ecs_register_system(ecs,   Sys1, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1); // Entities must have component 1

    ecs_register_system(ecs,   Sys2, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys2, Comp1);
    ecs_require_component(ecs, Sys2, Comp2);

    // Create a couple entities
    ecs_id_t id1 = ecs_create(ecs);
    ecs_id_t id2 = ecs_create(ecs);

    // Add a component to entity 1
    comp_t* comp1 = ecs_add(ecs, id1, Comp1);
    comp1->used = false;

    // Run Sys1
    ecs_update_system(ecs, Sys1, 0.0);

    // Confirm that entity 1 was process by the system
    PU_ASSERT(comp1->used);

    // Add entities to entity 2
    comp1 = ecs_add(ecs, id2, Comp1);
    comp1->used = false;

    comp_t* comp2 = ecs_add(ecs, id2, Comp2);
    comp2->used = false;

    // Run Sys2
    ecs_update_system(ecs, Sys2, 0.0);

    // Confirm that both entities we processed by the system
    PU_ASSERT(comp1->used);
    PU_ASSERT(comp2->used);

    return true;
}

PU_TEST(test_remove)
{
    // Set up system
    ecs_register_system(ecs, Sys1, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1); // Entity must have at least
    ecs_require_component(ecs, Sys1, Comp2); // component 1 and 2 to match

    // Create an entity
    ecs_id_t id = ecs_create(ecs);

    // Add components to the entity
    comp_t* comp1 = ecs_add(ecs, id, Comp1);
    comp_t* comp2 = ecs_add(ecs, id, Comp2);

    comp1->used = false;
    comp2->used = false;

    // Run system
    ecs_update_system(ecs, Sys1, 0.0);

    // Verify that the system processed the entity
    PU_ASSERT(comp1->used);
    PU_ASSERT(comp2->used);

    // Reset components
    comp1->used = false;
    comp2->used = false;

    // Remove compoent 2
    ecs_remove(ecs, id, Comp2);

    // Run system again
    ecs_update_system(ecs, Sys1, 0.0);

    // Verify that the entity was remove by the system
    PU_ASSERT(!comp1->used);
    PU_ASSERT(!comp2->used);

    return true;
}

PU_TEST(test_destroy)
{
    // Set up system
    ecs_register_system(ecs, Sys1, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1);
    ecs_require_component(ecs, Sys1, Comp2);

    // Create an entity
    ecs_id_t id = ecs_create(ecs);

    // Add components to entity
    comp_t* comp1 = ecs_add(ecs, id, Comp1);
    comp_t* comp2 = ecs_add(ecs, id, Comp2);

    comp1->used = false;
    comp2->used = false;

    // Run system
    ecs_update_system(ecs, Sys1, 0.0);

    // Verify that the entity was processed by the system
    PU_ASSERT(comp1->used);
    PU_ASSERT(comp2->used);

    // Destroy entity
    ecs_destroy(ecs, id);

    // Reset components
    comp1->used = false;
    comp2->used = false;

    // Run system again
    ecs_update_system(ecs, Sys1, 0.0);

    // Verify that entity was not processed by the system
    PU_ASSERT(!comp1->used);
    PU_ASSERT(!comp2->used);

    // Verify entity is inactive
    PU_ASSERT(!ecs_is_ready(ecs, id));

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

PU_TEST(test_destroy_system)
{
    ecs_register_system(ecs,   Sys1, destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1);
    ecs_require_component(ecs, Sys1, Comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, Comp1);
        ecs_add(ecs, id, Comp2);
        PU_ASSERT(ecs_is_ready(ecs, id));
    }

    // Run system again
    ecs_ret_t ret = ecs_update_system(ecs, Sys1, 0.0);

    PU_ASSERT(ret == 0);

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

        ecs_remove(ecs, entity_id, Comp1);
        entity_count--;

        if (ecs_has(ecs, entity_id, Comp1))
            return -1;
    }

    return 0;
}

PU_TEST(test_remove_system)
{
    ecs_register_system(ecs,   Sys1, remove_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1);
    ecs_require_component(ecs, Sys1, Comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, Comp1);
        ecs_add(ecs, id, Comp2);
        PU_ASSERT(ecs_is_ready(ecs, id));
    }

    // Run system again
    ecs_ret_t ret = ecs_update_system(ecs, Sys1, 0.0);

    PU_ASSERT(ret == 0);

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

PU_TEST(test_queue_destroy_system)
{
    ecs_register_system(ecs,   Sys1, queue_destroy_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1);
    ecs_require_component(ecs, Sys1, Comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, Comp1);
        ecs_add(ecs, id, Comp2);
    }

    // Run system again
    ecs_update_system(ecs, Sys1, 0.0);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        PU_ASSERT(!ecs_is_ready(ecs, i));
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
        ecs_queue_remove(ecs, entities[i], Comp1);
    }

    return 0;
}

PU_TEST(test_queue_remove_system)
{
    ecs_register_system(ecs,   Sys1, queue_remove_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1);
    ecs_require_component(ecs, Sys1, Comp2);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs_id_t id = ecs_create(ecs);
        ecs_add(ecs, id, Comp1);
        ecs_add(ecs, id, Comp2);
    }

    // Run system again
    ecs_update_system(ecs, Sys1, 0.0);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (ecs_is_ready(ecs, i))
            PU_ASSERT(!ecs_has(ecs, i, Comp1));
    }

    return true;
}

PU_TEST(test_enable_disable)
{
    // Set up system
    ecs_register_system(ecs, Sys1, comp_system, NULL, NULL, NULL);
    ecs_require_component(ecs, Sys1, Comp1);

    // Create entity
    ecs_id_t id = ecs_create(ecs);

    // Add component to entity
    comp_t* comp = ecs_add(ecs, id, Comp1);
    comp->used = false;

    // Run system
    ecs_update_system(ecs, Sys1, 0.0);

    // Verify that entity was processed by system
    PU_ASSERT(comp->used);

    // Reset component
    comp->used = false;

    // Disable system
    ecs_disable_system(ecs, Sys1);

    // Run system
    ecs_update_system(ecs, Sys1, 0.0);

    // Verify that the entity was not processed by the system
    PU_ASSERT(!comp->used);

    // Enable system
    ecs_enable_system(ecs, Sys1);

    // Run system again
    ecs_update_system(ecs, Sys1, 0.0);

    // Verify that entity was processed by the system
    PU_ASSERT(comp->used);

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

PU_TEST(test_add_remove_callbacks)
{
    ecs_register_system(ecs, Sys1, empty_system,
                        on_add, on_remove, NULL);

    ecs_require_component(ecs, Sys1, Comp1);

    ecs_update_system(ecs, Sys1, 0.0);

    ecs_id_t entity_id = ecs_create(ecs);
    ecs_add(ecs, entity_id, Comp1);
    ecs_destroy(ecs, entity_id);

    PU_ASSERT(added);
    PU_ASSERT(removed);

    return true;
}

static PU_SUITE(suite_ecs)
{
    PU_RUN_TEST(test_create_destroy);
    PU_RUN_TEST(test_add_remove);
    PU_RUN_TEST(test_add_systems);
    PU_RUN_TEST(test_remove);
    PU_RUN_TEST(test_destroy);
    PU_RUN_TEST(test_destroy_system);
    PU_RUN_TEST(test_remove_system);
    PU_RUN_TEST(test_queue_destroy_system);
    PU_RUN_TEST(test_queue_remove_system);
    PU_RUN_TEST(test_enable_disable);
    PU_RUN_TEST(test_add_remove_callbacks);
}

int main ()
{
    pu_display_colors(true);
    pu_setup(setup, teardown);
    PU_RUN_SUITE(suite_ecs);
    pu_print_stats();
    return pu_test_failed();
}
