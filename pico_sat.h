/**
    @file pico_sat.h
    @brief Separating Axis Theorem (SAT) Tests written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in C99
    - Single header library for easy build system integration
    - Tests overlaps for AABBs, polygons, and circles using SAT
    - Provides collision information including the normal and amount of overlap
    - Permissive license (MIT)

    Summary:
    --------
    The Separating Axis Theorem (SAT) roughly states that two convex shapes do
    not intersect if there is no axis separating them. In the case of simple
    shapes the theorem provides necessary and sufficient conditions. For
    example, in the case of convex polygons, it is sufficient to test the axises
    along the edge normals of both polygons.

    SAT tests are reasonably efficient and are frequently used for static,
    narrow phase, collision detection in games.

    This library provides SAT tests for polygons, AABBs (which are, of course,
    polygons), and circles. Manifold objects can be passed to test functions so
    that, in the case of a collision, they will contain the colliding edge
    normal, overlap (minimum translational distance), and a vector (minimum
    translation vector).

    IMPORTANT: Polygons in this library use counter-clockwise (CCW) winding. See
    the `sat_aabb_to_poly` for an example.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_SAT_IMPLEMENTATION
    > #include "pico_sat.h"

    to a source file (once), then simply include the header normally.

    Dependencies:
    -------------

    This library depends on "pico_math.h", which must be in the include path.
    You must also add

    > #define PICO_MATH_IMPLEMENTATION
    > #include "pico_math.h"

    to the same or other source file (once).

    Todo:
    -----
    - pm_b2 sat_polygon_to_aabb(const sat_polygon_t* poly);
    - pm_b2 sat_circle_to_aabb(const sat_circle_t* circle);
*/

#ifndef PICO_SAT_H
#define PICO_SAT_H

#include <float.h>

#include "pico_math.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of vertices in a polygon
#ifndef PICO_SAT_MAX_POLY_VERTS
#define PICO_SAT_MAX_POLY_VERTS 8
#endif

/**
 * @brief A circle shape
 */
typedef struct
{
    pm_v2  pos;      //!< Center of circle
    pm_float radius; //!< Radius of the circle
} sat_circle_t;

/**
 * @brief A polygon shape
 * Must use CCW (counter-clockwise) winding
 */
typedef struct
{
    int   vertex_count;                      //!< Number of vertices in polygon
    pm_v2 vertices[PICO_SAT_MAX_POLY_VERTS]; //!< Polygon vertices
    pm_v2 normals[PICO_SAT_MAX_POLY_VERTS];  //!< Polygon edge normals
    pm_v2 edges[PICO_SAT_MAX_POLY_VERTS];    //!< Edges of polygon
} sat_poly_t;

/**
 * @brief A collision manifold
 * Provides information about a collision
 */
typedef struct
{
    pm_v2    normal;  //!< Normal to colliding edge (in direction of MTV)
    pm_float overlap; //!< Amount of overlap between two shapes along colliding axis (MTD)
    pm_v2    vector;  //!< Vector defined by `vector = normal * overlap`
} sat_manifold_t;

/**
 * @brief Initializes a circle
 * @param pos    Circle center
 * @param radius Circle radius
 */
sat_circle_t sat_make_circle(pm_v2 pos, pm_float radius);

/**
 * @brief Initializes a polygon
 * @param vertex_count The number of vertices of the polygon
 * @param vertices     The vertices of the polygon (must use CCW winding)
 * @returns The polygon with the given vertices
 */
sat_poly_t sat_make_polygon(int vertex_count, pm_v2 vertices[]);

/**
 * @brief Converts and axis-aligned bounding box (AABB) to a polygon
 * @brief aabb The AABB
 * @returns the AABB as a polygon
 */
sat_poly_t sat_aabb_to_poly(const pm_b2* aabb);

/**
 * @brief Tests to see if one polygon overlaps with another
 * @param poly1    The colliding polygon
 * @param poly2    The target polygon
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the polygons overlap and false otherwise
 */
bool sat_test_poly_poly(const sat_poly_t* poly1,
                        const sat_poly_t* poly2,
                        sat_manifold_t* manifold);

/**
 * @brief Tests to see if a polygon overlaps a circle
 * @param poly     The colliding polygon
 * @param circle   The target circle
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the polygon and circle overlap, and false otherwise
 */
bool sat_test_poly_circle(const sat_poly_t* poly,
                          const sat_circle_t* circle,
                          sat_manifold_t* manifold);

/**
 * @brief Tests to see if a circle overlaps a polygon
 * @param circle   The colliding circle
 * @param poly     The target polygon
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the circle overlaps the polygon, and false otherwise
 */
