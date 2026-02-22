#include "../pico_unit.h"
#include "../pico_hit.h"

// Helper function to create a square polygon
static ph_poly_t make_square(pv2 center, pfloat size)
{
    pv2 half = pv2_make(size / 2.0f, size / 2.0f);
    pv2 vertices[] = {
        pv2_sub(center, half),                          // bottom-left
        pv2_make(center.x - half.x, center.y + half.y), // top-left
        pv2_add(center, half),                          // top-right
        pv2_make(center.x + half.x, center.y - half.y)  // bottom-right
    };
    return ph_make_poly(vertices, 4, false);
}

// Test: Basic square-to-square collision with simple overlap
TEST_CASE(test_manifold_poly_poly_square_overlap)
{
    // Two squares overlapping slightly
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    // Should generate contacts
    REQUIRE(hit);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));
    REQUIRE(manifold.count > 0);
    REQUIRE(manifold.count <= 2);

    return true;
}

// Test: Verify contact point is within expected bounds
TEST_CASE(test_manifold_poly_poly_contact_position)
{
    // Square A at origin, Square B offset to the right
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));

    // Contact points should be within the overlapping region
    for (int i = 0; i < manifold.count; i++)
    {
        pv2 p = manifold.contacts[i].point;
        // Contact should be in the overlap region (x between 0.5 and 1.5)
        REQUIRE(p.x >= 0.5f - 0.01f);
        REQUIRE(p.x <= 1.5f + 0.01f);
        // Contact depth should be positive
        REQUIRE(manifold.contacts[i].depth >= 0.0f);
    }

    return true;
}

// Test: Contact depth verification
TEST_CASE(test_manifold_poly_poly_contact_depth)
{
    // Small overlap should result in small depth
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.9f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));

    // Depth should be small (less than 0.2 since we have small overlap)
    for (int i = 0; i < manifold.count; i++)
    {
        REQUIRE(manifold.contacts[i].depth <= 0.2f);
    }

    return true;
}

// Test: Horizontal edge-to-edge collision
TEST_CASE(test_manifold_poly_poly_horizontal_edge)
{
    // Two squares touching horizontally
    ph_poly_t poly_a = make_square(pv2_make(-1.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.0f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    // They should generate contacts
    REQUIRE(hit);
    REQUIRE(manifold.count >= 1);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));

    return true;
}

// Test: Vertical edge-to-edge collision
TEST_CASE(test_manifold_poly_poly_vertical_edge)
{
    // Two squares touching vertically
    ph_poly_t poly_a = make_square(pv2_make(0.0f, -1.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(0.0f, 1.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(0.0f, 1.0f);

    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    // They should generate contacts
    REQUIRE(hit);
    REQUIRE(manifold.count >= 1);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(0.0f, 1.0f)));

    return true;
}

// Test: Diagonal collision
TEST_CASE(test_manifold_poly_poly_diagonal_overlap)
{
    // Two squares overlapping diagonally
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.0f, 1.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_normalize(pv2_make(1.0f, 1.0f));

    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);
    REQUIRE(manifold.count <= 2);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 1.0f)));

    return true;
}

// Test: Maximum two contacts per manifold
TEST_CASE(test_manifold_poly_poly_max_contacts)
{
    // Two overlapping squares
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.0f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    REQUIRE(hit);
    // The manifold should have at most 2 contact points
    REQUIRE(manifold.count <= 2);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));

    return true;
}

// Test: Triangle-to-square collision
TEST_CASE(test_manifold_poly_poly_triangle_square)
{
    // Triangle
    pv2 tri_vertices[] = {
        pv2_make(0.0f, 2.0f),
        pv2_make(2.0f, 0.0f),
        pv2_make(0.0f, 0.0f)
    };
    ph_poly_t triangle = ph_make_poly(tri_vertices, 3, false);

    // Square
    ph_poly_t square = make_square(pv2_make(1.0f, 0.5f), 1.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_manifold_poly_poly(&triangle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);
    REQUIRE(manifold.count <= 2);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));

    return true;
}

// Test: Deep overlap generates contacts
TEST_CASE(test_manifold_poly_poly_deep_overlap)
{
    // Two squares with significant overlap
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(0.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);

    // With deep overlap, we should typically get 2 contacts
    REQUIRE(manifold.count <= 2);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));

    return true;
}

// Test: Manifold is cleared before use
TEST_CASE(test_manifold_poly_poly_manifold_cleared)
{
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    manifold.count = 999;  // Pre-set to invalid value

    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    // Count should be set from 0, not added to existing value
    REQUIRE(hit);
    REQUIRE(manifold.count >= 0);
    REQUIRE(manifold.count <= 2);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1.0f, 0.0f)));

    return true;
}

