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
TEST_CASE(test_contacts_poly_poly_square_overlap)
{
    // Two squares overlapping slightly
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);  // Normal pointing to the right

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    // Should generate contacts
    REQUIRE(hit);
    REQUIRE(manifold.count > 0);
    REQUIRE(manifold.count <= 2);

    return true;
}

// Test: Verify contact point is within expected bounds
TEST_CASE(test_contacts_poly_poly_contact_position)
{
    // Square A at origin, Square B offset to the right
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);

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
TEST_CASE(test_contacts_poly_poly_contact_depth)
{
    // Small overlap should result in small depth
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.9f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);

    // Depth should be small (less than 0.2 since we have small overlap)
    for (int i = 0; i < manifold.count; i++)
    {
        REQUIRE(manifold.contacts[i].depth <= 0.2f);
    }

    return true;
}

// Test: Horizontal edge-to-edge collision
TEST_CASE(test_contacts_poly_poly_horizontal_edge)
{
    // Two squares touching horizontally
    ph_poly_t poly_a = make_square(pv2_make(-1.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.0f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    // They should generate contacts
    REQUIRE(hit);
    REQUIRE(manifold.count >= 1);

    return true;
}

// Test: Vertical edge-to-edge collision
TEST_CASE(test_contacts_poly_poly_vertical_edge)
{
    // Two squares touching vertically
    ph_poly_t poly_a = make_square(pv2_make(0.0f, -1.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(0.0f, 1.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(0.0f, 1.0f);

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    // They should generate contacts
    REQUIRE(hit);
    REQUIRE(manifold.count >= 1);

    return true;
}

// Test: Diagonal collision
TEST_CASE(test_contacts_poly_poly_diagonal_overlap)
{
    // Two squares overlapping diagonally
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.0f, 1.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_normalize(pv2_make(1.0f, 1.0f));

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);
    REQUIRE(manifold.count <= 2);

    return true;
}

// Test: Maximum two contacts per manifold
TEST_CASE(test_contacts_poly_poly_max_contacts)
{
    // Two overlapping squares
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.0f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    REQUIRE(hit);
    // The manifold should have at most 2 contact points
    REQUIRE(manifold.count <= 2);

    return true;
}

// Test: Triangle-to-square collision
TEST_CASE(test_contacts_poly_poly_triangle_square)
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

    bool hit = ph_contacts_poly_poly(&triangle, &square, normal, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);
    REQUIRE(manifold.count <= 2);

    return true;
}

// Test: Deep overlap generates contacts
TEST_CASE(test_contacts_poly_poly_deep_overlap)
{
    // Two squares with significant overlap
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(0.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    REQUIRE(hit);
    REQUIRE(manifold.count > 0);

    // With deep overlap, we should typically get 2 contacts
    REQUIRE(manifold.count <= 2);

    return true;
}

// Test: Manifold is cleared before use
TEST_CASE(test_contacts_poly_poly_manifold_cleared)
{
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    manifold.count = 999;  // Pre-set to invalid value

    pv2 normal = pv2_make(1.0f, 0.0f);

    bool hit = ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    // Count should be set from 0, not added to existing value
    REQUIRE(hit);
    REQUIRE(manifold.count >= 0);
    REQUIRE(manifold.count <= 2);

    return true;
}

// Test: Normalized normal input handling
TEST_CASE(test_contacts_poly_poly_with_normalized_normal)
{
    ph_poly_t poly_a = make_square(pv2_make(0.0f, 0.0f), 2.0f);
    ph_poly_t poly_b = make_square(pv2_make(1.5f, 0.0f), 2.0f);

    ph_manifold_t manifold = {0};
    // Provide a normalized normal
    pv2 normal = pv2_normalize(pv2_make(3.0f, 4.0f));

    ph_contacts_poly_poly(&poly_a, &poly_b, normal, &manifold);

    // Should still work with normalized normal
    REQUIRE(manifold.count >= 0);
    REQUIRE(manifold.count <= 2);

    return true;
}

TEST_SUITE(suite_contacts)
{
    RUN_TEST_CASE(test_contacts_poly_poly_square_overlap);
    RUN_TEST_CASE(test_contacts_poly_poly_contact_position);
    RUN_TEST_CASE(test_contacts_poly_poly_contact_depth);
    RUN_TEST_CASE(test_contacts_poly_poly_horizontal_edge);
    RUN_TEST_CASE(test_contacts_poly_poly_vertical_edge);
    RUN_TEST_CASE(test_contacts_poly_poly_diagonal_overlap);
    RUN_TEST_CASE(test_contacts_poly_poly_max_contacts);
    RUN_TEST_CASE(test_contacts_poly_poly_triangle_square);
    RUN_TEST_CASE(test_contacts_poly_poly_deep_overlap);
    RUN_TEST_CASE(test_contacts_poly_poly_manifold_cleared);
    RUN_TEST_CASE(test_contacts_poly_poly_with_normalized_normal);
}