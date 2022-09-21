#include "../pico_math.h"
#include "../pico_unit.h"

PU_TEST(test_v2_equal)
{
    { // Should not be equal
        pm_v2 v1 = pm_v2_make(1, 2);
        pm_v2 v2 = pm_v2_make(3, 4);
        PU_ASSERT(!pm_v2_equal(v1, v2));
    }

    { // Should be equal
        pm_v2 v1 = pm_v2_make(1, 2);
        pm_v2 v2 = pm_v2_make(1, 2);
        PU_ASSERT(pm_v2_equal(v1, v2));
    }

    return true;
}

PU_TEST(test_v2_add)
{
    pm_v2 v1  = pm_v2_make(1, 2);
    pm_v2 v2  = pm_v2_make(3, 4);
    pm_v2 exp = pm_v2_make(1 + 3, 2 + 4);

    pm_v2 res = pm_v2_add(v1, v2);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_v2_scale)
{
    pm_v2 v   = pm_v2_make(1, 1);
    pm_v2 exp = pm_v2_make(2, 2);

    pm_v2 res = pm_v2_scale(v, 2);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_v2_sub)
{
    pm_v2 v1  = pm_v2_make(1, 2);
    pm_v2 v2  = pm_v2_make(3, 4);
    pm_v2 exp = pm_v2_make(1 - 3, 2 - 4);

    pm_v2 res = pm_v2_sub(v1, v2);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_v2_dot)
{
    pm_v2 v1 = pm_v2_make(1, 2);
    pm_v2 v2 = pm_v2_make(3, 4);

    pm_float exp = 1 * 3 + 2 * 4;

    PU_ASSERT(pm_equal(exp, pm_v2_dot(v1, v2)));

    return true;
}

PU_TEST(test_v2_len)
{
    pm_v2 v = pm_v2_make(1, 1);

    PU_ASSERT(pm_equal(sqrtf(2), pm_v2_len(v)));

    return true;
}

PU_TEST(test_v2_normalize)
{
    pm_v2 v = pm_v2_make(1, 1);

    v = pm_v2_normalize(v);

    PU_ASSERT(pm_equal(1, pm_v2_len(v)));

    return true;
}

PU_TEST(test_v2_perp)
{
    pm_v2 v1 = pm_v2_make(1, 2);
    pm_v2 v2 = pm_v2_make(1, 2);

    pm_v2 res = pm_v2_perp(v1);

    PU_ASSERT(pm_equal(0, pm_v2_dot(res, v2)));

    return true;
}

PU_TEST(test_v2_cross)
{
    pm_v2 v1 = pm_v2_make(2, 0);
    pm_v2 v2 = pm_v2_make(1, 1);

    { // Should be positive
        pm_float c = pm_v2_cross(v1, v2);

        c /= (2 * sqrtf(2));

        PU_ASSERT(pm_equal(c, pm_sin(PM_PI / 4)));
    }

    { // Should be negative
        pm_float c = pm_v2_cross(v2, v1);

        c /= (2 * sqrtf(2));

        PU_ASSERT(pm_equal(c, -pm_sin(PM_PI / 4)));
    }

    return true;
}

PU_TEST(test_v2_angle)
{
    pm_v2 v = pm_v2_make(1, 1);
    pm_float a = pm_v2_angle(v);
    PU_ASSERT(pm_equal(a, PM_PI / 4));

    return true;
}

