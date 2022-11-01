#include "../pico_unit.h"

static PU_SUITE(suite_qt)
{

}

int main()
{
    pu_display_colors(true);
    PU_RUN_SUITE(suite_qt);
    pu_print_stats();
    return pu_test_failed();
}

#define PICO_SAT_IMPLEMENTATION
#include "../pico_qt.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"
