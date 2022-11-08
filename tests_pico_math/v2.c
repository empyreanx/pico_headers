#include "../pico_math.h"
#include "../pico_unit.h"

TEST_CASE(test_v2_equal)
{
    { // Should not be equal
        pm_v2 v1 = pm_v2_make(1, 2);
        pm_v2 v2 = pm_v2_make(3, 4);
        REQUIRE(!pm_v2_equal(v1, v2));
    }

    { // Should be equal
        pm_v2 v1 = pm_v2_make(1, 2);
        pm_v2 v2 = pm_v2_make(1, 2);
        REQUIRE(pm_v2_equal(v1, v2));
    }

    return true;
}

TEST_CASE(test_v2_add)
{
    pm_v2 v1  = pm_v2_make(1, 2);
    pm_v2 v2  = pm_v2_make(3, 4);
    pm_v2 exp = pm_v2_make(1 + 3, 2 + 4);

    pm_v2 res = pm_v2_add(v1, v2);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_scale)
{
    pm_v2 v   = pm_v2_make(1, 1);
    pm_v2 exp = pm_v2_make(2, 2);

    pm_v2 res = pm_v2_scale(v, 2);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_sub)
{
    pm_v2 v1  = pm_v2_make(1, 2);
    pm_v2 v2  = pm_v2_make(3, 4);
    pm_v2 exp = pm_v2_make(1 - 3, 2 - 4);

    pm_v2 res = pm_v2_sub(v1, v2);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_dot)
{
    pm_v2 v1 = pm_v2_make(1, 2);
    pm_v2 v2 = pm_v2_make(3, 4);

    pm_float exp = 1 * 3 + 2 * 4;

    REQUIRE(pm_equal(exp, pm_v2_dot(v1, v2)));

    return true;
}

TEST_CASE(test_v2_len)
{
    pm_v2 v = pm_v2_make(1, 1);

    REQUIRE(pm_equal(sqrtf(2), pm_v2_len(v)));

    return true;
}

TEST_CASE(test_v2_normalize)
{
    pm_v2 v = pm_v2_make(1, 1);

    v = pm_v2_normalize(v);

    REQUIRE(pm_equal(1, pm_v2_len(v)));

    return true;
}

TEST_CASE(test_v2_reflect)
{
    pm_v2 v = pm_v2_make(1, -1);
    pm_v2 exp = pm_v2_make(-1, 1);

    pm_v2 res = pm_v2_reflect(v);

    REQUIRE(pm_v2_equal(exp, res));

    return true;
}

TEST_CASE(test_v2_perp)
{
    pm_v2 v1 = pm_v2_make(1, 2);
    pm_v2 v2 = pm_v2_make(1, 2);

    pm_v2 res = pm_v2_perp(v1);

    REQUIRE(pm_equal(0, pm_v2_dot(res, v2)));

    return true;
}

TEST_CASE(test_v2_cross)
{
    pm_v2 v1 = pm_v2_make(2, 0);
    pm_v2 v2 = pm_v2_make(1, 1);

    { // Should be positive
        pm_float c = pm_v2_cross(v1, v2);

        c /= (2 * sqrtf(2));

        REQUIRE(pm_equal(c, pm_sin(PM_PI / 4)));
    }

    { // Should be negative
        pm_float c = pm_v2_cross(v2, v1);

        c /= (2 * sqrtf(2));

        REQUIRE(pm_equal(c, -pm_sin(PM_PI / 4)));
    }

    return true;
}

TEST_CASE(test_v2_angle)
{
    pm_v2 v = pm_v2_make(1, 1);
    pm_float a = pm_v2_angle(v);
    REQUIRE(pm_equal(a, PM_PI / 4));

    return true;
}

