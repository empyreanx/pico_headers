#include "../pico_math.h"
#include "../pico_unit.h"

TEST_CASE(test_v2_equal)
{
    { // Should not be equal
        pv2 v1 = pv2_make(1, 2);
        pv2 v2 = pv2_make(3, 4);
        REQUIRE(!pv2_equal(v1, v2));
    }

    { // Should be equal
        pv2 v1 = pv2_make(1, 2);
        pv2 v2 = pv2_make(1, 2);
        REQUIRE(pv2_equal(v1, v2));
    }

    return true;
}

TEST_CASE(test_v2_add)
{
    pv2 v1  = pv2_make(1, 2);
    pv2 v2  = pv2_make(3, 4);
    pv2 exp = pv2_make(1 + 3, 2 + 4);

    pv2 res = pv2_add(v1, v2);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_scale)
{
    pv2 v   = pv2_make(1, 1);
    pv2 exp = pv2_make(2, 2);

    pv2 res = pv2_scale(v, 2);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_sub)
{
    pv2 v1  = pv2_make(1, 2);
    pv2 v2  = pv2_make(3, 4);
    pv2 exp = pv2_make(1 - 3, 2 - 4);

    pv2 res = pv2_sub(v1, v2);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_dot)
{
    pv2 v1 = pv2_make(1, 2);
    pv2 v2 = pv2_make(3, 4);

    pfloat exp = 1 * 3 + 2 * 4;

    REQUIRE(pf_equal(exp, pv2_dot(v1, v2)));

    return true;
}

TEST_CASE(test_v2_len)
{
    pv2 v = pv2_make(1, 1);

    REQUIRE(pf_equal(sqrtf(2), pv2_len(v)));

    return true;
}

TEST_CASE(test_v2_normalize)
{
    pv2 v = pv2_make(1, 1);

    v = pv2_normalize(v);

    REQUIRE(pf_equal(1, pv2_len(v)));

    return true;
}

TEST_CASE(test_v2_reflect)
{
    pv2 v = pv2_make(1, -1);
    pv2 exp = pv2_make(-1, 1);

    pv2 res = pv2_reflect(v);

    REQUIRE(pv2_equal(exp, res));

    return true;
}

TEST_CASE(test_v2_perp)
{
    pv2 v1 = pv2_make(1, 2);
    pv2 v2 = pv2_make(1, 2);

    pv2 res = pv2_perp(v1);

    REQUIRE(pf_equal(0, pv2_dot(res, v2)));

    return true;
}

TEST_CASE(test_v2_cross)
{
    pv2 v1 = pv2_make(2, 0);
    pv2 v2 = pv2_make(1, 1);

    { // Should be positive
        pfloat c = pv2_cross(v1, v2);

        c /= (2 * sqrtf(2));

        REQUIRE(pf_equal(c, pf_sin(PM_PI / 4)));
    }

    { // Should be negative
        pfloat c = pv2_cross(v2, v1);

        c /= (2 * sqrtf(2));

        REQUIRE(pf_equal(c, -pf_sin(PM_PI / 4)));
    }

    return true;
}

TEST_CASE(test_v2_angle)
{
    pv2 v = pv2_make(1, 1);
    pfloat a = pv2_angle(v);
    REQUIRE(pf_equal(a, PM_PI / 4));

    return true;
}

TEST_CASE(test_v2_proj)
{
    pv2 v1  = pv2_make(3, 2);
    pv2 v2  = pv2_make(2, 0);
    pv2 exp = pv2_make(3, 0);

    pv2 res = pv2_proj(v1, v2);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_dist)
{
    pv2 v1 = pv2_make(0, 0);
    pv2 v2 = pv2_make(1, 1);
    pv2 v3 = pv2_make(2, 2);

    REQUIRE(pf_equal(0, pv2_dist(v1, v1)));
    REQUIRE(pf_equal(pf_sqrt(2), pv2_dist(v1, v2)));
    REQUIRE(pf_equal(pf_sqrt(2) * 2, pv2_dist(v1, v3)));
    REQUIRE(pf_equal(pf_sqrt(2) * 2, pv2_dist(v3, v1)));
    REQUIRE(pf_equal(pf_sqrt(2), pv2_dist(v2, v3)));

    return true;
}

TEST_CASE(test_v2_lerp)
{
    pv2 v1 = pv2_make(1, 1);
    pv2 v2 = pv2_make(2, 2);

    { // Alpha 0
        pv2 exp = pv2_make(1, 1);
        pv2 res = pv2_lerp(v1, v2, 0.0f);
        REQUIRE(pv2_equal(res, exp));
    }

    { // Alpha 1.5
        pv2 exp = pv2_make(1.5f, 1.5f);
        pv2 res = pv2_lerp(v1, v2, 0.5f);
        REQUIRE(pv2_equal(res, exp));
    }

    { // Alpha 2.0
        pv2 exp = pv2_make(2, 2);
        pv2 res = pv2_lerp(v1, v2, 1.0f);
        REQUIRE(pv2_equal(res, exp));
    }

    return true;
}

TEST_CASE(test_v2_polar)
{
    pv2 v = pv2_polar(PM_PI / 8, 3);

    REQUIRE(pf_equal(3, pv2_len(v)));
    REQUIRE(pf_equal(PM_PI / 8, pv2_angle(v)));

    return true;
}

TEST_CASE(test_v2_min)
{
    pv2 v1 = pv2_make(1.0f, 4.0f);
    pv2 v2 = pv2_make(2.0f, 3.0f);

    pv2 res = pv2_min(v1, v2);
    pv2 exp = pv2_make(1.0f, 3.0f);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_max)
{
    pv2 v1 = pv2_make(1.0f, 4.0f);
    pv2 v2 = pv2_make(2.0f, 3.0f);

    pv2 res = pv2_max(v1, v2);
    pv2 exp = pv2_make(2.0f, 4.0f);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_floor)
{
    pv2 v = pv2_make(1.2f, -4.5f);

    pv2 res = pv2_floor(v);
    pv2 exp = pv2_make(1.0f, -5.0f);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_v2_ceil)
{
    pv2 v = pv2_make(1.2f, -4.5f);

    pv2 res = pv2_ceil(v);
    pv2 exp = pv2_make(2.0f, -4.0f);

    REQUIRE(pv2_equal(res, exp));

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
