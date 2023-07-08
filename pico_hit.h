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
    not intersect if there is an axis separating them. In the case of simple
    shapes the theorem provides necessary and sufficient conditions. For
    example, in the case of convex polygons, it is sufficient to test the axises
    along the edge normals of both polygons.

    SAT tests are reasonably efficient and are frequently used for static,
    narrow phase, collision detection in games.

    This library provides SAT tests for polygons, AABBs (which are, of course,
    polygons), and circles. Manifold objects can be passed to test functions so
    that, in the case of a collision, they will contain the colliding edge
    normal, overlap (minimum translational distance or MTD), and a vector
    (minimum translation vector or MTV).

    IMPORTANT: Polygons in this library use counter-clockwise (CCW) winding. See
    the `ph_aabb_to_poly` for an example.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_HIT_IMPLEMENTATION
    > #include "pico_sat.h"

    to a source file (once), then simply include the header normally.

    Dependencies:
    -------------

    This library depends on "pico_math.h", which must be in the include path.
    You must also add

    > #define PICO_MATH_IMPLEMENTATION
    > #include "pico_math.h"

    to the same or other source file (once).
*/

#ifndef PICO_HIT_H
#define PICO_HIT_H

#include "pico_math.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of vertices in a polygon
#ifndef PICO_HIT_MAX_POLY_VERTS
#define PICO_HIT_MAX_POLY_VERTS 8
#endif

/**
 * @brief A circle shape
 */
typedef struct
{
    pm_v2  pos;      //!< Center of circle
    pm_float radius; //!< Radius of the circle
} ph_circle_t;

/**
 * @brief A polygon shape
 * Must use CCW (counter-clockwise) winding
 */
typedef struct
{
    int   vertex_count;                      //!< Number of vertices in polygon
    pm_v2 vertices[PICO_HIT_MAX_POLY_VERTS]; //!< Polygon vertices
    pm_v2 normals[PICO_HIT_MAX_POLY_VERTS];  //!< Polygon edge normals
    pm_v2 edges[PICO_HIT_MAX_POLY_VERTS];    //!< Edges of polygon
} ph_poly_t;

typedef struct
{
    pm_v2 pos;
    pm_v2 dir;
    pm_float dist;
} ph_ray_t;

/**
 * @brief A collision manifold
 * Provides information about a collision. Normals always point from shape 1 to
 * shape 2.
 */
typedef struct
{
    pm_v2    normal;  //!< Normal to colliding edge (in direction of MTV)
    pm_float overlap; //!< Amount of overlap between two shapes along colliding axis (MTD)
    pm_v2    vector;  //!< Vector defined by `vector = normal * overlap`
} ph_manifold_t;

typedef struct
{
    pm_v2 normal;
    pm_float dist;
} ph_raycast_t;

/**
 * @brief Initializes a circle
 * @param pos    Circle center
 * @param radius Circle radius
 */
ph_circle_t ph_make_circle(pm_v2 pos, pm_float radius);

/**
 * @brief Initializes a polygon
 * @param vertex_count The number of vertices of the polygon
 * @param vertices     The vertices of the polygon (must use CCW winding)
 * @returns The polygon with the given vertices
 */
ph_poly_t ph_make_poly(const pm_v2 vertices[], int vertex_count);

ph_ray_t ph_make_ray(pm_v2 pos, pm_v2 dir, pm_float dist);


/**
 * @brief Converts and axis-aligned bounding box (AABB) to a polygon
 * @brief aabb The AABB
 * @returns the AABB as a polygon
 */
ph_poly_t ph_aabb_to_poly(const pm_b2* aabb);

/**
 * @brief Tests to see if one polygon overlaps with another
 * @param poly_a    The colliding polygon
 * @param poly_b    The target polygon
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the polygons overlap and false otherwise
 */
bool ph_sat_poly_poly(const ph_poly_t* poly_a,
                      const ph_poly_t* poly_b,
                      ph_manifold_t* manifold);

