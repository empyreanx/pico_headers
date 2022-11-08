#include "../pico_math.h"
#include "../pico_unit.h"

TEST_CASE(test_b2_pos)
{
    pm_b2 b = pm_b2_make(1.0f, 2.0f, 3.0f, 4.0f);

    pm_v2 res = pm_b2_pos(&b);
    pm_v2 exp = pm_v2_make(1.0f, 2.0f);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_b2_size)
{
    pm_b2 b = pm_b2_make(1.0f, 2.0f, 3.0f, 4.0f);

    pm_v2 res = pm_b2_size(&b);
    pm_v2 exp = pm_v2_make(3.0f, 4.0f);

    REQUIRE(pm_v2_equal(res, exp));

    return true;
}

TEST_CASE(test_b2_equal)
{
    { // Should be true

        pm_b2 b1 = pm_b2_make(1, 2, 3, 4);
        pm_b2 b2 = pm_b2_make(1, 2, 3, 4);
        REQUIRE(pm_b2_equal(&b1, &b2));
    }

    { // Should be false
        pm_b2 b1 = pm_b2_make(0, 2, 3, 4);
        pm_b2 b2 = pm_b2_make(1, 2, 3, 4);
        REQUIRE(!pm_b2_equal(&b1, &b2));
    }

    return true;
}

TEST_CASE(test_b2_combine)
{
    { // Case 1
        pm_b2 b1  = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2  = pm_b2_make(0.5f, 0.5f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0.0f, 0.0f, 1.5f, 1.5f);
        pm_b2 res = pm_b2_combine(&b1, &b2);
        REQUIRE(pm_b2_equal(&res, &exp));
    }

    { // Case 2
        pm_b2 b1  = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2  = pm_b2_make(1.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0.0f, 0.0f, 2.0f, 1.0f);
        pm_b2 res = pm_b2_combine(&b1, &b2);
        REQUIRE(pm_b2_equal(&res, &exp));
    }


    return true;
}

TEST_CASE(test_b2_overlaps)
{
    { // Case 1
        pm_b2 b1 = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2 = pm_b2_make(0.5f, 0.5f, 1.0f, 1.0f);
        REQUIRE(pm_b2_overlaps(&b1, &b2));
    }

    { // Case 2
        pm_b2 b1 = pm_b2_make(1000.0f, 0.0f, 800.0f, 600.0f);
        pm_b2 b2 = pm_b2_make(813.0f, 100.0f, 192.0f, 192.0f);
        REQUIRE(pm_b2_overlaps(&b1, &b2));
    }

    return true;
}

TEST_CASE(test_b2_overlap)
{
    { // Case 1
        pm_b2 b1  = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2  = pm_b2_make(0.5f, 0.5f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0.5f, 0.5f, 0.5f, 0.5f);
        pm_b2 res = pm_b2_overlap(&b1, &b2);
        REQUIRE(pm_b2_equal(&res, &exp));
    }

    { // Case 2
        pm_b2 b1 =  pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2 =  pm_b2_make(2.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0, 0, 0, 0);
        pm_b2 res = pm_b2_overlap(&b1, &b2);
        REQUIRE(pm_b2_equal(&res, &exp));
    }

    return true;
}

TEST_CASE(test_b2_contains)
{
    { // Contains
        pm_b2 b1 = pm_b2_make(2.0f, 2.0f, 4.0f, 4.0f);
        pm_b2 b2 = pm_b2_make(3.0f, 3.0f, 2.0f, 2.0f);
        REQUIRE(pm_b2_contains(&b1, &b2));
    }

    { // Overlaps
        pm_b2 b1 = pm_b2_make(2.0f, 2.0f, 4.0f, 4.0f);
        pm_b2 b2 = pm_b2_make(1.0f, 1.0f, 2.0f, 2.0f);
        REQUIRE(!pm_b2_contains(&b1, &b2));
    }

    { // Does not overlap
        pm_b2 b1 = pm_b2_make(2.0f, 2.0f, 4.0f, 4.0f);
        pm_b2 b2 = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        REQUIRE(!pm_b2_contains(&b1, &b2));
    }

    return true;
}

TEST_CASE(test_b2_contains_point)
{
    pm_b2 b = pm_b2_make(1.0f, 1.0f, 2.0f, 2.0f);

    { // Should contain
        REQUIRE(pm_b2_contains_point(&b, pm_v2_make(1.5f, 1.5f)));
        REQUIRE(pm_b2_contains_point(&b, pm_v2_make(1.56f, 1.8f)));
    }

    { // Should not contain
        REQUIRE(!pm_b2_contains_point(&b, pm_v2_make(0.0f, 1.8f)));
    }

    return true;
}

TEST_CASE(test_b2_enclosing)
{
    pm_v2 verts[4];
    verts[0] = pm_v2_make(1.0f, 2.0f);
    verts[1] = pm_v2_make(1.0f, 2.0f + 4.0f);
    verts[2] = pm_v2_make(1.0f + 3.0f, 2.0f + 4.0f);
    verts[3] = pm_v2_make(1.0f + 3.0f, 4.0f);

    pm_b2 res = pm_b2_enclosing(verts, 4);
    pm_b2 exp = pm_b2_make(1.0f, 2.0f, 3.0f, 4.0f);

    REQUIRE(pm_b2_equal(&res, &exp));

    return true;
}

TEST_CASE(test_b2_transform)
{
    pm_b2 b = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);

    pm_t2 t = pm_t2_identity();
    pm_t2_translate(&t, pm_v2_make(-0.5f, -0.5f));
    pm_t2_rotate(&t, -PM_PI / 4.0f);

    pm_b2 res = pm_b2_transform(&t, &b);
    pm_float len = pm_sin(PM_PI / 4.0f);
    pm_b2 exp = pm_b2_make(-len, -len, 2.0f * len, 2.0f * len);

    REQUIRE(pm_b2_equal(&res, &exp));

    return true;
}

TEST_SUITE(suite_b2)
{
    RUN_TEST_CASE(test_b2_pos);
    RUN_TEST_CASE(test_b2_size);
    RUN_TEST_CASE(test_b2_equal);
    RUN_TEST_CASE(test_b2_combine);
    RUN_TEST_CASE(test_b2_overlaps);
    RUN_TEST_CASE(test_b2_overlap);
    RUN_TEST_CASE(test_b2_contains);
    RUN_TEST_CASE(test_b2_contains_point);
    RUN_TEST_CASE(test_b2_enclosing);
    RUN_TEST_CASE(test_b2_transform);
}
