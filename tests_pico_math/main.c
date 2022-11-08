#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

void suite_scalar();
void suite_v2();
void suite_t2();
void suite_b2();
void suite_rng();

int main()
{
    pu_display_colors(true);

    RUN_TEST_SUITE(suite_scalar);
    RUN_TEST_SUITE(suite_v2);
    RUN_TEST_SUITE(suite_t2);
    RUN_TEST_SUITE(suite_b2);
    RUN_TEST_SUITE(suite_rng);

    pu_print_stats();

    return pu_test_failed();
}