/**
 * @brief Tests to see if a polygon overlaps a circle
 * @param poly     The colliding polygon
 * @param circle   The target circle
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the polygon and circle overlap, and false otherwise
 */
bool ph_sat_poly_circle(const ph_poly_t* poly,
                        const ph_circle_t* circle,
                        ph_manifold_t* manifold);

/**
 * @brief Tests to see if a circle overlaps a polygon
 * @param circle   The colliding circle
 * @param poly     The target polygon
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the circle overlaps the polygon, and false otherwise
 */
bool ph_sat_circle_poly(const ph_circle_t* circle,
                        const ph_poly_t* poly,
                        ph_manifold_t* manifold);

/**
 * @brief Tests to see if two circles overlap
 * @param circle_a  The colliding circle
 * @param circle_b  The target circle
 * @param manifold The collision manifold to populate (or NULL)
 * @returns True if the circle and the other circle, and false otherwise
 */
bool ph_sat_circle_circle(const ph_circle_t* circle_a,
                          const ph_circle_t* circle_b,
                          ph_manifold_t* manifold);

/**
 * @brief Transforms a polygon using an affine transform
 * @param transform The transform
 * @param poly      The polygon to transform
 * @returns A new polygon
 */
ph_poly_t ph_transform_poly(const pm_t2* transform, const ph_poly_t* poly);

bool ph_ray_segment(const ph_ray_t* ray, pm_v2 s1, pm_v2 s2, ph_raycast_t* raycast);
bool ph_ray_poly(const ph_ray_t* ray, const ph_poly_t* poly, ph_raycast_t* raycast);
bool ph_ray_circle(pm_v2 r1, pm_v2 r2, const ph_circle_t* circle, ph_raycast_t* raycast);

/**
 * @brief Transforms a circle using an affine transform
 * @param transform The transform
 * @param poly      The circle to transform
 * @returns A new circle
 */
ph_circle_t ph_transform_circle(const pm_t2* transform, const ph_circle_t* circle);

/**
 * @brief Returns the bounding box for the given polygon
 */
pm_b2 ph_poly_to_aabb(const ph_poly_t* poly);

/**
 * @brief Returns the bounding box for the given circle
 */
pm_b2 ph_circle_to_aabb(const ph_circle_t* circle);

#ifdef __cplusplus
}
#endif

#endif // PICO_HIT_H

#ifdef PICO_HIT_IMPLEMENTATION // Define once

#ifdef NDEBUG
    #define PICO_HIT_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_HIT_ASSERT
        #include <assert.h>
        #define PICO_HIT_ASSERT(expr) (assert(expr))
    #endif
#endif

#define SAT_ASSERT PICO_HIT_ASSERT // Alias

/*=============================================================================
 * Internal function declarations
 *============================================================================*/

// Initializes a manifold
static void ph_init_manifold(ph_manifold_t* manifold);

// Updates manifold if requried
static void ph_update_manifold(ph_manifold_t* manifold,
                               pm_v2 normal,
                               pm_float overlap);

// Determines the polygon's limits when projected onto the normal vector
static void ph_axis_range(const ph_poly_t* poly, pm_v2 normal, pm_float range[2]);

// Determines the amount overlap of the polygons along the specified axis
static pm_float ph_axis_overlap(const ph_poly_t* poly_a,
                                const ph_poly_t* poly_b,
                                pm_v2 axis);

// Line Voronoi regions
typedef enum
{
    PH_VORONOI_LEFT,
    PH_VORONOI_RIGHT,
    PH_VORONOI_MIDDLE
} ph_voronoi_region_t;

// Determines the Voronoi region the point belongs to along the specified line
//
//            |       (0)      |
//     (-1)  [L]--------------[R]  (1)
//            |    ^  (0)      |
//               line
static ph_voronoi_region_t ph_voronoi_region(pm_v2 point, pm_v2 line);

/*=============================================================================
 * Public API implementation
 *============================================================================*/