bool sat_test_circle_poly(const sat_circle_t* circle,
                          const sat_poly_t* poly,
                          sat_manifold_t* manifold);

/**
 * @brief Tests to see if two circles overlap
 * @param circle1  The colliding circle
 * @param circle2  The target circle
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the circle and the other circle, and false otherwise
 */
bool sat_test_circle_circle(const sat_circle_t* circle1,
                            const sat_circle_t* circle2,
                            sat_manifold_t* manifold);

#ifdef __cplusplus
}
#endif

#endif // PICO_SAT_H

#ifdef PICO_SAT_IMPLEMENTATION // Define once

#ifdef NDEBUG
    #define PICO_SAT_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_SAT_ASSERT
        #include <assert.h>
        #define PICO_SAT_ASSERT(expr) (assert(expr))
    #endif
#endif

#define SAT_ASSERT PICO_SAT_ASSERT // Alias

/*=============================================================================
 * Internal function declarations
 *============================================================================*/

// Initializes a manifold
static void sat_init_manifold(sat_manifold_t* manifold);

// Updates manifold if requried
static void sat_update_manifold(sat_manifold_t* manifold, pm_v2 normal, pm_float overlap);

// Determines the polygon's limits when projected onto the normal vector
static void sat_axis_range(const sat_poly_t* poly, pm_v2 normal, pm_float range[2]);

// Determines the amount overlap of the polygons along the specified axis
static pm_float sat_axis_overlap(const sat_poly_t* poly1, const sat_poly_t* poly2, pm_v2 axis);

// Line Voronoi regions
typedef enum
{
    SAT_VORONOI_LEFT,
    SAT_VORONOI_RIGHT,
    SAT_VORONOI_MIDDLE
} sat_voronoi_region_t;

// Determines the Voronoi region the point belongs to along the specified line
//
//            |       (0)      |
//     (-1)  [L]--------------[R]  (1)
//            |       (0)      |
//
static sat_voronoi_region_t sat_voronoi_region(pm_v2 point, pm_v2 line);

/*=============================================================================
 * Public API implementation
 *============================================================================*/

sat_circle_t sat_make_circle(pm_v2 pos, pm_float radius)
{
    sat_circle_t circle;
    circle.pos = pos;
    circle.radius = radius;
    return circle;
}

sat_poly_t sat_make_poly(int vertex_count, pm_v2 vertices[])
{
    SAT_ASSERT(vertex_count <= PICO_SAT_MAX_POLY_VERTS);
    SAT_ASSERT(vertices);

    sat_poly_t poly;

    // Copy vertices
    poly.vertex_count = vertex_count;

    for (int i = 0; i < vertex_count; i++)
    {
        poly.vertices[i] = vertices[i];
    }

    // Cache edges and edge normals
    for (int i = 0; i < vertex_count; i++)
    {
        int next = (i + 1) == vertex_count ? 0 : i + 1;

        pm_v2 v1 = vertices[i];
        pm_v2 v2 = vertices[next];
        poly.edges[i] = pm_v2_sub(v2, v1);
        poly.normals[i] = pm_v2_perp(poly.edges[i]);
        poly.normals[i] = pm_v2_normalize(poly.normals[i]);
    }

    return poly;
}

sat_poly_t sat_aabb_to_poly(const pm_b2* aabb)
{
    SAT_ASSERT(aabb);

    // Get AABB properties
    pm_v2 pos = pm_b2_pos(aabb);
    pm_v2 size = pm_b2_size(aabb);

    // Specify AABB vertices using CCW winding
    pm_v2 vertices[] =
    {
        { pos.x, pos.y                   },
        { pos.x,          pos.y + size.y },
        { pos.x + size.x, pos.y + size.y },
        { pos.x + size.x, pos.y          }
    };

    return sat_make_poly(4, vertices);
}

bool sat_test_poly_poly(const sat_poly_t* poly1,
                        const sat_poly_t* poly2,
                        sat_manifold_t* manifold)
{
    SAT_ASSERT(poly1);
    SAT_ASSERT(poly2);

    if (manifold)
        sat_init_manifold(manifold);

    // Test axises on poly1
    for (int i = 0; i < poly1->vertex_count; i++)
    {
        // Get signed overlap of poly1 on poly2 along specified normal direction
        pm_float overlap = sat_axis_overlap(poly1, poly2, poly1->normals[i]);

        // Axis is separating, polygons do not overlap
        if (overlap == 0.0f)
            return false;

        // Update manifold information with new overlap and normal
        if (manifold)
            sat_update_manifold(manifold, poly2->normals[i], overlap);
    }

    // Test axises on poly2
    for (int i = 0; i < poly2->vertex_count; i++)
    {
        // Get signed overlap of poly2 on poly1 along specified normal direction
        pm_float overlap = sat_axis_overlap(poly2, poly1, poly2->normals[i]);

        // Axis is separating, polygons do not overlap
        if (overlap == 0.0f)
            return false;

        // Update manifold information with new overlap and normal
        if (manifold)
            sat_update_manifold(manifold, poly2->normals[i], overlap);
    }

    return true;
}

