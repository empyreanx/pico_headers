#define PICO_QT_USE_UINTPTR
#define PICO_QT_IMPLEMENTATION
#include "../pico_qt.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#include <stdlib.h>
#include <math.h>

// Helper functions
static int cmp_values(const void * a, const void * b);
static void sort_values(qt_value_t* values, int size);
static int random_int(int min, int max);

static qt_t* qt;
static qt_value_t* values;

TEST_CASE(test_insert_single)
{
    qt_insert(qt, qt_make_rect(-5, -5, 10, 10), 0);

    int size;

    // Found
    {
        values = qt_query(qt, qt_make_rect(-7, -7, 5, 5), &size);

        REQUIRE(size == 1);
        REQUIRE(values[0] == 0);

        qt_free(qt, values);
    }

    // Not found
    {
        values = qt_query(qt, qt_make_rect(6, 6, 5, 5), &size);

        REQUIRE(size == 0);
        REQUIRE(values == NULL);

        qt_free(qt, values);
    }

    return true;
}

TEST_CASE(test_insert_single_contained)
{
    qt_insert(qt, qt_make_rect(-5, -5, 3, 3), 0);

    int size;

    // Found
    {
        values = qt_query(qt, qt_make_rect(-7, -7, 7, 7), &size);

        REQUIRE(size == 1);
        REQUIRE(values[0] == 0);

        qt_free(qt, values);
    }

    // Not found
    {
        values = qt_query(qt, qt_make_rect(5, 5, 5, 5), &size);

        REQUIRE(size == 0);
        REQUIRE(values == NULL);

        qt_free(qt, values);
    }

    return true;
}

TEST_CASE(test_insert_multiple)
{
    qt_insert(qt, qt_make_rect(-7, -7, 2, 2), 0);
    qt_insert(qt, qt_make_rect(-5, -5, 3, 3), 1);
    qt_insert(qt, qt_make_rect(-3, -5, 4, 4), 2);
    qt_insert(qt, qt_make_rect( 3,  3, 3, 5), 3);

    int size;

    values = qt_query(qt, qt_make_rect(-6, -6, 5, 5), &size);

    REQUIRE(size == 3);

    sort_values(values, size);

    REQUIRE(values[0] == 0);
    REQUIRE(values[1] == 1);
    REQUIRE(values[2] == 2);

    qt_free(qt, values);

    return true;
}

TEST_CASE(test_insert_multiple_random)
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

    REQUIRE(size == 32);

    sort_values(values, size);

    for (int i = 0; i < size; i++)
    {
        REQUIRE(values[i] == (qt_value_t)i);
    }

    qt_free(qt, values);

    return true;
}

TEST_CASE(test_insert_multiple_random_contained)
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

        REQUIRE(size == 8);

        sort_values(values, size);

        for (int i = 0; i < size; i++)
        {
            REQUIRE(values[i] == (qt_value_t)i);
        }

        qt_free(qt, values);
    }

    // Not found
    {
        values = qt_query(qt, qt_make_rect(-7, -7, 3, 3), &size);

        REQUIRE(size == 0);
        REQUIRE(values == NULL);

        qt_free(qt, values);
    }

    return true;
}

TEST_CASE(test_remove)
{
    qt_insert(qt, qt_make_rect(-3, -3, 2, 2), 0);
    qt_insert(qt, qt_make_rect( 5,  5, 3, 3), 1);
    qt_insert(qt, qt_make_rect( 3, -5, 4, 3), 2);
    qt_insert(qt, qt_make_rect(-5,  3, 3, 5), 3);

    qt_remove(qt, 0);
    qt_remove(qt, 1);

    int size;

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);
    qt_free(qt, values);

    REQUIRE(size == 2);

    qt_remove(qt, 2);
    qt_remove(qt, 3);

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);

    REQUIRE(size == 0);
    REQUIRE(values == NULL);

    qt_free(qt, values);

    return true;
}