ph_circle_t ph_make_circle(pm_v2 pos, pm_float radius)
{
    ph_circle_t circle;
    circle.pos = pos;
    circle.radius = radius;
    return circle;
}

ph_poly_t ph_make_poly(const pm_v2 vertices[], int vertex_count)
{
    SAT_ASSERT(vertex_count <= PICO_HIT_MAX_POLY_VERTS);
    SAT_ASSERT(vertices);

    ph_poly_t poly;

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

ph_ray_t ph_make_ray(pm_v2 pos, pm_v2 dir, pm_float dist)
{
    return (ph_ray_t){ pos, pm_v2_normalize(dir), dist };
}

ph_poly_t ph_aabb_to_poly(const pm_b2* aabb)
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

    return ph_make_poly(vertices, 4);
}

bool ph_sat_poly_poly(const ph_poly_t* poly_a,
                      const ph_poly_t* poly_b,
                      ph_manifold_t* manifold)
{
    SAT_ASSERT(poly_a);
    SAT_ASSERT(poly_b);

    if (manifold)
        ph_init_manifold(manifold);

    // Test axises on poly_a
    for (int i = 0; i < poly_a->vertex_count; i++)
    {
        // Get signed overlap of poly_a on poly_b along specified normal direction
        pm_float overlap = ph_axis_overlap(poly_a, poly_b, poly_a->normals[i]);

        // Axis is separating, polygons do not overlap
        if (overlap == 0.0f)
            return false;

        // Update manifold information with new overlap and normal
        if (manifold)
            ph_update_manifold(manifold, poly_b->normals[i], overlap);
    }

    // Test axises on poly_b
    for (int i = 0; i < poly_b->vertex_count; i++)
    {
        // Get signed overlap of poly_b on poly_a along specified normal direction
        pm_float overlap = ph_axis_overlap(poly_b, poly_a, poly_b->normals[i]);

        // Axis is separating, polygons do not overlap
        if (overlap == 0.0f)
            return false;

        // Update manifold information with new overlap and normal
        if (manifold)
            ph_update_manifold(manifold, poly_b->normals[i], overlap);
    }

    return true;
}

bool ph_sat_poly_circle(const ph_poly_t* poly,
                        const ph_circle_t* circle,
                        ph_manifold_t* manifold)
{
    SAT_ASSERT(poly);
    SAT_ASSERT(circle);

    if (manifold)
        ph_init_manifold(manifold);

    pm_float radius2 = circle->radius * circle->radius;

    // The main idea behind this function is to classify the position of the
    // circle relative to the Voronoi region(s) it is part of. This uses very
    // inexpensive operations. Essentially it efficiently narrows down the
    // position of the circle so that the correct separating axis test can be
    // performed.

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
        // vertex)  with respect to the edge
        ph_voronoi_region_t region = ph_voronoi_region(point, edge);

        // Test if point is in the left Voronoi region
        if (region == PH_VORONOI_LEFT)
        {
            // If it is, check if it is in the right Voronoi region of the
            // previous edge. If this is the case, the circle is "sandwiched"
            // in the intersection of the Voronoi regions defined by the
            // endpoints of the edges.

            pm_v2 point2 = pm_v2_sub(circle->pos, poly->vertices[prev]);
            edge = poly->edges[prev];

            region = ph_voronoi_region(point2, edge);

            if (region == PH_VORONOI_RIGHT)
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
                    ph_update_manifold(manifold, normal, overlap);
                }
            }
        }
        // This case is symmetric to the above
        else if (region == PH_VORONOI_RIGHT)
        {
            pm_v2 point2 = pm_v2_sub(circle->pos, poly->vertices[next]);
            edge = poly->edges[next];

            region = ph_voronoi_region(point2, edge);

            if (region == PH_VORONOI_LEFT)
            {
                pm_float diff2 = pm_v2_len2(point);

                if (diff2 > radius2)
                    return false;

                if (manifold)
                {
                    pm_float diff = pm_sqrt(diff2);
                    pm_float overlap = circle->radius - diff;
                    pm_v2 normal = pm_v2_normalize(point);
                    ph_update_manifold(manifold, normal, overlap);
                }
            }
        }
        else // PH_VORONOI_MIDDLE
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
                ph_update_manifold(manifold, normal, overlap);
            }
        }
    }

    return true;
}