bool sat_test_poly_circle(const sat_poly_t* poly,
                          const sat_circle_t* circle,
                          sat_manifold_t* manifold)
{
    SAT_ASSERT(poly);
    SAT_ASSERT(circle);

    if (manifold)
        sat_init_manifold(manifold);

    pm_float radius2 = circle->radius * circle->radius;

    // The main idea behind this function is to classify the position of the
    // circle according to the Voronoi region(s) it is part of using inexpensive
    // checks. Essentially it efficiently narrows down the position of the
    // circle so that the correct separating axis test can be invoked.

    int count = poly->vertex_count;

    for (int i = 0; i < count; i++)
    {
        int next = (i + 1) == count ? 0 : i + 1;
        int prev = (i - 1) <= 0 ? count - 1 : i - 1;

        // Edge to test
        pm_v2 edge = poly->edges[i];

        // Postion of circle relative to vertex
        pm_v2 point = pm_v2_sub(circle->pos, poly->vertices[i]);

        // Find the Voronoi region of the point (circle center relative to
        // vertex)  with respect to edge
        sat_voronoi_region_t region = sat_voronoi_region(point, edge);

        // Test if point is in the left Voronoi region
        if (region == SAT_VORONOI_LEFT)
        {
            // If it is, check if it is in the right Voronoi region of the
            // previous edge. If this is the case, the circle is "sandwiched"
            // in the intersection of the Voronoi regions defined by the
            // endpoints of the edges.

            pm_v2 point2 = pm_v2_sub(circle->pos, poly->vertices[prev]);
            edge = poly->edges[prev];

            region = sat_voronoi_region(point2, edge);

            if (region == SAT_VORONOI_RIGHT)
            {
                // The circle center is in the left/right Voronoi region, so
                // check to see if it contains the vertex
                pm_float diff2 = pm_v2_len2(point);

                // Equivalent to diff > radius
                if (diff2 > radius2)
                    return false;

                // Vertex is contained within circle, so the circle overlaps the
                // polygon

                if (manifold)
                {
                    // Calculate distance because we need it now
                    pm_float diff = pm_sqrt(diff2);

                    // Calculate overlap
                    pm_float overlap = circle->radius - diff;

                    // Normal direction is just the circle relative to the vertex
                    pm_v2 normal = pm_v2_normalize(point);

                    // Update manifold
                    sat_update_manifold(manifold, normal, overlap);
                }
            }
        }
        // This case is symmetric to the above
        else if (region == SAT_VORONOI_RIGHT)
        {
            pm_v2 point2 = pm_v2_sub(circle->pos, poly->vertices[next]);
            edge = poly->edges[next];

            region = sat_voronoi_region(point2, edge);

            if (region == SAT_VORONOI_LEFT)
            {
                pm_float diff2 = pm_v2_len2(point);

                if (diff2 > radius2)
                    return false;

                if (manifold)
                {
                    pm_float diff = pm_sqrt(diff2);
                    pm_float overlap = circle->radius - diff;
                    pm_v2 normal = pm_v2_normalize(point);
                    sat_update_manifold(manifold, normal, overlap);
                }
            }
        }
        else // SAT_VORONOI_MIDDLE
        {
            // In this case, the location of the circle is between the endpoints
            // of an edge.
            pm_v2 normal = poly->normals[i];

            // Location of center of circle along the edge normal
            pm_float diff = pm_v2_dot(normal, point);
            pm_float abs_diff = pm_abs(diff);

            // Test if circle does not intersect edge
            if (diff > 0.0f && abs_diff > circle->radius)
                return false;

            if (manifold)
            {
                // Calculate overlap
                pm_float overlap = circle->radius - diff;

                // Update manifold
                sat_update_manifold(manifold, normal, overlap);
            }
        }
    }

    return true;
}

bool sat_test_circle_poly(const sat_circle_t* circle,
                          const sat_poly_t* poly,
                          sat_manifold_t* manifold)
{
    SAT_ASSERT(poly);
    SAT_ASSERT(circle);

    bool collides = sat_test_poly_circle(poly, circle, (manifold) ? manifold : NULL);

    if (manifold)
    {
        // Since arguments were swapped, reversing the vectors is all that is
        // required
        manifold->normal = pm_v2_neg(manifold->normal);
        manifold->vector = pm_v2_neg(manifold->vector);
    }

    return collides;
}

