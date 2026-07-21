#include "../pico_hit.h"
#include "../pico_unit.h"

TEST_CASE(test_transform_poly)
{
    pb2 b = pb2_make(-0.5f, -0.5f, 1, 1);
    ph_poly_t p = ph_aabb_to_poly(&b);

    pt2 t = pt2_identity();
    pt2_rotate(&t, PM_PI / 4.0f);

    ph_poly_t res = ph_transform_poly(&t, &p);

    const pfloat half_diag = 0.5f * pf_sqrt(2.0f);

    REQUIRE(pv2_equal(res.vertices[0], pv2_make(0.0f, -half_diag)));
    REQUIRE(pv2_equal(res.vertices[1], pv2_make(-half_diag, 0.0f)));
    REQUIRE(pv2_equal(res.vertices[2], pv2_make(0.0f, half_diag)));
    REQUIRE(pv2_equal(res.vertices[3], pv2_make(half_diag, 0.0f)));

    return true;
}

TEST_CASE(test_transform_circle)
{
    ph_circle_t c = ph_make_circle(pv2_make(1, 0), 1);

    pt2 t = pt2_identity();
    pt2_rotate(&t, PM_PI / 2.0f);
    pt2_translate(&t, pv2_make(0, 1));

    ph_circle_t res = ph_transform_circle(&t, &c);

    REQUIRE(pv2_equal(res.center, pv2_make(0, 2)));

    return true;
}

TEST_CASE(test_transform_ray_identity)
{
    ph_ray_t r = ph_make_ray(pv2_make(1, 2), pv2_make(1, 0), 5.f);

    pt2 t = pt2_identity();

    ph_ray_t res = ph_transform_ray(&t, &r);

    REQUIRE(pv2_equal(res.origin, pv2_make(1, 2)));
    REQUIRE(pv2_equal(res.dir, pv2_make(1, 0)));
    REQUIRE(pf_equal(res.len, 5.f));

    return true;
}

TEST_CASE(test_transform_ray_translate)
{
    // Translation shifts the origin but leaves the direction unchanged
    ph_ray_t r = ph_make_ray(pv2_make(0, 0), pv2_make(1, 0), 10.f);

    pt2 t = pt2_identity();
    pt2_translate(&t, pv2_make(3, 4));

    ph_ray_t res = ph_transform_ray(&t, &r);

    REQUIRE(pv2_equal(res.origin, pv2_make(3, 4)));
    REQUIRE(pv2_equal(res.dir, pv2_make(1, 0)));
    REQUIRE(pf_equal(res.len, 10.f));

    return true;
}

TEST_CASE(test_transform_ray_rotate)
{
    // Rotating by 90 degrees rotates both origin and direction
    ph_ray_t r = ph_make_ray(pv2_make(1, 0), pv2_make(1, 0), 7.f);

    pt2 t = pt2_identity();
    pt2_rotate(&t, PM_PI / 2.0f);

    ph_ray_t res = ph_transform_ray(&t, &r);

    REQUIRE(pv2_equal(res.origin, pv2_make(0, 1)));
    REQUIRE(pv2_equal(res.dir, pv2_make(0, 1)));
    REQUIRE(pf_equal(res.len, 7.f));

    return true;
}

TEST_CASE(test_transform_ray_rotate_translate)
{
    // Combined rotate-then-translate transform
    ph_ray_t r = ph_make_ray(pv2_make(1, 0), pv2_make(1, 0), 3.f);

    pt2 t = pt2_identity();
    pt2_rotate(&t, PM_PI / 2.0f);
    pt2_translate(&t, pv2_make(0, 1));

    ph_ray_t res = ph_transform_ray(&t, &r);

    REQUIRE(pv2_equal(res.origin, pv2_make(0, 2)));
    REQUIRE(pv2_equal(res.dir, pv2_make(0, 1)));
    REQUIRE(pf_equal(res.len, 3.f));

    return true;
}

TEST_CASE(test_circle_to_aabb)
{
    ph_circle_t c = ph_make_circle(pv2_make(0, 0), 1);

    pb2 exp = pb2_make(-0.5f, -0.5f, 1, 1);
    pb2 res = ph_circle_to_aabb(&c);

    REQUIRE(pb2_equal(&exp, &res));

    return true;
}

TEST_CASE(test_poly_to_aabb)
{
    const pv2 vertices[] =
    {
        {  2, 5 },
        { -4, 3 },
        {  5, 1 }
    };

    ph_poly_t p = ph_make_poly(vertices, 3, false);

    pb2 exp = pb2_make_minmax(pv2_make(-4, 1), pv2_make(5, 5));
    pb2 res = ph_poly_to_aabb(&p);

    REQUIRE(pb2_equal(&exp, &res));

    return true;
}

TEST_SUITE(suite_transforms)
{
    RUN_TEST_CASE(test_transform_poly);
    RUN_TEST_CASE(test_transform_circle);
    RUN_TEST_CASE(test_transform_ray_identity);
    RUN_TEST_CASE(test_transform_ray_translate);
    RUN_TEST_CASE(test_transform_ray_rotate);
    RUN_TEST_CASE(test_transform_ray_rotate_translate);
    RUN_TEST_CASE(test_circle_to_aabb);
    RUN_TEST_CASE(test_poly_to_aabb);
}