bool ph_sat_circle_poly(const ph_circle_t* circle,
                        const ph_poly_t* poly,
                        ph_manifold_t* manifold)
{
    SAT_ASSERT(poly);
    SAT_ASSERT(circle);

    bool collides = ph_sat_poly_circle(poly, circle, (manifold) ? manifold : NULL);

    if (manifold)
    {
        // Since arguments were swapped, reversing these vectors is all that is
        // required
        manifold->normal = pm_v2_reflect(manifold->normal);
        manifold->vector = pm_v2_reflect(manifold->vector);
    }

    return collides;
}

bool ph_sat_circle_circle(const ph_circle_t* circle_a,
                          const ph_circle_t* circle_b,
                          ph_manifold_t* manifold)
{
    SAT_ASSERT(circle_a);
    SAT_ASSERT(circle_b);

    if (manifold)
        ph_init_manifold(manifold);

    // Position of circle_b relative to circle_a
    pm_v2 diff = pm_v2_sub(circle_b->pos, circle_a->pos);

    // Squared distance between circle centers
    pm_float dist2 = pm_v2_len2(diff);

    // Sum of radii
    pm_float total_radius = circle_a->radius + circle_b->radius;

    // Square sum of radii for optimization (avoid calculating length until we
    // have to)
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

        // Normal direction is just circle_b relative to circle_a
        pm_v2 normal = pm_v2_normalize(diff);

        // Update manifold
        ph_update_manifold(manifold, normal, overlap);
    }

    return true;
}

ph_poly_t ph_transform_poly(const pm_t2* transform, const ph_poly_t* poly)
{
    pm_v2 vertices[poly->vertex_count];

    for (int i = 0; i < poly->vertex_count; i++)
    {
        vertices[i] = pm_t2_map(transform, poly->vertices[i]);
    }

    return ph_make_poly(vertices, poly->vertex_count);
}

ph_circle_t ph_transform_circle(const pm_t2* transform,
                                const ph_circle_t* circle)
{
    return ph_make_circle(pm_t2_map(transform, circle->pos), circle->radius);
}

pm_b2 ph_poly_to_aabb(const ph_poly_t* poly)
{
    return pm_b2_enclosing(poly->vertices, poly->vertex_count);
}

pm_b2 ph_circle_to_aabb(const ph_circle_t* circle)
{
    pm_v2 half_radius = pm_v2_make(circle->radius / 2.0f, circle->radius / 2.0f);

    pm_v2 min = pm_v2_sub(circle->pos, half_radius);
    pm_v2 max = pm_v2_add(circle->pos, half_radius);

    return pm_b2_make_minmax(min, max);
}

/*=============================================================================
 * Internal function definitions
 *============================================================================*/

static void ph_init_manifold(ph_manifold_t* manifold)
{
    SAT_ASSERT(manifold);

    manifold->overlap = PM_FLOAT_MAX;
    manifold->normal  = pm_v2_zero();
    manifold->vector  = pm_v2_zero();
}

static void ph_update_manifold(ph_manifold_t* manifold, pm_v2 normal, pm_float overlap)
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
            manifold->normal = pm_v2_reflect(normal);
        else if (overlap > 0.0f)
            manifold->normal = normal;

        manifold->vector = pm_v2_scale(manifold->normal, manifold->overlap);
    }
}

static void ph_axis_range(const ph_poly_t* poly, pm_v2 normal, pm_float range[2])
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

static pm_float ph_axis_overlap(const ph_poly_t* poly_a,
                                const ph_poly_t* poly_b,
                                pm_v2 axis)

