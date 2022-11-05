#include "../pico_qt.h"
#include "../pico_unit.h"

#include <stdlib.h>

// Helper functions
static int cmp_values(const void * a, const void * b);
static void sort_values(qt_value_t* values, int size);
static int random_int(int min, int max);

qt_t* qt;
qt_value_t* values;

PU_TEST(test_insert_single_contained)
{
    qt_insert(qt, qt_make_rect(-5, -5, 3, 3), 0);

    int size;

    // Found
    {
        values = qt_query(qt, qt_make_rect(-7, -7, 5, 5), &size);

        PU_ASSERT(size == 1);
        PU_ASSERT(values[0] == 0);
    }

    // Not found
    {
        values = qt_query(qt, qt_make_rect(5, 5, 5, 5), &size);

        PU_ASSERT(size == 0);
    }

    return true;
}

PU_TEST(test_insert_single_no_fit)
{
    qt_insert(qt, qt_make_rect(-5, -5, 10, 10), 0);

    int size;

    // Found
    {
        values = qt_query(qt, qt_make_rect(-7, -7, 5, 5), &size);

        PU_ASSERT(size == 1);
        PU_ASSERT(values[0] == 0);

        qt_free(values);
    }

    // Not found
    {
        values = qt_query(qt, qt_make_rect(6, 6, 5, 5), &size);

        PU_ASSERT(size == 0);
    }

    return true;
}

PU_TEST(test_insert_multiple_random)
{
    srand(42);

    for (int i = 0; i < 32; i++)
    {
        int x = random_int(-9, 9);
        int y = random_int(-9, 9);
        int w = random_int( 1, 10 - x);
        int h = random_int( 1, 10 - y);
        qt_insert(qt, qt_make_rect(x, y, w, h), i);
    }

    int size;

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);

    PU_ASSERT(size == 32);

    sort_values(values, size);

    for (int i = 0; i < size; i++)
    {
        PU_ASSERT(values[i] == (qt_value_t)i);
    }

    return true;
}

PU_TEST(test_insert_multiple_random_quadrant)
{
    srand(42);

    for (int i = 0; i < 8; i++)
    {
        int x = random_int(1, 5);
        int y = random_int(1, 5);
        int w = random_int(1, 10 - x);
        int h = random_int(1, 10 - y);
        qt_insert(qt, qt_make_rect(x, y, w, h), i);
    }

    int size;

    // Found
    {
        values = qt_query(qt, qt_make_rect(-1, -1, 11, 11), &size);

        PU_ASSERT(size == 8);

        sort_values(values, size);

        for (int i = 0; i < size; i++)
        {
            PU_ASSERT(values[i] == (qt_value_t)i);
        }

        qt_free(values);
    }

    // Not found
    {
        values = qt_query(qt, qt_make_rect(-7, -7, 3, 3), &size);

        PU_ASSERT(size == 0);
    }

    return true;
}

PU_TEST(test_reset_multiple_random)
{
    srand(42);

    for (int i = 0; i < 32; i++)
    {
        int x = random_int(-9, 9);
        int y = random_int(-9, 9);
        int w = random_int( 1, 10 - x);
        int h = random_int( 1, 10 - y);
        qt_insert(qt, qt_make_rect(x, y, w, h), i);
    }

    int size;

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);

    PU_ASSERT(size == 32);

    qt_free(values);

    qt_reset(qt);

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);

    PU_ASSERT(size == 0);

    return true;
}

static PU_SUITE(suite_qt)
{
    PU_RUN_TEST(test_insert_single_contained);
    PU_RUN_TEST(test_insert_single_no_fit);
    PU_RUN_TEST(test_insert_multiple_random);
    PU_RUN_TEST(test_insert_multiple_random_quadrant);
    PU_RUN_TEST(test_reset_multiple_random);
}

void setup()
{
    qt = qt_create(qt_make_rect(-10, -10, 20, 20));
    values = NULL;
}

void teardown()
{
    qt_destroy(qt);
    qt_free(values);
    qt = NULL;
}

int main()
{
    pu_display_colors(true);
    pu_setup(setup, teardown);
    PU_RUN_SUITE(suite_qt);
    pu_print_stats();
    return pu_test_failed();
}

static int cmp_values(const void * a, const void * b)
{
   return ( *(qt_value_t*)a - *(qt_value_t*)b );
}

static void sort_values(qt_value_t* values, int size)
{
   qsort(values, size, sizeof(qt_value_t), cmp_values);
}

static int random_int(int min, int max)
{
    return rand() % (max + 1 - min) + min;
}

#define PICO_QT_IMPLEMENTATION
#include "../pico_qt.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"
