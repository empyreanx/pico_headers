#define PICO_BVH_IMPLEMENTATION
#include "../pico_bvh.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

TEST_SUITE(bvh_test_suite)
{
}

int main()
{
    pu_display_colors(true);
    RUN_TEST_SUITE(bvh_test_suite);
    pu_print_stats();
    return pu_test_failed();
}
