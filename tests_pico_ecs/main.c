#include "common.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#define PICO_ECS_IMPLEMENTATION
#include "../pico_ecs.h"

ecs_t* ecs = NULL;
ecs_comp_t comp1;
ecs_comp_t comp2;
ecs_comp_t comp3;

ecs_system_t sys1;
ecs_system_t sys2;

void setup()
{
    ecs = ecs_new(MIN_ENTITIES, NULL);
    comp1 = ecs_define_component(ecs, sizeof(comp_t), NULL, NULL, NULL, NULL);
    comp2 = ecs_define_component(ecs, sizeof(comp_t), NULL, NULL, NULL, NULL);
    comp3 = ecs_define_component(ecs, sizeof(comp_t), NULL, NULL, NULL, NULL);
}

void teardown()
{
    ecs_free(ecs);
    ecs = NULL;
}

TEST_CASE(test_capacity_validation)
{
    // capacity with the high bit set is not valid
    REQUIRE(ecs_is_valid_capacity(SIZE_MAX >> 8, 1 << 7));
    REQUIRE(!ecs_is_valid_capacity((SIZE_MAX >> 8) + 1, 1 << 7));
    REQUIRE(!ecs_is_valid_capacity(SIZE_MAX >> 8, 1 << 8));

    // capacity with the high bit set is not valid
    REQUIRE(ecs_is_valid_capacity((SIZE_MAX >> 1), 1));
    REQUIRE(!ecs_is_valid_capacity((SIZE_MAX >> 1) + 1, 1));

    // zero capacity is invalid
    REQUIRE(!ecs_is_valid_capacity(0, 16));
    REQUIRE(!ecs_is_valid_capacity(16, 0));
    REQUIRE(!ecs_is_valid_capacity(0, 0));

    // normal cases
    REQUIRE(ecs_is_valid_capacity(500, 128));
    REQUIRE(ecs_is_valid_capacity(1000, 8));

    return true;
}

TEST_SUITE(suite_validation)
{
    RUN_TEST_CASE(test_capacity_validation);
}

TEST_SUITE(suite_entity);
TEST_SUITE(suite_components);
TEST_SUITE(suite_systems);
TEST_SUITE(suite_exclude);
TEST_SUITE(suite_deferred);
TEST_SUITE(suite_validation);

int main ()
{
    pu_display_colors(true);
    pu_setup(setup, teardown);
    RUN_TEST_SUITE(suite_entity);
    RUN_TEST_SUITE(suite_components);
    RUN_TEST_SUITE(suite_systems);
    RUN_TEST_SUITE(suite_exclude);
    RUN_TEST_SUITE(suite_deferred);
    RUN_TEST_SUITE(suite_validation);
    pu_print_stats();
    return pu_test_failed();
}
