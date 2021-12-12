/*=============================================================================
 * MIT License
 *
 * Copyright (c) 2020 James McLean
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
=============================================================================*/

#define PUNIT_IMPLEMENTATION
#include "../pico_unit.h"

static unsigned g_fix = 0;

/* Sets up fixture for (called before test). */
static void
test_setup ()
{
    g_fix = 42;
}

/* Resets fixture (called after test). */
void static
test_teardown ()
{
    g_fix = 0;
}

/* All assertions pass in this test. */
PU_TEST(test_passing1)
{
    PU_ASSERT(1);
    PU_ASSERT(42 == 42);
    PU_ASSERT_STREQ("towel", "towel");

    return true;
}

/*
 * All assertions pass in this test. Checks the value of the fixture initialized
 * in the test setup function.
 */
PU_TEST(test_passing2)
{
    PU_ASSERT(42 == g_fix);
    PU_ASSERT_STREQ("frog", "frog");

    return true;
}

/* Test containing failing assertion. */
PU_TEST(test_failing1)
{
    PU_ASSERT(1);
    PU_ASSERT(24 == 42); /* Fails here */
    PU_ASSERT(1);        /* Never called */

    return true;
}

/* Another test containing a failed assertion. */
PU_TEST(test_failing2)
{
    PU_ASSERT_STREQ("frog", "butterfly"); /* Fails here */
    PU_ASSERT(true);                      /* Never called */

    return true;
}

/* A test suite containing two passing and one failing test. */
static void
test_suite1 ()
{
    pu_setup(test_setup, test_teardown);

    PU_RUN_TEST(test_passing1);
    PU_RUN_TEST(test_passing2);
    PU_RUN_TEST(test_failing1);

    pu_clear_setup();
}

/* A test suite containing two passing and one failing test. */
static void
test_suite2 ()
{
    PU_RUN_TEST(test_passing1);
    PU_RUN_TEST(test_failing2);
    PU_RUN_TEST(test_passing1);
}

/* Run all test suites and print test statistics. */
int
main ()
{
    pu_display_colors(true);
    pu_display_time(true);
    PU_RUN_SUITE(test_suite1);
    PU_RUN_SUITE(test_suite2);
    pu_print_stats();
    return 0;
}

/* EoF */
