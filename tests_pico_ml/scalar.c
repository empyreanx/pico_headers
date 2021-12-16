#include "../pico_ml.h"
#include "../pico_unit.h"

PU_TEST(test_min)
{
    PU_ASSERT(2 == pm_min(2, 4));
    return true;
}

PU_TEST(test_max)
{
    PU_ASSERT(4 == pm_max(2, 4));
    return true;
}

PU_TEST(test_clamp)
{
    PU_ASSERT(-5 == pm_clamp(-10, -5, 5));
    PU_ASSERT( 5 == pm_clamp(10, -5, 5));
    PU_ASSERT( 0 == pm_clamp(0, -5, 5));

    return true;
}

PU_TEST(test_next_pow2)
{
    PU_ASSERT(16 == pm_next_pow2(9));
    PU_ASSERT(16 == pm_next_pow2(8));
    PU_ASSERT(16 == pm_next_pow2(15));

    return true;
}

PU_TEST(test_lerp_angle)
{
    pm_flt angle = pm_lerp_angle(0.0f, PM_PI / 4.0, 0.5f);
    PU_ASSERT(pm_equal(angle, PM_PI / 8.0f));

    angle = pm_lerp_angle(PM_PI / 4.0, PM_PI * 3.0 / 4.0, 0.5f);
    PU_ASSERT(pm_equal(angle, PM_PI / 2.0f));

    angle = pm_lerp_angle(PM_PI / 4.0, PM_PI * 3.0 / 4.0, 0.0f);
    PU_ASSERT(pm_equal(angle, PM_PI / 4.0f));

    angle = pm_lerp_angle(PM_PI / 4.0, PM_PI * 3.0 / 4.0, 1.0f);
    PU_ASSERT(pm_equal(angle, PM_PI * 3.0 / 4.0));

    //NOTE: This pathological
    //angle = pm_lerp_angle(PM_PI / 4.0f, PM_PI * 7.0f / 4.0f, 0.5);
    //PU_ASSERT(pm_equal(angle, 0.0f));

    angle = pm_lerp_angle(PM_PI / 4.0f, PM_PI * 7.0f / 4.0f, 0.5);
    PU_ASSERT(pm_equal(angle, PM_PI));

    angle = pm_lerp_angle(PM_PI * 7.0f / 4.0f, PM_PI / 4.0f, 0.75);
    PU_ASSERT(pm_equal(angle, PM_PI / 8.0f));

    return true;
}

PU_SUITE(suite_scalar)
{
    PU_RUN_TEST(test_min);
    PU_RUN_TEST(test_max);
    PU_RUN_TEST(test_clamp);
    PU_RUN_TEST(test_next_pow2);
    PU_RUN_TEST(test_lerp_angle);
}