{
    SAT_ASSERT(poly_a);
    SAT_ASSERT(poly_b);

    pm_float range_a[2];
    pm_float range_b[2];

    // Get the ranges of polygons projected onto the axis vector
    ph_axis_range(poly_a, axis, range_a);
    ph_axis_range(poly_b, axis, range_b);

    // Ranges do not overlaps
    if (range_a[1] < range_b[0] || range_b[1] < range_a[0])
        return 0.0f;

    // Calculate overlap candidates
    pm_float overlap1 = range_a[1] - range_b[0];
    pm_float overlap2 = range_b[1] - range_a[0];

    // Return the smaller overlap
    return (overlap2 > overlap1) ? overlap1 : -overlap2;
}

static ph_voronoi_region_t ph_voronoi_region(pm_v2 point, pm_v2 line)
{
    pm_float len2 = pm_v2_len2(line);
    pm_float dot  = pm_v2_dot(point, line);

    if (dot < 0.0f)                 // Point is to the left of the line
        return PH_VORONOI_LEFT;
    else if (dot > len2)            // Point is to the right of the line
        return PH_VORONOI_RIGHT;
    else
        return PH_VORONOI_MIDDLE;  // Point is somewhere in the middle
}

typedef struct
{
    pm_float a11, a12, a21, a22;
} ph_m2;

pm_float ph_m2_det(ph_m2 m)
{
    return m.a11 * m.a22 - m.a21 * m.a12;
}

ph_m2 ph_m2_inverse(ph_m2 m, float det)
{
    pm_float inv_det = 1.0f / det;
    return (ph_m2) { m.a22 * inv_det, -m.a12 * inv_det, -m.a21 * inv_det, m.a11 * inv_det };
}

pm_v2 ph_m2_map(ph_m2 m, pm_v2 v)
{
    return (pm_v2){ m.a11 * v.x + m.a12 * v.y, m.a21 * v.x + m.a22 * v.y };
}

bool ph_ray_segment(const ph_ray_t* ray, pm_v2 s1, pm_v2 s2, ph_raycast_t* raycast)
{
    pm_v2 r1 = ray->pos;
    pm_v2 r2 = pm_v2_add(ray->pos, pm_v2_scale(ray->dir, ray->dist));

    pm_v2 v = pm_v2_sub(r2, r1);
    pm_v2 w = pm_v2_sub(s2, s1);

    ph_m2 m =
    {
        -v.x, w.x,
        -v.y, w.y,
    };

    pm_float det = ph_m2_det(m);

    if (pm_equal(det, 0.0f))
        return false;

    ph_m2 m_inv = ph_m2_inverse(m, det);

    pm_v2 c = pm_v2_sub(r1, s1);
    pm_v2 p = ph_m2_map(m_inv, c);

    bool hit = 0.0f <= p.x && p.x <= 1.0f &&
               0.0f <= p.y && p.y <= 1.0f;

    if (hit && raycast)
    {
        raycast->normal = pm_v2_normalize(pm_v2_perp(w));
        raycast->dist  = p.x * ray->dist;
    }

    return hit;
}

bool ph_ray_poly(const ph_ray_t* ray, const ph_poly_t* poly, ph_raycast_t* raycast)
{
    pm_v2 min_normal = pm_v2_zero();
    pm_float min_dist = PM_FLOAT_MAX;

    bool has_hit = false;

    for (int i = 0; i < poly->vertex_count; i++)
    {
        int next = (i + 1) == poly->vertex_count ? 0 : i + 1;

        pm_v2 s1 = poly->vertices[next];
        pm_v2 s2 = poly->vertices[i];

        bool hit = ph_ray_segment(ray, s1, s2, raycast);

        if (hit && raycast)
        {
            has_hit = true;

            if (raycast->dist < min_dist)
            {
                min_dist = raycast->dist;
                min_normal = raycast->normal;
            }
        }
        else if (hit)
        {
            return true;
        }
    }

    if (has_hit)
    {
        raycast->dist = min_dist;
        raycast->normal = min_normal;
        return true;
    }
    else
    {
        return false;
    }
}

#endif // PICO_HIT_IMPLEMENTATION

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

