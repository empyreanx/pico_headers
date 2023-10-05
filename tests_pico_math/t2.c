#include "../pico_math.h"
#include "../pico_unit.h"

#include <stdio.h>

TEST_CASE(test_t2_equal)
{
    { // Should be equal
        pt2 t = pt2_make(2, 3, 3, 2, 1, 1);
        REQUIRE(pt2_equal(&t, &t));
    }

    { // Should not be equal
        pt2 t1 = pt2_make(2, 3, 3, 2, 1, 1);
        pt2 t2 = pt2_make(2, 3, 3, 5, 1, 1);
        REQUIRE(!pt2_equal(&t1, &t2));
    }

    return true;
}

TEST_CASE(test_t2_identity)
{
    pt2 exp = pt2_make(1, 0, 0,
                       0, 1, 0);

    pt2 res = pt2_identity();
    REQUIRE(pt2_equal(&res, &exp));

    return true;
}

TEST_CASE(test_t2_get_pos)
{
    pt2 t = pt2_make(1, 1, 2, 0, 0, 3);
    pv2 exp = pv2_make(2, 3);

    pv2 res = pt2_get_pos(&t);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_t2_set_pos)
{
    pt2 t = pt2_identity();
    pv2 pos = pv2_make(2, 3);
    pv2 exp = pv2_make(2, 3);

    pt2_set_pos(&t, pos);
    pv2 res = pt2_get_pos(&t);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_t2_get_angle)
{
    pt2 t = pt2_rotation(PM_PI / 8.0f);
    REQUIRE(pf_equal(pt2_get_angle(&t), PM_PI / 8.0f));

    t = pt2_rotation(PM_PI / 2.0f);
    REQUIRE(pf_equal(pt2_get_angle(&t), PM_PI / 2.0f));

    return true;
}

TEST_CASE(test_t2_get_scale)
{
    { // Identity
        pt2 t1 = pt2_scaling(pv2_make(2, 3));
        pv2 res = pt2_get_scale(&t1);
        pv2 exp = pv2_make(2, 3);
        REQUIRE(pv2_equal(res, exp));
    }

    { // Acute angle
        pt2 t1 = pt2_scaling(pv2_make(2, 3));
        pt2 t2 = pt2_rotation(PM_PI / 4.0f);
        pt2 t3 = pt2_mult(&t2, &t1);
        pv2 exp = pv2_make(2, 3);
        pv2 res = pt2_get_scale(&t3);
        REQUIRE(pv2_equal(res, exp));
    }

    { // Obtuse angle
        pt2 t1 = pt2_scaling(pv2_make(2, 3));
        pt2 t2 = pt2_rotation(PM_PI * 3.0/ 4.0f);
        pt2 t3 = pt2_mult(&t2, &t1);
        pv2 exp = pv2_make(2, 3);
        pv2 res = pt2_get_scale(&t3);
        REQUIRE(pv2_equal(res, exp));
    }

    {
        pt2 t1 = pt2_scaling(pv2_make(2, 2));
        pt2 t2 = pt2_rotation(PM_PI / 2.0f);
        pt2 t3 = pt2_mult(&t2, &t1);
        pv2 exp = pv2_make(2, 2);
        pv2 res = pt2_get_scale(&t3);
        REQUIRE(pv2_equal(res, exp));
    }

    return true;
}

TEST_CASE(test_t2_set_angle)
{
    pt2 t1 = pt2_scaling(pv2_make(2, 3));
    pt2 t2 = pt2_rotation(PM_PI / 2.0f);
    pt2 t3 = pt2_mult(&t2, &t1);

    { // Case 0
        pt2_set_angle(&t3, PM_PI / 8.0f);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI / 8.0f));
    }

    { // Case 1
        pt2_set_angle(&t3, PM_PI / 4.0f);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI / 4.0f));
    }

    { // Case 2
        pt2_set_angle(&t3, PM_PI * 3.0f / 8.0f);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI * 3.0f / 8.0f));
    }

    { // Case 3
        pt2_set_angle(&t3, PM_PI * 7.0f / 8.0f);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI * 7.0f / 8.0f));
    }

    { // Case 4
        pt2_set_angle(&t3, PM_PI / 2.0f);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI / 2.0f));
    }

    { // Case 5
        pt2_set_angle(&t3, PM_PI);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI));
    }

    { // Case 6
        pt2_set_angle(&t3, PM_PI * 3.0f / 4.0f);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI * 3.0f / 4.0f));
    }

    { // Case 9
        pt2_set_angle(&t3, PM_PI * 9.0f / 8.0f);
        pfloat angle = pt2_get_angle(&t3);
        REQUIRE(pf_equal(angle, PM_PI * 9.0f / 8.0f));
    }

    return true;
}

