/**
 * For a more complete demonstration, see example2.c
 */

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#include <string.h>  /* strcmp */

/*
 * Used to extend REQUIRE
 */
static bool str_eq(const char* str1, const char* str2)
{
    return 0 == strcmp(str1, str2);
}

/*
 * Passing test. Note that the test function declaration returns a boolean value
 * and that the test definition returns true. All test functions must return
 * true.
 */

TEST_CASE(test1)
{
    REQUIRE(2 + 2 == 4);                /* Boolean assertion (ok)         */
    REQUIRE(str_eq("apples", "apples"));  /* String equality assertion (ok) */
    return true;
}

/* Failing test */
TEST_CASE(test2)
{
    REQUIRE(2 + 2 != 4);                /* Boolean assertion (fails) */
    REQUIRE(str_eq("apples", "oranges")); /* String equality (fails */
    return true;
}

/* Mixed results */
TEST_CASE(test3)
{
    REQUIRE(2 + 2 == 4);                 /* Boolean assertion (ok) */
    REQUIRE(str_eq("apples", "oranges"));  /* String equality fails */
    return true;
}

/* Test suite container function (multiple test suites can be specified. */
static TEST_SUITE(test_suite)
{
    RUN_TEST_CASE(test1);
    RUN_TEST_CASE(test2);
    RUN_TEST_CASE(test3);
}

int
main ()
{
    RUN_TEST_SUITE(test_suite);
    pu_print_stats(); /* Optional */
    return 0;
}

/* EoF */
