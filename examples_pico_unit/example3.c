#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

/*
 * Used to extend REQUIRE
 */
static bool str_eq(const char* str1, const char* str2)
{
    return 0 == strcmp(str1, str2);
}

static unsigned g_fix = 0;

/* Sets up fixture for (called before test). */
static void
test_setup ()
{
    g_fix = 42;
}

/* Resets fixture (called after test). */
static void
test_teardown ()
{
    g_fix = 0;
}

/* All assertions pass in this test. */
TEST_CASE(test_passing1)
{
    REQUIRE(1);
    REQUIRE(42 == 42);
    REQUIRE(str_eq("towel", "towel"));

    return true;
}

/*
 * All assertions pass in this test. Checks the value of the fixture initialized
 * in the test setup function.
 */
TEST_CASE(test_passing2)
{
    REQUIRE(42 == g_fix);
    REQUIRE(str_eq("frog", "frog"));

    return true;
}

/* Test containing failing assertion. */
TEST_CASE(test_failing1)
{
    REQUIRE(1);
    REQUIRE(24 == 42); /* Fails here */
    REQUIRE(1);        /* Never called */

    return true;
}

/* Another test containing a failed assertion. */
TEST_CASE(test_failing2)
{
    REQUIRE(str_eq("frog", "butterfly")); /* Fails here */
    REQUIRE(true);                        /* Never called */

    return true;
}

/* A test suite containing two passing and two failing tests. */
static void
test_suite1 ()
{
    pu_setup(test_setup, test_teardown);
    RUN_TEST_CASE(test_passing1);
    RUN_TEST_CASE(test_passing2);
    RUN_TEST_CASE(test_failing1);
    RUN_TEST_CASE(test_failing2);
    pu_clear_setup();
}

/* Run all test suites in quiet mode */
int
main ()
{
    pu_display_quiet(true); /* optional: only print failures. No stats or passing tests */
    RUN_TEST_SUITE(test_suite1);
    pu_print_stats(); /* prints nothing when quiet mode is enabled */
    return 0;
}

/* EoF */