TEST_CASE(test_t2_map)
{
    pt2 t1 = pt2_rotation(PM_PI / 4.0f);
    pt2 t2 = pt2_scaling(pv2_make(2, 2));
    pt2 t3 = pt2_mult(&t1, &t2);

    pv2 v = pv2_make(1, 0);

    pv2 exp = pv2_make(pf_sqrt(2), pf_sqrt(2));
    pv2 res = pt2_map(&t3, v);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_t2_compose)
{
    pt2 t1 = pt2_rotation(PM_PI / 8.0f);
    pt2 t2 = pt2_rotation(PM_PI / 8.0f);
    pt2 t3 = pt2_mult(&t1, &t2);

    pfloat angle = pt2_get_angle(&t3);
    REQUIRE(pf_equal(angle, PM_PI / 4.0f));

    t2 = pt2_scaling(pv2_make(2, 2));
    t3 = pt2_mult(&t3, &t2);

    pv2 scale = pt2_get_scale(&t3);

    REQUIRE(pf_equal(angle, PM_PI / 4.0f));

    pv2 exp = pv2_make(2, 2);

    REQUIRE(pv2_equal(scale, exp));

    return true;
}

TEST_CASE(test_t2_inv)
{
    pt2 t1 = pt2_rotation(PM_PI / 8.0f);
    pt2 t2 = pt2_rotation(PM_PI / 8.0f);
    pt2 t3 = pt2_mult(&t1, &t2);

    pt2 inv = pt2_inv(&t3);
    pt2 exp = pt2_identity();
    pt2 res = pt2_mult(&t3, &inv);

    REQUIRE(pt2_equal(&res, &exp));

    t1 = pt2_translation(pv2_make(1, 2));
    t2 = pt2_scaling(pv2_make(2, 2));
    t3 = pt2_mult(&t3, &t2);
    t3 = pt2_mult(&t3, &t1);
    inv = pt2_inv(&t3);
    res = pt2_mult(&t3, &inv);

    REQUIRE(pt2_equal(&res, &exp));

    return true;
}

TEST_CASE(test_t2_lerp)

{
    pt2 t1, t2, t3;

    t1 = pt2_translation(pv2_make(1, 1));
    t3 = pt2_scaling(pv2_make(1, 1));
    t1 = pt2_mult(&t1, &t3);
    t3 = pt2_rotation(PM_PI / 4.0f);
    t1 = pt2_mult(&t1, &t3);

    /*t1 = pt2_rotation(PM_PI / 4.0f);
    t3 = pt2_scaling(pv2_make(1, 1));
    t1 = pt2_mult(&t3, &t1);
    t3 = pt2_translation(pv2_make(1, 1));
    t1 = pt2_mult(&t3, &t1);*/

    t2 = pt2_rotation(PM_PI / 2.0f);
    t3 = pt2_scaling(pv2_make(2, 2));
    t2 = pt2_mult(&t3, &t2);
    t3 = pt2_translation(pv2_make(1, 1));
    t2 = pt2_mult(&t3, &t2);

    t3 = pt2_lerp(&t1, &t2, 0.5f);

    pv2 scale = pt2_get_scale(&t3);
    pv2 pos = pt2_get_pos(&t3);

    pfloat angle = pt2_get_angle(&t3);

    REQUIRE(pf_equal(angle, PM_PI * 3.0f / 8.0f));

    pv2 exp_scale = pv2_make(1.5, 1.5);
    pv2 exp_pos = pv2_make(1, 1);

    REQUIRE(pv2_equal(scale, exp_scale));
    REQUIRE(pv2_equal(pos, exp_pos));

    return true;
}

TEST_CASE(test_t2_lerp_identity)
{
    pt2 t1 = pt2_identity();
    pt2 t2 = pt2_identity();

    pt2 res = pt2_lerp(&t1, &t2, 0.3f);
    pt2 exp = pt2_identity();

    REQUIRE(pt2_equal(&res, &exp));

    return true;
}

TEST_SUITE(suite_t2)
{
    RUN_TEST_CASE(test_t2_equal);
    RUN_TEST_CASE(test_t2_identity);
    RUN_TEST_CASE(test_t2_get_pos);
    RUN_TEST_CASE(test_t2_set_pos);
    RUN_TEST_CASE(test_t2_get_angle);
    RUN_TEST_CASE(test_t2_get_scale);
    RUN_TEST_CASE(test_t2_set_angle);
    RUN_TEST_CASE(test_t2_map);
    RUN_TEST_CASE(test_t2_compose);
    RUN_TEST_CASE(test_t2_inv);
    RUN_TEST_CASE(test_t2_lerp);
    RUN_TEST_CASE(test_t2_lerp_identity);
}
