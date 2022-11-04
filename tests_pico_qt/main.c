#include "../pico_qt.h"
#include "../pico_unit.h"

#include <stdlib.h>

qt_t* qt;
qt_value_t* values;

static int cmp_values(const void * a, const void * b);
static void sort_values(qt_value_t* values, int size);
static int random_int(int min, int max);

PU_TEST(test_insert_single_contained)
{
    qt_insert(qt, qt_make_rect(-5, -5, 3, 3), 0);

    int size;

    values = qt_query(qt, qt_make_rect(-7, -7, 5, 5), &size);

    PU_ASSERT(size == 1);
    PU_ASSERT(values[0] == 0);

    return true;
}

PU_TEST(test_insert_single_no_fit)
{
    qt_insert(qt, qt_make_rect(-5, -5, 10, 10), 0);

    int size;

    values = qt_query(qt, qt_make_rect(-7, -7, 5, 5), &size);

    PU_ASSERT(size == 1);
    PU_ASSERT(values[0] == 0);

    return true;
}

static PU_SUITE(suite_qt)
{
    PU_RUN_TEST(test_insert_single_contained);
    PU_RUN_TEST(test_insert_single_no_fit);
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
