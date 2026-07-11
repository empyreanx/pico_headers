#include "common.h"

// --- Helpers ---

static size_t count_children(ecs_t* ecs, ecs_entity_t entity)
{
    size_t count = 0;

    ecs_entity_t child = ecs_get_first_child(ecs, entity);

    while (!ECS_IS_INVALID(child))
    {
        count++;
        child = ecs_get_next_sibling(ecs, child);
    }

    return count;
}

static bool has_child(ecs_t* ecs, ecs_entity_t parent, ecs_entity_t child)
{
    ecs_entity_t it = ecs_get_first_child(ecs, parent);

    while (!ECS_IS_INVALID(it))
    {
        if (it.id == child.id)
            return true;

        it = ecs_get_next_sibling(ecs, it);
    }

    return false;
}

static ecs_ret_t destroy_all_system(ecs_t* ecs,
                                    ecs_entity_t* entities,
                                    size_t entity_count,
                                    void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
        ecs_destroy(ecs, entities[i]);

    return 0;
}

static ecs_ret_t destroy_hierarchy_system(ecs_t* ecs,
                                          ecs_entity_t* entities,
                                          size_t entity_count,
                                          void* udata)
{
    (void)udata;

    for (size_t i = 0; i < entity_count; i++)
        ecs_destroy_hierarchy(ecs, entities[i]);

    return 0;
}

// =============================================================
// suite_hierarchy: parent/child hierarchy
// =============================================================

TEST_CASE(test_child_of)
{
    ecs_entity_t parent = ecs_create(ecs);
    ecs_entity_t child1 = ecs_create(ecs);
    ecs_entity_t child2 = ecs_create(ecs);

    // A fresh entity has no parent and no children
    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, parent)));
    REQUIRE(ECS_IS_INVALID(ecs_get_first_child(ecs, parent)));

    ecs_child_of(ecs, child1, parent);
    ecs_child_of(ecs, child2, parent);

    REQUIRE(ecs_get_parent(ecs, child1).id == parent.id);
    REQUIRE(ecs_get_parent(ecs, child2).id == parent.id);

    REQUIRE(count_children(ecs, parent) == 2);
    REQUIRE(has_child(ecs, parent, child1));
    REQUIRE(has_child(ecs, parent, child2));

    // Attaching twice is harmless
    ecs_child_of(ecs, child1, parent);

    REQUIRE(count_children(ecs, parent) == 2);

    return true;
}

TEST_CASE(test_detach)
{
    ecs_entity_t parent = ecs_create(ecs);
    ecs_entity_t child1 = ecs_create(ecs);
    ecs_entity_t child2 = ecs_create(ecs);
    ecs_entity_t child3 = ecs_create(ecs);

    ecs_child_of(ecs, child1, parent);
    ecs_child_of(ecs, child2, parent);
    ecs_child_of(ecs, child3, parent);

    // Detach the middle child (children are prepended, so child2 is in the
    // middle of the list)
    ecs_detach(ecs, child2);

    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, child2)));
    REQUIRE(count_children(ecs, parent) == 2);
    REQUIRE(has_child(ecs, parent, child1));
    REQUIRE(has_child(ecs, parent, child3));

    // Detaching an entity with no parent is harmless
    ecs_detach(ecs, child2);

    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, child2)));

    return true;
}

TEST_CASE(test_reparent)
{
    ecs_entity_t parent1 = ecs_create(ecs);
    ecs_entity_t parent2 = ecs_create(ecs);
    ecs_entity_t child   = ecs_create(ecs);

    ecs_child_of(ecs, child, parent1);
    ecs_child_of(ecs, child, parent2);

    REQUIRE(ecs_get_parent(ecs, child).id == parent2.id);
    REQUIRE(count_children(ecs, parent1) == 0);
    REQUIRE(count_children(ecs, parent2) == 1);

    return true;
}

TEST_CASE(test_destroy_child)
{
    ecs_entity_t parent = ecs_create(ecs);
    ecs_entity_t child1 = ecs_create(ecs);
    ecs_entity_t child2 = ecs_create(ecs);
    ecs_entity_t child3 = ecs_create(ecs);

    ecs_child_of(ecs, child1, parent);
    ecs_child_of(ecs, child2, parent);
    ecs_child_of(ecs, child3, parent);

    // Destroying a child unlinks it from its parent
    ecs_destroy(ecs, child2);

    REQUIRE(count_children(ecs, parent) == 2);
    REQUIRE(has_child(ecs, parent, child1));
    REQUIRE(has_child(ecs, parent, child3));

    return true;
}

TEST_CASE(test_destroy_parent)
{
    ecs_entity_t parent     = ecs_create(ecs);
    ecs_entity_t child1     = ecs_create(ecs);
    ecs_entity_t child2     = ecs_create(ecs);
    ecs_entity_t grandchild = ecs_create(ecs);

    ecs_child_of(ecs, child1, parent);
    ecs_child_of(ecs, child2, parent);
    ecs_child_of(ecs, grandchild, child1);

    // Destroying the parent orphans its children
    ecs_destroy(ecs, parent);

    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, child1)));
    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, child2)));

    // But the children keep their own children
    REQUIRE(ecs_get_parent(ecs, grandchild).id == child1.id);
    REQUIRE(count_children(ecs, child1) == 1);

    return true;
}

