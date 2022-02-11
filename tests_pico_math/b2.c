#include "../pico_math.h"
#include "../pico_unit.h"

PU_TEST(test_b2_pos)
{
    pm_b2 b = pm_b2_make(1.0f, 2.0f, 3.0f, 4.0f);

    pm_v2 res = pm_b2_pos(&b);
    pm_v2 exp = pm_v2_make(1.0f, 2.0f);

    PU_ASSERT(pm_v2_equal(&res, &exp));

    return true;
}

PU_TEST(test_b2_size)
{
    pm_b2 b = pm_b2_make(1.0f, 2.0f, 3.0f, 4.0f);

    pm_v2 res = pm_b2_size(&b);
    pm_v2 exp = pm_v2_make(3.0f, 4.0f);

    PU_ASSERT(pm_v2_equal(&res, &exp));

    return true;
}

PU_TEST(test_b2_equal)
{
    { // Should be true

        pm_b2 b1 = pm_b2_make(1, 2, 3, 4);
        pm_b2 b2 = pm_b2_make(1, 2, 3, 4);
        PU_ASSERT(pm_b2_equal(&b1, &b2));
    }

    { // Should be false
        pm_b2 b1 = pm_b2_make(0, 2, 3, 4);
        pm_b2 b2 = pm_b2_make(1, 2, 3, 4);
        PU_ASSERT(!pm_b2_equal(&b1, &b2));
    }

    return true;
}

PU_TEST(test_b2_union)
{
    { // Case 1
        pm_b2 b1  = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2  = pm_b2_make(0.5f, 0.5f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0.0f, 0.0f, 1.5f, 1.5f);
        pm_b2 res = pm_b2_union(&b1, &b2);
        PU_ASSERT(pm_b2_equal(&res, &exp));
    }

    { // Case 2
        pm_b2 b1  = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2  = pm_b2_make(1.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0.0f, 0.0f, 2.0f, 1.0f);
        pm_b2 res = pm_b2_union(&b1, &b2);
        PU_ASSERT(pm_b2_equal(&res, &exp));
    }


    return true;
}

PU_TEST(test_b2_intersects)
{
    { // Case 1
        pm_b2 b1 = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2 = pm_b2_make(0.5f, 0.5f, 1.0f, 1.0f);
        PU_ASSERT(pm_b2_intersects(&b1, &b2));
    }

    { // Case 2
        pm_b2 b1 = pm_b2_make(1000.0f, 0.0f, 800.0f, 600.0f);
        pm_b2 b2 = pm_b2_make(813.0f, 100.0f, 192.0f, 192.0f);
        PU_ASSERT(pm_b2_intersects(&b1, &b2));
    }

    return true;
}

PU_TEST(test_b2_intersection)
{
    { // Case 1
        pm_b2 b1  = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2  = pm_b2_make(0.5f, 0.5f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0.5f, 0.5f, 0.5f, 0.5f);
        pm_b2 res = pm_b2_intersection(&b1, &b2);
        PU_ASSERT(pm_b2_equal(&res, &exp));
    }

    { // Case 2
        pm_b2 b1 =  pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 b2 =  pm_b2_make(2.0f, 0.0f, 1.0f, 1.0f);
        pm_b2 exp = pm_b2_make(0, 0, 0, 0);
        pm_b2 res = pm_b2_intersection(&b1, &b2);
        PU_ASSERT(pm_b2_equal(&res, &exp));
    }

    return true;
}

PU_TEST(test_b2_contains)
{
    pm_b2 b = pm_b2_make(1.0f, 1.0f, 2.0f, 2.0f);

    { // Should contain
        PU_ASSERT(pm_b2_contains(&b, pm_v2_make(1.5f, 1.5f)));
        PU_ASSERT(pm_b2_contains(&b, pm_v2_make(1.56f, 1.8f)));
    }

    { // Should not contain
        PU_ASSERT(!pm_b2_contains(&b, pm_v2_make(0.0f, 1.8f)));
    }

    return true;
}

PU_TEST(test_b2_min_aabb)
{
    pm_v2 verts[4];
    verts[0] = pm_v2_make(1.0f, 2.0f);
    verts[1] = pm_v2_make(1.0f, 2.0f + 4.0f);
    verts[2] = pm_v2_make(1.0f + 3.0f, 2.0f + 4.0f);
    verts[3] = pm_v2_make(1.0f + 3.0f, 4.0f);

    pm_b2 res = pm_b2_min_aabb(verts, 4);
    pm_b2 exp = pm_b2_make(1.0f, 2.0f, 3.0f, 4.0f);

    PU_ASSERT(pm_b2_equal(&res, &exp));

    return true;
}

PU_TEST(test_b2_transform)
{
    pm_b2 b = pm_b2_make(0.0f, 0.0f, 1.0f, 1.0f);

    pm_t2 t = pm_t2_identity();
    pm_t2_translate(&t, pm_v2_make(-0.5f, -0.5f));
    pm_t2_rotate(&t, -PM_PI / 4.0f);

    pm_b2 res = pm_b2_transform(&t, &b);
    pm_flt len = pm_sin(PM_PI / 4.0f);
    pm_b2 exp = pm_b2_make(-len, -len, 2.0f * len, 2.0f * len);

    PU_ASSERT(pm_b2_equal(&res, &exp));

    return true;
}

PU_SUITE(suite_b2)
{
    PU_RUN_TEST(test_b2_pos);
    PU_RUN_TEST(test_b2_size);
    PU_RUN_TEST(test_b2_equal);
    PU_RUN_TEST(test_b2_union);
    PU_RUN_TEST(test_b2_intersects);
    PU_RUN_TEST(test_b2_intersection);
    PU_RUN_TEST(test_b2_contains);
    PU_RUN_TEST(test_b2_min_aabb);
    PU_RUN_TEST(test_b2_transform);
}
