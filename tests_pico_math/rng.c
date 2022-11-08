#include "../pico_math.h"
#include "../pico_unit.h"

// Test vector generated using:
// https://svn.mcs.anl.gov/repos/ZeptoOS/trunk/Selfish/detour/twister.c
static const uint32_t vector[] =
{
    3510405877, 4290933890, 2191955339,  564929546,  152112058, 4262624192,
    2687398418,  268830360, 1763988213,  578848526, 4212814465, 3596577449,
    4146913070,  950422373, 1908844540, 1452005258, 3029421110,  142578355,
    1583761762, 1816660702, 2530498888, 1339965000, 3874409922, 3044234909,
    1962617717, 2324289180,  310281170,  981016607,  908202274, 3371937721,
    2244849493,  675678546, 3196822098, 1040470160, 3059612017, 3055400130,
    2826830282, 2884538137, 3090587696, 2262235068, 3506294894, 2080537739,
    1636797501, 4292933080, 2037904983, 2465694618, 1249751105,   30084166,
     112252926, 1333718913,  880414402,  334691897, 3337628481,   17084333,
    1070118630, 2111543209, 1129029736, 2769716594,  198749844, 2123740404
};

TEST_CASE(test_random)
{
    pm_rng_t rng;

    pm_rng_seed(&rng, 4357);

    for (size_t i = 0; i < 6 * 10; i++)
    {
        REQUIRE(vector[i] == pm_random(&rng));
    }

    return true;
}

TEST_SUITE(suite_rng)
{
    RUN_TEST_CASE(test_random);
}
