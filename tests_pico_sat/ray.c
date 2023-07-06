#include "../pico_unit.h"

TEST_CASE(test_segment)
{
    return true;
}

TEST_SUITE(suite_ray)
{
    RUN_TEST_CASE(test_segment);
}