PU_TEST(test_v2_proj)
{
    pm_v2 v1  = pm_v2_make(3, 2);
    pm_v2 v2  = pm_v2_make(2, 0);
    pm_v2 exp = pm_v2_make(3, 0);

    pm_v2 res = pm_v2_proj(v1, v2);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_v2_dist)
{
    pm_v2 v1 = pm_v2_make(0, 0);
    pm_v2 v2 = pm_v2_make(1, 1);
    pm_v2 v3 = pm_v2_make(2, 2);

    PU_ASSERT(pm_equal(0, pm_v2_dist(v1, v1)));
    PU_ASSERT(pm_equal(pm_sqrt(2), pm_v2_dist(v1, v2)));
    PU_ASSERT(pm_equal(pm_sqrt(2) * 2, pm_v2_dist(v1, v3)));
    PU_ASSERT(pm_equal(pm_sqrt(2) * 2, pm_v2_dist(v3, v1)));
    PU_ASSERT(pm_equal(pm_sqrt(2), pm_v2_dist(v2, v3)));

    return true;
}

PU_TEST(test_v2_lerp)
{
    pm_v2 v1 = pm_v2_make(1, 1);
    pm_v2 v2 = pm_v2_make(2, 2);

    { // Alpha 0
        pm_v2 exp = pm_v2_make(1, 1);
        pm_v2 res = pm_v2_lerp(v1, v2, 0.0f);
        PU_ASSERT(pm_v2_equal(res, exp));
    }

    { // Alpha 1.5
        pm_v2 exp = pm_v2_make(1.5f, 1.5f);
        pm_v2 res = pm_v2_lerp(v1, v2, 0.5f);
        PU_ASSERT(pm_v2_equal(res, exp));
    }

    { // Alpha 2.0
        pm_v2 exp = pm_v2_make(2, 2);
        pm_v2 res = pm_v2_lerp(v1, v2, 1.0f);
        PU_ASSERT(pm_v2_equal(res, exp));
    }

    return true;
}

PU_TEST(test_v2_polar)
{
    pm_v2 v = pm_v2_polar(PM_PI / 8, 3);

    PU_ASSERT(pm_equal(3, pm_v2_len(v)));
    PU_ASSERT(pm_equal(PM_PI / 8, pm_v2_angle(v)));

    return true;
}

PU_TEST(test_v2_min)
{
    pm_v2 v1 = pm_v2_make(1.0f, 4.0f);
    pm_v2 v2 = pm_v2_make(2.0f, 3.0f);

    pm_v2 res = pm_v2_min(v1, v2);
    pm_v2 exp = pm_v2_make(1.0f, 3.0f);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_v2_max)
{
    pm_v2 v1 = pm_v2_make(1.0f, 4.0f);
    pm_v2 v2 = pm_v2_make(2.0f, 3.0f);

    pm_v2 res = pm_v2_max(v1, v2);
    pm_v2 exp = pm_v2_make(2.0f, 4.0f);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_v2_floor)
{
    pm_v2 v = pm_v2_make(1.2f, -4.5f);

    pm_v2 res = pm_v2_floor(v);
    pm_v2 exp = pm_v2_make(1.0f, -5.0f);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_v2_ceil)
{
    pm_v2 v = pm_v2_make(1.2f, -4.5f);

    pm_v2 res = pm_v2_ceil(v);
    pm_v2 exp = pm_v2_make(2.0f, -4.0f);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_SUITE(suite_v2)
{
    PU_RUN_TEST(test_v2_equal);
    PU_RUN_TEST(test_v2_add);
    PU_RUN_TEST(test_v2_scale);
    PU_RUN_TEST(test_v2_sub);
    PU_RUN_TEST(test_v2_dot);
    PU_RUN_TEST(test_v2_len);
    PU_RUN_TEST(test_v2_normalize);
    PU_RUN_TEST(test_v2_perp);
    PU_RUN_TEST(test_v2_cross);
    PU_RUN_TEST(test_v2_angle);
    PU_RUN_TEST(test_v2_proj);
    PU_RUN_TEST(test_v2_dist);
    PU_RUN_TEST(test_v2_lerp);
    PU_RUN_TEST(test_v2_polar);
    PU_RUN_TEST(test_v2_min);
    PU_RUN_TEST(test_v2_max);
    PU_RUN_TEST(test_v2_floor);
    PU_RUN_TEST(test_v2_ceil);
}