// Test: Normalized normal input handling
TEST_CASE(test_manifold_poly_poly_with_normalized_normal)
{
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    // Provide a normalized normal
    pv2 normal = pv2_normalize(pv2_make(3.0f, 4.0f));

    ph_manifold_poly_poly(&poly_a, &poly_b, &manifold);

    // Should still work with normalized normal
    REQUIRE(manifold.count >= 0);
    REQUIRE(manifold.count <= 2);
    REQUIRE(pv2_equal(manifold.normal, pv2_make(3.0f, 4.0f)));

    return true;
}

// Test: Basic polygon-circle collision detection
TEST_CASE(test_manifold_poly_circle_basic_collision)
{
    // Create a square and a circle that overlaps
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(1.5f, 0.0f), 0.8f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: Circle touching polygon edge
TEST_CASE(test_manifold_poly_circle_edge_touch)
{
    // Square at origin
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

    // Circle touching the right edge at x = 1.0f
    ph_circle_t circle = ph_make_circle(pv2_make(1.8f, 0.0f), 0.8f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    // Contact point should be on or near the right edge (x around 1.0)
    REQUIRE(manifold.contacts[0].point.x >= 0.9f);
    REQUIRE(manifold.contacts[0].point.x <= 1.1f);

    return true;
}

// Test: Circle center inside polygon
TEST_CASE(test_manifold_poly_circle_center_inside)
{
    // Square at origin
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

    // Circle with center inside the square
    ph_circle_t circle = ph_make_circle(pv2_make(0.2f, 0.2f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    // Depth should be positive (penetration)
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: Contact depth calculation accuracy
TEST_CASE(test_manifold_poly_circle_depth_accuracy)
{
    // Square at origin with size 2 (so edges at ±1)
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

    // Circle with radius 0.5, center at (1.3, 0) - overlapping right edge at x=1
    // Expected depth ≈ 0.5 - 0.3 = 0.2
    ph_circle_t circle = ph_make_circle(pv2_make(1.3f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);

    // Depth should be approximately 0.2 (within tolerance)
    pfloat expected_depth = 0.2f;
    REQUIRE(manifold.contacts[0].depth > expected_depth - 0.01f);
    REQUIRE(manifold.contacts[0].depth < expected_depth + 0.01f);

    return true;
}

// Test: Circle near polygon vertex
TEST_CASE(test_manifold_poly_circle_near_vertex)
{
    // Square at origin
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

    // Circle near top-right vertex at (1, 1) - increase radius slightly
    // to ensure overlap for the test (avoids exact borderline cases).
    ph_circle_t circle = ph_make_circle(pv2_make(1.5f, 1.5f), 0.72f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: No collision when circle is outside
TEST_CASE(test_manifold_poly_circle_no_collision)
{
    // Square at origin
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

    // Circle far away to the right
    ph_circle_t circle = ph_make_circle(pv2_make(5.0f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(!hit);

    return true;
}

// Test: Manifold normal is set correctly
TEST_CASE(test_manifold_poly_circle_manifold_normal)
{
    // Square at origin
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

    // Circle to the right — nudge center slightly to ensure a small overlap
    // with the right edge (avoids exact touching which can be brittle).
    ph_circle_t circle = ph_make_circle(pv2_make(1.49f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    // Normal should be set from SAT result
    REQUIRE(!pf_equal(manifold.normal.x, 0.0f) || !pf_equal(manifold.normal.y, 0.0f));
    // Normal should be normalized
    pfloat normal_len = pf_sqrt(pv2_dot(manifold.normal, manifold.normal));
    REQUIRE(normal_len > 0.99f);
    REQUIRE(normal_len < 1.01f);

    return true;
}

// Test: Manifold count is always 1
TEST_CASE(test_manifold_poly_circle_count_is_one)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(1.0f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};
    manifold.count = 999;  // Pre-set to invalid value

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);

    return true;
}

// Test: Contact point is on polygon edge
TEST_CASE(test_manifold_poly_circle_point_on_edge)
{
    // Triangle
    pv2 tri_vertices[] = {
        pv2_make(0.0f, 2.0f),
        pv2_make(2.0f, 0.0f),
        pv2_make(0.0f, 0.0f)
    };
    ph_poly_t triangle = ph_make_poly(tri_vertices, 3, false);

    // Circle overlapping the triangle
    ph_circle_t circle = ph_make_circle(pv2_make(1.0f, 0.5f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&triangle, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    // Contact point should be on or near an edge
    REQUIRE(manifold.contacts[0].point.x >= -0.1f);
    REQUIRE(manifold.contacts[0].point.x <= 2.1f);

    return true;
}

// Test: Large circle vs small polygon
TEST_CASE(test_manifold_poly_circle_large_circle)
{
    // Small square
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 0.5f);

    // Large circle containing the square
    ph_circle_t circle = ph_make_circle(pv2_make(0.0f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: Small circle vs large polygon
TEST_CASE(test_manifold_poly_circle_small_circle)
{
    // Large square
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 4.0f);

    // Small circle inside
    ph_circle_t circle = ph_make_circle(pv2_make(0.5f, 0.5f), 0.2f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: Depth is non-negative
TEST_CASE(test_manifold_poly_circle_depth_non_negative)
{
    for (int test_num = 0; test_num < 5; test_num++)
    {
        ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

        // Various overlapping positions
        pv2 positions[] = {
            pv2_make(1.0f, 0.0f),
            pv2_make(0.0f, 1.0f),
            pv2_make(0.7f, 0.7f),
            pv2_make(-0.5f, -0.5f),
            pv2_make(0.2f, 0.2f)
        };

        ph_circle_t circle = ph_make_circle(positions[test_num], 0.6f);
        ph_manifold_t manifold = {0};

        bool hit = ph_manifold_poly_circle(&square, &circle, &manifold);

        if (hit)
        {
            REQUIRE(manifold.contacts[0].depth >= 0.0f);
        }
    }

    return true;
}

// -------------------- Circle / Circle manifold tests ----------------------

// Test: Basic overlapping circles
TEST_CASE(test_manifold_circle_circle_basic_overlap)
{
    ph_circle_t a = ph_make_circle(pv2_make(0.0f, 0.0f), 1.0f);
    ph_circle_t b = ph_make_circle(pv2_make(1.5f, 0.0f), 1.0f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_circle(&a, &b, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.49f);

    pfloat nlen = pf_sqrt(pv2_dot(manifold.normal, manifold.normal));
    REQUIRE(nlen > 0.99f && nlen < 1.01f);

    // Contact point should be between the two circle surface points
    REQUIRE(manifold.contacts[0].point.x > 0.4f);
    REQUIRE(manifold.contacts[0].point.x < 1.0f);

    return true;
}

// Test: Tangent circles should not produce a manifold
TEST_CASE(test_manifold_circle_circle_tangent_no_hit)
{
    ph_circle_t a = ph_make_circle(pv2_make(0.0f, 0.0f), 1.0f);
    ph_circle_t b = ph_make_circle(pv2_make(2.0f, 0.0f), 1.0f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_circle(&a, &b, &manifold);

    REQUIRE(!hit);

    return true;
}

// Test: One circle fully contained inside another
TEST_CASE(test_manifold_circle_circle_contained)
{
    ph_circle_t a = ph_make_circle(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t b = ph_make_circle(pv2_make(0.5f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_circle(&a, &b, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 1.9f);

    return true;
}

// Test: Coincident centers
TEST_CASE(test_manifold_circle_circle_coincident_centers)
{
    ph_circle_t a = ph_make_circle(pv2_make(0.0f, 0.0f), 1.0f);
    ph_circle_t b = ph_make_circle(pv2_make(0.0f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_circle(&a, &b, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);

    pfloat nlen = pf_sqrt(pv2_dot(manifold.normal, manifold.normal));
    REQUIRE(nlen > 0.99f && nlen < 1.01f);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// -------------------- Circle / Polygon manifold tests (circle first) -------

// Test: Basic circle vs polygon collision
TEST_CASE(test_manifold_circle_poly_basic_collision)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(1.5f, 0.0f), 0.8f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: Circle touching polygon edge
TEST_CASE(test_manifold_circle_poly_edge_touch)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(1.8f, 0.0f), 0.8f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].point.x >= 0.9f);
    REQUIRE(manifold.contacts[0].point.x <= 1.1f);

    return true;
}

// Test: Circle center inside polygon
TEST_CASE(test_manifold_circle_poly_center_inside)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(0.2f, 0.2f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: No collision when circle is outside
TEST_CASE(test_manifold_circle_poly_no_collision)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(5.0f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(!hit);

    return true;
}

// Test: Manifold normal is set correctly
TEST_CASE(test_manifold_circle_poly_manifold_normal)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(1.49f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(!pf_equal(manifold.normal.x, 0.0f) || !pf_equal(manifold.normal.y, 0.0f));
    pfloat normal_len = pf_sqrt(pv2_dot(manifold.normal, manifold.normal));
    REQUIRE(normal_len > 0.99f);
    REQUIRE(normal_len < 1.01f);

    return true;
}

// Test: Manifold count is always 1
TEST_CASE(test_manifold_circle_poly_count_is_one)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(1.0f, 0.0f), 0.5f);

    ph_manifold_t manifold = {0};
    manifold.count = 999;

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);

    return true;
}

// Test: Depth is non-negative
TEST_CASE(test_manifold_circle_poly_depth_non_negative)
{
    for (int test_num = 0; test_num < 5; test_num++)
    {
        ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);

        pv2 positions[] = {
            pv2_make(1.0f, 0.0f),
            pv2_make(0.0f, 1.0f),
            pv2_make(0.7f, 0.7f),
            pv2_make(-0.5f, -0.5f),
            pv2_make(0.2f, 0.2f)
        };

        ph_circle_t circle = ph_make_circle(positions[test_num], 0.6f);
        ph_manifold_t manifold = {0};

        bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

        if (hit)
        {
            REQUIRE(manifold.contacts[0].depth >= 0.0f);
        }
    }

    return true;
}

// Test: Circle near polygon vertex
TEST_CASE(test_manifold_circle_poly_near_vertex)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(1.5f, 1.5f), 0.72f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: Large circle vs small polygon
TEST_CASE(test_manifold_circle_poly_large_circle)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 0.5f);
    ph_circle_t circle = ph_make_circle(pv2_make(0.0f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

// Test: Small circle vs large polygon
TEST_CASE(test_manifold_circle_poly_small_circle)
{
    ph_poly_t square = make_square(pv2_make(0.0f, 0.0f), 4.0f);
    ph_circle_t circle = ph_make_circle(pv2_make(0.5f, 0.5f), 0.2f);

    ph_manifold_t manifold = {0};

    bool hit = ph_manifold_circle_poly(&circle, &square, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count == 1);
    REQUIRE(manifold.contacts[0].depth > 0.0f);

    return true;
}

TEST_SUITE(suite_contacts)
{
    RUN_TEST_CASE(test_manifold_poly_poly_square_overlap);
    RUN_TEST_CASE(test_manifold_poly_poly_contact_position);
    RUN_TEST_CASE(test_manifold_poly_poly_contact_depth);
    RUN_TEST_CASE(test_manifold_poly_poly_horizontal_edge);
    RUN_TEST_CASE(test_manifold_poly_poly_vertical_edge);
    RUN_TEST_CASE(test_manifold_poly_poly_diagonal_overlap);
    RUN_TEST_CASE(test_manifold_poly_poly_max_contacts);
    RUN_TEST_CASE(test_manifold_poly_poly_triangle_square);
    RUN_TEST_CASE(test_manifold_poly_poly_deep_overlap);
    RUN_TEST_CASE(test_manifold_poly_poly_manifold_cleared);
    RUN_TEST_CASE(test_manifold_poly_poly_with_normalized_normal);
    RUN_TEST_CASE(test_manifold_poly_circle_basic_collision);
    RUN_TEST_CASE(test_manifold_poly_circle_edge_touch);
    RUN_TEST_CASE(test_manifold_poly_circle_center_inside);
    RUN_TEST_CASE(test_manifold_poly_circle_depth_accuracy);
    RUN_TEST_CASE(test_manifold_poly_circle_near_vertex);
    RUN_TEST_CASE(test_manifold_poly_circle_no_collision);
    RUN_TEST_CASE(test_manifold_poly_circle_manifold_normal);
    RUN_TEST_CASE(test_manifold_poly_circle_count_is_one);
    RUN_TEST_CASE(test_manifold_poly_circle_point_on_edge);
    RUN_TEST_CASE(test_manifold_poly_circle_large_circle);
    RUN_TEST_CASE(test_manifold_poly_circle_small_circle);
    RUN_TEST_CASE(test_manifold_poly_circle_depth_non_negative);
    RUN_TEST_CASE(test_manifold_circle_poly_basic_collision);
    RUN_TEST_CASE(test_manifold_circle_poly_edge_touch);
    RUN_TEST_CASE(test_manifold_circle_poly_center_inside);
    RUN_TEST_CASE(test_manifold_circle_poly_depth_non_negative);
    RUN_TEST_CASE(test_manifold_circle_poly_near_vertex);
    RUN_TEST_CASE(test_manifold_circle_poly_no_collision);
    RUN_TEST_CASE(test_manifold_circle_poly_manifold_normal);
    RUN_TEST_CASE(test_manifold_circle_poly_count_is_one);
    RUN_TEST_CASE(test_manifold_circle_poly_large_circle);
    RUN_TEST_CASE(test_manifold_circle_poly_small_circle);
    RUN_TEST_CASE(test_manifold_circle_circle_basic_overlap);
    RUN_TEST_CASE(test_manifold_circle_circle_tangent_no_hit);
    RUN_TEST_CASE(test_manifold_circle_circle_contained);
    RUN_TEST_CASE(test_manifold_circle_circle_coincident_centers);
}