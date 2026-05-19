#define EVENTEMITTER_IMPLEMENTATION
#include "../pico_emitter.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

int main()
{
    pu_display_colors(true);
    pu_print_stats();
    return pu_test_failed();
}
