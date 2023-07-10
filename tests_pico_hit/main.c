#include "../pico_hit.h"
#include "../pico_unit.h"

TEST_SUITE(suite_sat);
TEST_SUITE(suite_ray);

int main()
{
    pu_display_colors(true);
    RUN_TEST_SUITE(suite_sat);
    RUN_TEST_SUITE(suite_ray);
    pu_print_stats();
    return pu_test_failed();
}

#define PICO_HIT_IMPLEMENTATION
#include "../pico_hit.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"