TEST_CASE(test_recycled_entity_has_no_links)
{
    ecs_entity_t parent = ecs_create(ecs);
    ecs_entity_t child  = ecs_create(ecs);

    ecs_child_of(ecs, child, parent);

    ecs_destroy(ecs, child);
    ecs_destroy(ecs, parent);

    // The IDs are recycled from the pool
    ecs_entity_t entity1 = ecs_create(ecs);
    ecs_entity_t entity2 = ecs_create(ecs);

    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, entity1)));
    REQUIRE(ECS_IS_INVALID(ecs_get_first_child(ecs, entity1)));
    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, entity2)));
    REQUIRE(ECS_IS_INVALID(ecs_get_first_child(ecs, entity2)));

    return true;
}

TEST_CASE(test_destroy_hierarchy)
{
    // A three level hierarchy with a sibling subtree that must survive
    ecs_entity_t root        = ecs_create(ecs);
    ecs_entity_t child1      = ecs_create(ecs);
    ecs_entity_t child2      = ecs_create(ecs);
    ecs_entity_t grandchild1 = ecs_create(ecs);
    ecs_entity_t grandchild2 = ecs_create(ecs);

    ecs_entity_t other       = ecs_create(ecs);
    ecs_entity_t other_child = ecs_create(ecs);

    ecs_child_of(ecs, child1, root);
    ecs_child_of(ecs, child2, root);
    ecs_child_of(ecs, grandchild1, child1);
    ecs_child_of(ecs, grandchild2, child1);

    ecs_child_of(ecs, other_child, other);

    ecs_destroy_hierarchy(ecs, root);

    // The entire subtree is destroyed
    REQUIRE(!ecs_is_ready(ecs, root));
    REQUIRE(!ecs_is_ready(ecs, child1));
    REQUIRE(!ecs_is_ready(ecs, child2));
    REQUIRE(!ecs_is_ready(ecs, grandchild1));
    REQUIRE(!ecs_is_ready(ecs, grandchild2));

    // The unrelated subtree is untouched
    REQUIRE(ecs_is_ready(ecs, other));
    REQUIRE(ecs_is_ready(ecs, other_child));
    REQUIRE(ecs_get_parent(ecs, other_child).id == other.id);

    return true;
}

TEST_CASE(test_destroy_hierarchy_leaf)
{
    // Destroying a subtree consisting of a single entity is equivalent to
    // ecs_destroy
    ecs_entity_t parent = ecs_create(ecs);
    ecs_entity_t child  = ecs_create(ecs);

    ecs_child_of(ecs, child, parent);
    ecs_destroy_hierarchy(ecs, child);

    REQUIRE(!ecs_is_ready(ecs, child));
    REQUIRE(ecs_is_ready(ecs, parent));
    REQUIRE(count_children(ecs, parent) == 0);

    return true;
}

TEST_CASE(test_deferred_destroy_hierarchy)
{
    ecs_system_t sys = ecs_define_system(ecs, destroy_hierarchy_system, NULL);
    ecs_require(ecs, sys, comp1);

    ecs_entity_t root       = ecs_create(ecs);
    ecs_entity_t child      = ecs_create(ecs);
    ecs_entity_t grandchild = ecs_create(ecs);

    ecs_child_of(ecs, child, root);
    ecs_child_of(ecs, grandchild, child);

    // Only the root matches the system
    ecs_add(ecs, root, comp1, NULL);

    // The destroys are deferred and applied when the queue is flushed
    ecs_run_system(ecs, sys, 0);

    REQUIRE(!ecs_is_ready(ecs, root));
    REQUIRE(!ecs_is_ready(ecs, child));
    REQUIRE(!ecs_is_ready(ecs, grandchild));

    return true;
}

TEST_CASE(test_deferred_destroy_parent)
{
    ecs_system_t sys = ecs_define_system(ecs, destroy_all_system, NULL);
    ecs_require(ecs, sys, comp1);

    ecs_entity_t parent = ecs_create(ecs);
    ecs_entity_t child  = ecs_create(ecs);

    ecs_child_of(ecs, child, parent);

    // Only the parent matches the system
    ecs_add(ecs, parent, comp1, NULL);

    // The destroy is deferred and applied when the queue is flushed
    ecs_run_system(ecs, sys, 0);

    REQUIRE(!ecs_is_ready(ecs, parent));
    REQUIRE(ecs_is_ready(ecs, child));
    REQUIRE(ECS_IS_INVALID(ecs_get_parent(ecs, child)));

    return true;
}

TEST_SUITE(suite_hierarchy)
{
    RUN_TEST_CASE(test_child_of);
    RUN_TEST_CASE(test_detach);
    RUN_TEST_CASE(test_reparent);
    RUN_TEST_CASE(test_destroy_child);
    RUN_TEST_CASE(test_destroy_parent);
    RUN_TEST_CASE(test_recycled_entity_has_no_links);
    RUN_TEST_CASE(test_destroy_hierarchy);
    RUN_TEST_CASE(test_destroy_hierarchy_leaf);
    RUN_TEST_CASE(test_deferred_destroy_hierarchy);
    RUN_TEST_CASE(test_deferred_destroy_parent);
}