bool sat_test_circle_circle(const sat_circle_t* circle1,
                            const sat_circle_t* circle2,
                            sat_manifold_t* manifold)
{
    SAT_ASSERT(circle1);
    SAT_ASSERT(circle2);

    if (manifold)
        sat_init_manifold(manifold);

    // Position of circle2 relative to circle1
    pm_v2 diff = pm_v2_sub(circle2->pos, circle1->pos);

    // Squared distance between circle centers
    pm_float dist2 = pm_v2_len2(diff);

    // Sum of radii
    pm_float total_radius = circle1->radius + circle2->radius;

    // Square sum of radii for optimization
    pm_float total_radius2 = total_radius * total_radius;

    // Equivalent to dist >= total_radius
    if (dist2 >= total_radius2)
        return false;

    if (manifold)
    {
         // Calculate distance because we need it now
        pm_float dist = pm_sqrt(dist2);

        // Calculate overlap
        pm_float overlap = total_radius - dist;

        // Normal direction is just circle2 relative to circle1
        pm_v2 normal = pm_v2_normalize(diff);

        // Update manifold
        sat_update_manifold(manifold, normal, overlap);
    }

    return true;
}

/*=============================================================================
 * Internal function definitions
 *============================================================================*/

static void sat_init_manifold(sat_manifold_t* manifold)
{
    SAT_ASSERT(manifold);

    manifold->overlap = FLT_MAX;
    manifold->normal  = pm_v2_zero();
    manifold->vector  = pm_v2_zero();
}

static void sat_update_manifold(sat_manifold_t* manifold, pm_v2 normal, pm_float overlap)
{
    SAT_ASSERT(manifold);

    pm_float abs_overlap = pm_abs(overlap);

    // Only update if the new overlap is smaller than the old one
    if (abs_overlap < manifold->overlap)
    {
        // Update overlap (always positive)
        manifold->overlap = abs_overlap;

        // If the overlap is less that zero the normal must be reversed
        if (overlap < 0.0f)
            manifold->normal = pm_v2_neg(normal);
        else if (overlap > 0.0f)
            manifold->normal = normal;

        manifold->vector = pm_v2_scale(manifold->normal, manifold->overlap);
    }
}

static void sat_axis_range(const sat_poly_t* poly, pm_v2 normal, pm_float range[2])
{
    SAT_ASSERT(poly);
    SAT_ASSERT(range);

    pm_float dot = pm_v2_dot(poly->vertices[0], normal);
    pm_float min = dot;
    pm_float max = dot;

    // Find the minimum and maximum distance of the polygon along the normal
    for (int i = 1; i < poly->vertex_count; i++)
    {
        dot = pm_v2_dot(poly->vertices[i], normal);

        if (dot < min)
            min = dot;

        if (dot > max)
            max = dot;
    }

    // The range defines the interval induced by the polygon projected onto the
    // normal vector
    range[0] = min;
    range[1] = max;
}

static pm_float sat_axis_overlap(const sat_poly_t* poly1,
                                 const sat_poly_t* poly2,
                                 pm_v2 axis)

{
    SAT_ASSERT(poly1);
    SAT_ASSERT(poly2);

    pm_float range1[2];
    pm_float range2[2];

    // Get the ranges of polygons projected onto the axis vector
    sat_axis_range(poly1, axis, range1);
    sat_axis_range(poly2, axis, range2);

    // Ranges do not overlaps
    if (range1[1] < range2[0] || range2[1] < range1[0])
        return 0.0f;

    // Calculate overlap candidates
    pm_float overlap1 = range1[1] - range2[0];
    pm_float overlap2 = range2[1] - range1[0];

    // Return the smaller overlap
    return (overlap2 > overlap1) ? overlap1 : -overlap2;
}

static sat_voronoi_region_t sat_voronoi_region(pm_v2 point, pm_v2 line)
{
    pm_float len2 = pm_v2_len2(line);
    pm_float dot  = pm_v2_dot(point, line);

    if (dot < 0.0f)                 // Point is to the left of the line
        return SAT_VORONOI_LEFT;
    else if (dot > len2)            // Point is to the right of the line
        return SAT_VORONOI_RIGHT;
    else
        return SAT_VORONOI_MIDDLE;  // Point is somewhere in the middle
}

#endif // PICO_SAT_IMPLEMENTATION

/*
The MIT License (MIT)

Copyright (C) 2022 by James McLean
Copyright (C) 2012 - 2015 by Jim Riecken

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