TEST_CASE(test_reset)
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

    REQUIRE(size == 32);

    qt_free(qt, values);

    qt_reset(qt);

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);

    REQUIRE(size == 0);
    REQUIRE(values == NULL);

    qt_free(qt, values);

    return true;
}

TEST_CASE(test_clear)
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

    REQUIRE(size == 32);

    qt_free(qt, values);

    qt_clear(qt);

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);

    REQUIRE(size == 0);
    REQUIRE(values == NULL);

    qt_free(qt, values);

    return true;
}

TEST_CASE(test_clean)
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

    REQUIRE(size == 32);

    qt_free(qt, values);

    qt_clean(qt);

    values = qt_query(qt, qt_make_rect(-10, -10, 20, 20), &size);

    REQUIRE(size == 32);

    qt_free(qt, values);

    return true;
}

static bool rect_in_array(qt_rect_t* rects, int size, qt_rect_t rect)
{
    static const float EPSILON = 1e-5f;

    for (int i = 0; i < size; i++)
    {
        if (fabsf(rects[i].x - rect.x) < EPSILON &&
            fabsf(rects[i].y - rect.y) < EPSILON &&
            fabsf(rects[i].w - rect.w) < EPSILON &&
            fabsf(rects[i].h - rect.h) < EPSILON)
        {
            return true;
        }
    }

    return false;
}

TEST_CASE(test_grid_rects)
{
    qt_insert(qt, qt_make_rect(-5, -5, 3, 3), 0);
    qt_insert(qt, qt_make_rect(5, 5, 3, 3), 1);

    qt_insert(qt, qt_make_rect(0, -5, 5, 5), 2);
    qt_insert(qt, qt_make_rect(0, -5, 2.5f, 2.5f), 3);

    qt_insert(qt, qt_make_rect(-3, 3, 4, 4), 4);
    qt_insert(qt, qt_make_rect(-5, -5, 10, 10), 5);

    int size;
    qt_rect_t* rects = qt_grid_rects(qt, &size);

    REQUIRE(size == 7);

    REQUIRE(rect_in_array(rects, size, qt_make_rect(-10, -10, 10, 10)));
    REQUIRE(rect_in_array(rects, size, qt_make_rect(-5, -5, 5, 5)));

    REQUIRE(rect_in_array(rects, size, qt_make_rect(0, 0, 10, 10)));
    REQUIRE(rect_in_array(rects, size, qt_make_rect(5, 5, 5, 5)));

    REQUIRE(rect_in_array(rects, size, qt_make_rect(0, -10, 10, 10)));
    REQUIRE(rect_in_array(rects, size, qt_make_rect(0, -5, 5, 5)));

    qt_free(qt, rects);

    return true;
}

static TEST_SUITE(suite_qt)
{
    RUN_TEST_CASE(test_insert_single);
    RUN_TEST_CASE(test_insert_single_contained);
    RUN_TEST_CASE(test_insert_multiple);
    RUN_TEST_CASE(test_insert_multiple_random);
    RUN_TEST_CASE(test_insert_multiple_random_contained);
    RUN_TEST_CASE(test_remove);
    RUN_TEST_CASE(test_reset);
    RUN_TEST_CASE(test_clear);
    RUN_TEST_CASE(test_clean);
    RUN_TEST_CASE(test_grid_rects);
}

void setup(void)
{
    qt = qt_create(qt_make_rect(-10, -10, 20, 20), 6, NULL);
}

void teardown(void)
{
    qt_destroy(qt);
    qt = NULL;
}

int main()
{
    pu_display_colors(true);
    pu_setup(setup, teardown);
    RUN_TEST_SUITE(suite_qt);
    pu_print_stats();
    return pu_test_failed();
}

static int cmp_values(const void * a, const void * b)
{
   return (int)( *(qt_value_t*)a - *(qt_value_t*)b );
}

static void sort_values(qt_value_t* values, int size)
{
   qsort(values, size, sizeof(qt_value_t), cmp_values);
}

static int random_int(int min, int max)
{
    return rand() % (max + 1 - min) + min;
}