TEST_CASE(test_v2_proj)
{
    pm_v2 v1  = pm_v2_make(3, 2);
    pm_v2 v2  = pm_v2_make(2, 0);
    pm_v2 exp = pm_v2_make(3, 0);

    pm_v2 res = pm_v2_proj(v1, v2);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_dist)
{
    pm_v2 v1 = pm_v2_make(0, 0);
    pm_v2 v2 = pm_v2_make(1, 1);
    pm_v2 v3 = pm_v2_make(2, 2);

    REQUIRE(pm_equal(0, pm_v2_dist(v1, v1)));
    REQUIRE(pm_equal(pm_sqrt(2), pm_v2_dist(v1, v2)));
    REQUIRE(pm_equal(pm_sqrt(2) * 2, pm_v2_dist(v1, v3)));
    REQUIRE(pm_equal(pm_sqrt(2) * 2, pm_v2_dist(v3, v1)));
    REQUIRE(pm_equal(pm_sqrt(2), pm_v2_dist(v2, v3)));

    return true;
}

TEST_CASE(test_v2_lerp)
{
    pm_v2 v1 = pm_v2_make(1, 1);
    pm_v2 v2 = pm_v2_make(2, 2);

    { // Alpha 0
        pm_v2 exp = pm_v2_make(1, 1);
        pm_v2 res = pm_v2_lerp(v1, v2, 0.0f);
        REQUIRE(pm_v2_equal(res, exp));
    }

    { // Alpha 1.5
        pm_v2 exp = pm_v2_make(1.5f, 1.5f);
        pm_v2 res = pm_v2_lerp(v1, v2, 0.5f);
        REQUIRE(pm_v2_equal(res, exp));
    }

    { // Alpha 2.0
        pm_v2 exp = pm_v2_make(2, 2);
        pm_v2 res = pm_v2_lerp(v1, v2, 1.0f);
        REQUIRE(pm_v2_equal(res, exp));
    }

    return true;
}

TEST_CASE(test_v2_polar)
{
    pm_v2 v = pm_v2_polar(PM_PI / 8, 3);

    REQUIRE(pm_equal(3, pm_v2_len(v)));
    REQUIRE(pm_equal(PM_PI / 8, pm_v2_angle(v)));

    return true;
}

TEST_CASE(test_v2_min)
{
    pm_v2 v1 = pm_v2_make(1.0f, 4.0f);
    pm_v2 v2 = pm_v2_make(2.0f, 3.0f);

    pm_v2 res = pm_v2_min(v1, v2);
    pm_v2 exp = pm_v2_make(1.0f, 3.0f);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_max)
{
    pm_v2 v1 = pm_v2_make(1.0f, 4.0f);
    pm_v2 v2 = pm_v2_make(2.0f, 3.0f);

    pm_v2 res = pm_v2_max(v1, v2);
    pm_v2 exp = pm_v2_make(2.0f, 4.0f);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_floor)
{
    pm_v2 v = pm_v2_make(1.2f, -4.5f);

    pm_v2 res = pm_v2_floor(v);
    pm_v2 exp = pm_v2_make(1.0f, -5.0f);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_ceil)
{
    pm_v2 v = pm_v2_make(1.2f, -4.5f);

    pm_v2 res = pm_v2_ceil(v);
    pm_v2 exp = pm_v2_make(2.0f, -4.0f);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_SUITE(suite_v2)
{
    RUN_TEST_CASE(test_v2_equal);
    RUN_TEST_CASE(test_v2_add);
    RUN_TEST_CASE(test_v2_scale);
    RUN_TEST_CASE(test_v2_sub);
    RUN_TEST_CASE(test_v2_dot);
    RUN_TEST_CASE(test_v2_len);
    RUN_TEST_CASE(test_v2_normalize);
    RUN_TEST_CASE(test_v2_reflect);
    RUN_TEST_CASE(test_v2_perp);
    RUN_TEST_CASE(test_v2_cross);
    RUN_TEST_CASE(test_v2_angle);
    RUN_TEST_CASE(test_v2_proj);
    RUN_TEST_CASE(test_v2_dist);
    RUN_TEST_CASE(test_v2_lerp);
    RUN_TEST_CASE(test_v2_polar);
    RUN_TEST_CASE(test_v2_min);
    RUN_TEST_CASE(test_v2_max);
    RUN_TEST_CASE(test_v2_floor);
    RUN_TEST_CASE(test_v2_ceil);
}
