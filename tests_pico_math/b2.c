#include "../pico_math.h"
#include "../pico_unit.h"

TEST_CASE(test_b2_get_pos)
{
    pb2 b = pb2_make(1.0f, 2.0f, 3.0f, 4.0f);

    pv2 res = pb2_get_pos(&b);
    pv2 exp = pv2_make(1.0f, 2.0f);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_b2_get_size)
{
    pb2 b = pb2_make(1.0f, 2.0f, 3.0f, 4.0f);

    pv2 res = pb2_get_size(&b);
    pv2 exp = pv2_make(3.0f, 4.0f);

    REQUIRE(pv2_equal(res, exp));

    return true;
}

TEST_CASE(test_b2_equal)
{
    { // Should be true

        pb2 b1 = pb2_make(1, 2, 3, 4);
        pb2 b2 = pb2_make(1, 2, 3, 4);
        REQUIRE(pb2_equal(&b1, &b2));
    }

    { // Should be false
        pb2 b1 = pb2_make(0, 2, 3, 4);
        pb2 b2 = pb2_make(1, 2, 3, 4);
        REQUIRE(!pb2_equal(&b1, &b2));
    }

    return true;
}

TEST_CASE(test_b2_combine)
{
    { // Case 1
        pb2 b1  = pb2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pb2 b2  = pb2_make(0.5f, 0.5f, 1.0f, 1.0f);
        pb2 exp = pb2_make(0.0f, 0.0f, 1.5f, 1.5f);
        pb2 res = pb2_combine(&b1, &b2);
        REQUIRE(pb2_equal(&res, &exp));
    }

    { // Case 2
        pb2 b1  = pb2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pb2 b2  = pb2_make(1.0f, 0.0f, 1.0f, 1.0f);
        pb2 exp = pb2_make(0.0f, 0.0f, 2.0f, 1.0f);
        pb2 res = pb2_combine(&b1, &b2);
        REQUIRE(pb2_equal(&res, &exp));
    }


    return true;
}

TEST_CASE(test_b2_overlaps)
{
    { // Case 1
        pb2 b1 = pb2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pb2 b2 = pb2_make(0.5f, 0.5f, 1.0f, 1.0f);
        REQUIRE(pb2_overlaps(&b1, &b2));
    }

    { // Case 2
        pb2 b1 = pb2_make(1000.0f, 0.0f, 800.0f, 600.0f);
        pb2 b2 = pb2_make(813.0f, 100.0f, 192.0f, 192.0f);
        REQUIRE(pb2_overlaps(&b1, &b2));
    }

    { // Case 3

        pb2 b1 = pb2_make(0, 0, 32, 64);
        pb2 b2 = pb2_make(32, 5, 10, 10);
        REQUIRE(pb2_overlaps(&b1, &b2));

        b2 = pb2_make(33, 0, 10, 10);
        REQUIRE(!pb2_overlaps(&b1, &b2));
    }

    return true;
}

TEST_CASE(test_b2_overlap)
{
    { // Case 1
        pb2 b1  = pb2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pb2 b2  = pb2_make(0.5f, 0.5f, 1.0f, 1.0f);
        pb2 exp = pb2_make(0.5f, 0.5f, 0.5f, 0.5f);
        pb2 res = pb2_overlap(&b1, &b2);
        REQUIRE(pb2_equal(&res, &exp));
    }

    { // Case 2
        pb2 b1 =  pb2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pb2 b2 =  pb2_make(2.0f, 0.0f, 1.0f, 1.0f);
        pb2 exp = pb2_make(0, 0, 0, 0);
        pb2 res = pb2_overlap(&b1, &b2);
        REQUIRE(pb2_equal(&res, &exp));
    }

    return true;
}

TEST_CASE(test_b2_contains)
{
    { // Contains
        pb2 b1 = pb2_make(2.0f, 2.0f, 4.0f, 4.0f);
        pb2 b2 = pb2_make(3.0f, 3.0f, 2.0f, 2.0f);
        REQUIRE(pb2_contains(&b1, &b2));
    }

    { // Overlaps
        pb2 b1 = pb2_make(2.0f, 2.0f, 4.0f, 4.0f);
        pb2 b2 = pb2_make(1.0f, 1.0f, 2.0f, 2.0f);
        REQUIRE(!pb2_contains(&b1, &b2));
    }

    { // Does not overlap
        pb2 b1 = pb2_make(2.0f, 2.0f, 4.0f, 4.0f);
        pb2 b2 = pb2_make(0.0f, 0.0f, 1.0f, 1.0f);
        REQUIRE(!pb2_contains(&b1, &b2));
    }

    return true;
}

TEST_CASE(test_b2_contains_point)
{
    pb2 b = pb2_make(1.0f, 1.0f, 2.0f, 2.0f);

    { // Should contain
        REQUIRE(pb2_contains_point(&b, pv2_make(1.5f, 1.5f)));
        REQUIRE(pb2_contains_point(&b, pv2_make(1.56f, 1.8f)));
    }

    { // Should not contain
        REQUIRE(!pb2_contains_point(&b, pv2_make(0.0f, 1.8f)));
    }

    return true;
}

TEST_CASE(test_b2_enclosing)
{
    pv2 verts[4];
    verts[0] = pv2_make(1.0f, 2.0f);
    verts[1] = pv2_make(1.0f, 2.0f + 4.0f);
    verts[2] = pv2_make(1.0f + 3.0f, 2.0f + 4.0f);
    verts[3] = pv2_make(1.0f + 3.0f, 4.0f);

    pb2 res = pb2_enclosing(verts, 4);
    pb2 exp = pb2_make(1.0f, 2.0f, 3.0f, 4.0f);

    REQUIRE(pb2_equal(&res, &exp));

    return true;
}

TEST_CASE(test_b2_transform)
{
    pb2 b = pb2_make(0.0f, 0.0f, 1.0f, 1.0f);

    pt2 t = pt2_identity();
    pt2_translate(&t, pv2_make(-0.5f, -0.5f));
    pt2_rotate(&t, -PM_PI / 4.0f);

    pb2 res = pb2_transform(&t, &b);
    pfloat len = pf_sin(PM_PI / 4.0f);
    pb2 exp = pb2_make(-len, -len, 2.0f * len, 2.0f * len);

    REQUIRE(pb2_equal(&res, &exp));

    return true;
}

TEST_SUITE(suite_b2)
{
    RUN_TEST_CASE(test_b2_get_pos);
    RUN_TEST_CASE(test_b2_get_size);
    RUN_TEST_CASE(test_b2_equal);
    RUN_TEST_CASE(test_b2_combine);
    RUN_TEST_CASE(test_b2_overlaps);
    RUN_TEST_CASE(test_b2_overlap);
    RUN_TEST_CASE(test_b2_contains);
    RUN_TEST_CASE(test_b2_contains_point);
    RUN_TEST_CASE(test_b2_enclosing);
    RUN_TEST_CASE(test_b2_transform);
}
