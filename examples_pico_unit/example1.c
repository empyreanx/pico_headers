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

/**
 * For a more complete demonstration, see example2.c
 */

#define PUNIT_IMPLEMENTATION
#include "../pico_unit.h"

/*
 * Passing test. Note that the test function declaration returns a boolean value
 * and that the test definition returns true. All test functions must return
 * true.
 */

PUNIT_TEST(test1)
{
    PUNIT_ASSERT(2 + 2 == 4);                /* Boolean assertion (ok)         */
    PUNIT_ASSERT_STREQ("apples", "apples");  /* String equality assertion (ok) */
    return true;
}

/* Failing test */
PUNIT_TEST(test2)
{
    PUNIT_ASSERT(2 + 2 != 4);                /* Boolean assertion (fails) */
    PUNIT_ASSERT_STREQ("apples", "oranges"); /* String equality (fails */
    return true;
}

/* Mixed results */
PUNIT_TEST(test3)
{
    PUNIT_ASSERT(2 + 2 == 4);                 /* Boolean assertion (ok) */
    PUNIT_ASSERT_STREQ("apples", "oranges");  /* String equality fails */
    return true;
}

/* Test suite container function (multiple test suites can be specified. */
static void
test_suite ()
{
    PUNIT_RUN_TEST(test1);
    PUNIT_RUN_TEST(test2);
    PUNIT_RUN_TEST(test3);
}

int
main ()
{
    PUNIT_RUN_SUITE(test_suite);
    punit_print_stats(); /* Optional */
    return 0;
}

/* EoF */
