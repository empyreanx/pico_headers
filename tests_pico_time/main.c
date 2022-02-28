#define PT_IMPLEMENTATION
#include "../pico_time.h"

#define PU_IMPLEMENTATION
#include "../pico_unit.h"

PU_TEST(test_sleep)
{
    ptime_t before = pt_now();
    pt_sleep(pt_from_sec(0.5));
    ptime_t after = pt_now();

    PU_ASSERT(pt_to_sec(after - before) >= 0.5);

    return true;
}

int main (int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    PU_RUN_TEST(test_sleep);

    return 0;
}
