/**
    @file pico_hit.h
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
    - Ray casts against line segments, polygons, and circles
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

    Rays (directed line segments) can be cast against line segments, polygons,
    and circles. Aside from reporting hits, the normal at and distance to the
    point of impact is also available.

    IMPORTANT: Polygons in this library use counter-clockwise (CCW) winding. See
    the `ph_aabb_to_poly` for an example.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_HIT_IMPLEMENTATION
    > #include "pico_hit.h"

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
#define PICO_HIT_MAX_POLY_VERTS 16
#endif

/**
 * @brief A circle shape
 */
typedef struct
{
    pv2 center;    //!< Center of circle
    pfloat radius; //!< Radius of the circle
} ph_circle_t;

/**
 * @brief A polygon shape
 * Must use CCW (counter-clockwise) winding
 */
typedef struct
{
    int count;                             //!< Number of vertices in polygon
    pv2 vertices[PICO_HIT_MAX_POLY_VERTS]; //!< Polygon vertices
    pv2 normals[PICO_HIT_MAX_POLY_VERTS];  //!< Polygon edge normals
    pv2 edges[PICO_HIT_MAX_POLY_VERTS];    //!< Edges of polygon
    pv2 centroid;
} ph_poly_t;

/**
 * @brief A collision result
 * Provides information about a collision. Normals always point from the target
 * shape to the colliding shape.
 */
typedef struct
{
    pv2    normal;  //!< Normal to colliding edge (in direction of MTV)
    pfloat overlap; //!< Amount of overlap between two shapes along colliding axis (MTD)
    pv2    mtv;     //!< Minimum Translation Vector (MTV) defined by `mtv = normal * overlap
} ph_sat_t;

/**
 * @brief A contact point
 */
typedef struct
{
    pv2 point;    //!< Position of the contact in world-space
    pfloat depth; //< Depth of the contact relative to the incident edge
} ph_contact_t;

typedef struct
{
    pv2 normal;               //!< Contact normal (from SAT)
    pfloat overlap;           //!< Amount of overlap between two shapes along colliding axis (MTD)
    ph_contact_t contacts[2]; //!< Contact points (maximum of two)
    int count;                //!< Numer of contacts
} ph_manifold_t;

/**
 * @brief A ray (directed line segment)
*/
typedef struct
{
    pv2 origin; //!< The origin of the ray
    pv2 dir;    //!< The direction of the ray (normalized)
    pfloat len; //!< The length of the ray
} ph_ray_t;

/**
 *  @brief Raycast information
 */
typedef struct
{
    pv2 normal;  //!< The surface normal at the point of impact
    pfloat dist; //!< The distance from the ray's origin to the point of impact
} ph_raycast_t;

/**
 * @brief Initializes a circle
 * @param center Circle center
 * @param radius Circle radius
 */
ph_circle_t ph_make_circle(pv2 center, pfloat radius);

/**
 * @brief Initializes a polygon
 * @param count The number of vertices of the polygon
 * @param vertices The vertices of the polygon
 * @param reverse Converts a polygon with CW winding to CCW
 * @returns The polygon with the given vertices
 */
ph_poly_t ph_make_poly(const pv2 vertices[], int count, bool reverse);

/**
 * @brief Constructs a ray
 * @param pos The origin of the array
 * @param dir The direction of the ray (normalized)
 * @param len The length of the ray
 */
ph_ray_t ph_make_ray(pv2 origin, pv2 dir, pfloat len);

/**
 * @brief Converts an axis-aligned bounding box (AABB) to a polygon
 * @brief aabb The AABB
 * @returns the AABB as a polygon
 */
ph_poly_t ph_aabb_to_poly(const pb2* aabb);

/**
 * @brief Tests to see if one convex polygon overlaps with another
 * @param poly_a The colliding polygon
 * @param poly_b The target polygon
 * @param result The collision result to populate (or NULL)
 * @returns True if the polygons overlap and false otherwise
 */
bool ph_sat_poly_poly(const ph_poly_t* poly_a,
                      const ph_poly_t* poly_b,
                      ph_sat_t* result);

/**
 * @brief Tests to see if a convex polygon overlaps a circle
 * @param poly The colliding polygon
 * @param circle The target circle
 * @param result The collision result to populate (or NULL)
 * @returns True if the polygon and circle overlap, and false otherwise
 */
bool ph_sat_poly_circle(const ph_poly_t* poly,
                        const ph_circle_t* circle,
                        ph_sat_t* result);

/**
 * @brief Tests to see if a circle overlaps a polygon
 * @param circle The colliding circle
 * @param poly The target polygon
 * @param result The collision result to populate (or NULL)
 * @returns True if the circle overlaps the polygon, and false otherwise
 */
bool ph_sat_circle_poly(const ph_circle_t* circle,
                        const ph_poly_t* poly,
                        ph_sat_t* result);

/**
 * @brief Tests to see if two circles overlap
 * @param circle_a The colliding circle
 * @param circle_b The target circle
 * @param result The collision result to populate (or NULL)
 * @returns True if the circle and the other circle, and false otherwise
 */
bool ph_sat_circle_circle(const ph_circle_t* circle_a,
                          const ph_circle_t* circle_b,
                          ph_sat_t* result);

bool ph_manifold_poly_poly(const ph_poly_t* poly_a,
                           const ph_poly_t* poly_b,
                           pv2 normal,
                           ph_manifold_t* manifold);

/**
 * @brief Tests if a polygon and circle collide and generates contact information
 * @param poly The polygon
 * @param circle The circle
 * @param manifold The contact manifold to populate
 * @returns True if the polygon and circle collide, false otherwise
 */
bool ph_manifold_poly_circle(ph_poly_t *poly, ph_circle_t *circle, ph_manifold_t* manifold);

/**
 * @brief Tests if two circles collide and generates contact information
 * @param circle_a First circle
 * @param circle_b Second circle
 * @param manifold The contact manifold to populate
 * @returns True if the circles collide, false otherwise
 */
bool ph_manifold_circle_circle(const ph_circle_t* circle_a, const ph_circle_t* circle_b, ph_manifold_t* manifold);

/**
 * @brief Tests if ray intersects a (directed) line segment
 *
 * @param ray Ray to test
 * @param s1 First endpoint of segment
 * @param s2 Second endpoint of segment
 * @param raycast Normal and distance of impact (or NULL)
 * @returns True if the ray collides with the line segment and false otherwise
 */
bool ph_ray_line(const ph_ray_t* ray, pv2 s1, pv2 s2, ph_raycast_t* raycast);

/**
 * @brief Tests if ray intersects a polygon
 *
 * @param ray Ray to test
 * @param poly The target polygon
 * @param raycast Normal and distance of impact (or NULL). May terminate early if NULL
 * @returns True if the ray collides with the polygon and false otherwise
 */
bool ph_ray_poly(const ph_ray_t* ray, const ph_poly_t* poly, ph_raycast_t* raycast);

/**
 * @brief Tests if ray intersects a circle
 *
 * @param ray Ray to test
 * @param circle The target circle
 * @param raycast Normal and distance of impact (if not NULL).
 * @returns True if the ray collides with the circle and false otherwise
 */
bool ph_ray_circle(const ph_ray_t* ray, const ph_circle_t* circle, ph_raycast_t* raycast);

/**
 * @brief Finds the point along the ray at the specified distance from the origin
 */
pv2 ph_ray_at(const ph_ray_t* ray, pfloat dist);

/**
 * @brief Transforms a polygon using an affine transform
 * @param transform The transform
 * @param poly The polygon to transform
 * @returns A new polygon
 */
ph_poly_t ph_transform_poly(const pt2* transform, const ph_poly_t* poly);

/**
 * @brief Transforms a circle using an affine transform
 * @param transform The transform
 * @param poly The circle to transform
 * @returns A new circle
 */
ph_circle_t ph_transform_circle(const pt2* transform, const ph_circle_t* circle);

/**
 * @brief Returns the bounding box for the given polygon
 */
pb2 ph_poly_to_aabb(const ph_poly_t* poly);

/**
 * @brief Returns the bounding box for the given circle
 */
pb2 ph_circle_to_aabb(const ph_circle_t* circle);

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

#define PH_ASSERT PICO_HIT_ASSERT // Alias

/*=============================================================================
 * Internal function declarations
 *============================================================================*/

// Initializes a result
static void ph_init_result(ph_sat_t* result);

// Determines a polygon's limits under a projection onto an axis
static void ph_project_poly(const ph_poly_t* poly,
                            pv2 normal,
                            pfloat* min,
                            pfloat* max);

// Determines the limits of a circle's projection onto an axis
static void ph_project_circle(const ph_circle_t *circle,
                              pv2 axis,
                              pfloat *min,
                              pfloat *max);

// Calculates the overlap of two shapes from their projections on an axis
static pfloat ph_calc_overlap(pfloat min1, pfloat max1, pfloat min2, pfloat max2);

// Returns the vertex closest to a given point
static int ph_closest_vertex(const ph_poly_t *poly, pv2 point);

// 2D matrix
typedef struct
{
    pfloat a11, a12, a21, a22;
} ph_m2;

// Determinant of 2D matrix
static pfloat ph_m2_det(ph_m2 m);

// Inverse of 2D matrix
static ph_m2 ph_m2_inverse(ph_m2 m, pfloat det);

// Map 2D vector by 2D matrix
static pv2 ph_m2_map(ph_m2 m, pv2 v);

/*=============================================================================
 * Public API implementation
 *============================================================================*/

ph_circle_t ph_make_circle(pv2 center, pfloat radius)
{
    ph_circle_t circle;
    circle.center = center;
    circle.radius = radius;
    return circle;
}

ph_poly_t ph_make_poly(const pv2 vertices[], int count, bool reverse)
{
    PH_ASSERT(count <= PICO_HIT_MAX_POLY_VERTS);
    PH_ASSERT(vertices);

    ph_poly_t poly = { 0 };

    // Copy vertices
    poly.count = count;
    poly.centroid = pv2_zero();

    // Convert CW to CCW if true
    if (reverse)
    {
        for (int i = 0; i < count; i++)
        {
            poly.vertices[i] = vertices[count - i - 1];
            poly.centroid = pv2_add(poly.centroid, poly.vertices[i]);
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            poly.vertices[i] = vertices[i];
            poly.centroid = pv2_add(poly.centroid, poly.vertices[i]);
        }
    }

    poly.centroid = pv2_scale(poly.centroid, 1.0f / count);

    // Cache edges and edge normals
    for (int i = 0; i < count; i++)
    {
        pv2 v1 = vertices[i];
        pv2 v2 = vertices[(i + 1) % count];

        poly.edges[i] = pv2_sub(v2, v1);
        poly.normals[i] = pv2_perp(poly.edges[i]);
        poly.normals[i] = pv2_normalize(poly.normals[i]);
    }

    return poly;
}

ph_ray_t ph_make_ray(pv2 origin, pv2 dir, pfloat len)
{
    return (ph_ray_t){ origin, pv2_normalize(dir), len};
}

ph_poly_t ph_aabb_to_poly(const pb2* aabb)
{
    PH_ASSERT(aabb);

    // Get AABB properties
    pv2 pos = pb2_get_pos(aabb);
    pv2 size = pb2_get_size(aabb);

    // Specify AABB vertices using CCW winding
    pv2 vertices[] =
    {
        { pos.x, pos.y                   },
        { pos.x,          pos.y + size.y },
        { pos.x + size.x, pos.y + size.y },
        { pos.x + size.x, pos.y          }
    };

    return ph_make_poly(vertices, 4, false);
}

/*
 * The SAT test states that two convex polygons do not overlap if it is
 * possible to draw a non-intersecting line between them (a separating
 * axis). It is sufficient to check the normal directions of each face of the
 * polygons to see if there is overlap.
 */
bool ph_sat_poly_poly(const ph_poly_t* poly_a,
                      const ph_poly_t* poly_b,
                      ph_sat_t* result)
{
    PH_ASSERT(poly_a);
    PH_ASSERT(poly_b);

    // Reset result
    if (result)
        ph_init_result(result);

    // Test axises on poly_a
    for (int i = 0; i < poly_a->count; i++)
    {
        pfloat poly_a_min, poly_a_max, poly_b_min, poly_b_max;

        // Project poly_a onto the normal axis
        ph_project_poly(poly_a, poly_a->normals[i], &poly_a_min, &poly_a_max);

        // Project poly_b onto the normal axis
        ph_project_poly(poly_b, poly_a->normals[i], &poly_b_min, &poly_b_max);

        // Calculate the overlap of the two polygons
        pfloat overlap = ph_calc_overlap(poly_a_min, poly_a_max, poly_b_min, poly_b_max);

        // Axis is separating
        if (overlap <= 0.0f)
            return false;

        // Update result
        if (result && overlap < result->overlap)
        {
            result->overlap = overlap;
            result->normal = poly_a->normals[i];
        }
    }

    // Test axises on poly_b
    for (int i = 0; i < poly_b->count; i++)
    {
        pfloat poly_b_min, poly_b_max, poly_a_min, poly_a_max;

        // Project poly_b onto the normal axis
        ph_project_poly(poly_b, poly_b->normals[i], &poly_b_min, &poly_b_max);

        // Project poly_a the normal axis
        ph_project_poly(poly_a, poly_b->normals[i], &poly_a_min, &poly_a_max);

        // Calculate the overlap of the two polygons
        pfloat overlap = ph_calc_overlap(poly_b_min, poly_b_max, poly_a_min, poly_a_max);

        // Axis is separating
        if (overlap <= 0.0f) //FIXME (overlap <= PM_EPSILON)
            return false;

        // Update result
        if (result && overlap < result->overlap)
        {
            result->overlap = overlap;
            result->normal = poly_a->normals[i];
        }
    }

    if (result)
    {
        // Ensure the normal vector has the correct orientation
        pv2 diff = pv2_sub(poly_b->centroid, poly_a->centroid);

        if (pv2_dot(diff, result->normal) < 0.0f)
        {
            result->normal = pv2_reflect(result->normal);
        }

        result->mtv = pv2_scale(result->normal, -result->overlap);
    }

    return true;
}

/*
 * As in the poly/poly case, if an axis separates the a polygon and a circe
 * then they do not overlap (circles are essentially convex). Thus we check
 * along the face normals of the polyon to see if there is overlap. There are
 * cases, however, where this is not sufficent. In this case we check the axis
 * from the vertex closest to the center of the circle and see if there is any
 * overlap.
 */
bool ph_sat_poly_circle(const ph_poly_t* poly,
                        const ph_circle_t* circle,
                        ph_sat_t* result)
{
    // Reset result
    if (result)
        ph_init_result(result);

    for (int i = 0; i < poly->count; i++)
    {
        pv2 axis = poly->normals[i];

        float poly_min, poly_max, circle_min, circle_max;

        // Project polygon onto axis
        ph_project_poly(poly, axis, &poly_min, &poly_max);

        // Project circle onto axis
        ph_project_circle(circle, axis, &circle_min, &circle_max);

        // Calculate overlap of the polygon and circle
        float overlap = ph_calc_overlap(poly_min, poly_max, circle_min, circle_max);

        // Axis is separating
        if (overlap <= 0.0f)
            return false;

        // Update result
        if (result && overlap < result->overlap)
        {
            result->overlap = overlap;
            result->normal = axis;
        }
    }

    // Test axis from closest polygon vertex to circle center
    int closest = ph_closest_vertex(poly, circle->center);
    pv2 axis = pv2_normalize(pv2_sub(circle->center, poly->vertices[closest]));

    if (pv2_len(axis) > PM_EPSILON)
    {
        pfloat poly_min, poly_max, circle_min, circle_max;

        // Project polygon onto axis
        ph_project_poly(poly, axis, &poly_min, &poly_max);

        // Project circle onto axis
        ph_project_circle(circle, axis, &circle_min, &circle_max);

        // Calculate overlap of the polygon and circle
        pfloat overlap = ph_calc_overlap(poly_min, poly_max, circle_min, circle_max);

        // Axis is separating
        if (overlap <= 0.0f)
            return false;

        // Update result
        if (result && overlap < result->overlap)
        {
            result->overlap = overlap;
            result->normal = axis;
        }
    }

    if (result)
    {
        // Ensure the normal vector has the correct orientation
        pv2 diff = pv2_sub(circle->center, poly->centroid);


        if (pv2_dot(result->normal, diff) < 0.0f)
        {
            result->normal = pv2_reflect(result->normal);
        }

        result->mtv = pv2_scale(result->normal, -result->overlap);
    }

    return true;
}

bool ph_sat_circle_poly(const ph_circle_t* circle,
                        const ph_poly_t* poly,
                        ph_sat_t* result)
{
    PH_ASSERT(poly);
    PH_ASSERT(circle);

    bool hit = ph_sat_poly_circle(poly, circle, (result) ? result : NULL);

    if (hit && result)
    {
        // Since arguments were swapped, reversing these vectors is all that is
        // required
        result->normal = pv2_reflect(result->normal);
        result->mtv = pv2_reflect(result->mtv);
    }

    return hit;
}

bool ph_sat_circle_circle(const ph_circle_t* circle_a,
                          const ph_circle_t* circle_b,
                          ph_sat_t* result)
{
    PH_ASSERT(circle_a);
    PH_ASSERT(circle_b);

    // Reset result
    if (result)
        ph_init_result(result);

    // Position of circle_b relative to circle_a
    pv2 diff = pv2_sub(circle_b->center, circle_a->center);

    // Squared distance between circle centers
    pfloat dist2 = pv2_len2(diff);

    // Sum of radii
    pfloat total_radius = circle_a->radius + circle_b->radius;

    // Square sum of radii for optimization (avoid calculating length until we
    // have to)
    pfloat total_radius2 = total_radius * total_radius;

    // Equivalent to dist >= total_radius
    if (dist2 >= total_radius2)
        return false;

    if (result)
    {
         // Calculate distance because we need it now
        pfloat dist = pf_sqrt(dist2);

        // Calculate overlap
        pfloat overlap = total_radius - dist;

        // Normal direction is just circle_b relative to circle_a
        pv2 normal = pv2_normalize(diff);

        result->overlap = overlap;
        result->normal = normal;

        result->mtv = pv2_scale(result->normal, -result->overlap);
    }

    return true;
}

static int ph_find_best_edge(const ph_poly_t* poly, pv2 normal, pfloat* max_dot)
{
    int index = 0;
    *max_dot = PM_FLOAT_MIN;

    for (int i = 0; i < poly->count; i++)
    {
        pfloat dot = pv2_dot(poly->normals[i], normal);

        if (dot > *max_dot)
        {
            *max_dot = dot;
            index = i;
        }
    }

    return index;
}

static int ph_find_incident_edge(const ph_poly_t* poly, pv2 normal)
{
    pfloat min_dot = PM_FLOAT_MAX;

    int index = 0;

    for (int i = 0; i < poly->count; i++)
    {
        pfloat dot = pv2_dot(poly->normals[i], normal);

        if (dot < min_dot)
        {
            min_dot = dot;
            index = i;
        }
    }

    return index;
}

static int ph_clip_segment_to_line(pv2* v_in, pv2* v_out, pv2 plane_normal, pfloat offset)
{
    int num_out = 0;

    float d0 = pv2_dot(plane_normal, v_in[0]) - offset;
    float d1 = pv2_dot(plane_normal, v_in[1]) - offset;

    // Both points behind plane - keep both
    if (d0 <= 0.0f) v_out[num_out++] = v_in[0];
    if (d1 <= 0.0f) v_out[num_out++] = v_in[1];

    // Points on opposite sides - find intersection
    if (d0 * d1 < 0.0f)
    {
        pfloat alpha = d0 / (d0 - d1);
        pv2 intersection = pv2_add(v_in[0], pv2_scale(pv2_sub(v_in[1], v_in[0]), alpha));
        v_out[num_out++] = intersection;
    }

    return num_out;
}

bool ph_manifold_poly_poly(const ph_poly_t* poly_a,
                           const ph_poly_t* poly_b,
                           pv2 normal,
                           ph_manifold_t* manifold)
{
    PH_ASSERT(poly_a);
    PH_ASSERT(poly_b);
    PH_ASSERT(manifold);

    manifold->count = 0;
    manifold->normal = normal;
    manifold->overlap = 0.0f;

    pfloat max_dot_a = 0.f;
    pfloat max_dot_b = 0.f;

    int best_edge_a = ph_find_best_edge(poly_a, normal, &max_dot_a);
    int best_edge_b = ph_find_best_edge(poly_b, pv2_reflect(normal), &max_dot_b);

    const ph_poly_t* ref_poly = NULL;
    const ph_poly_t* inc_poly = NULL;
    int ref_index = 0;

    if (max_dot_a > max_dot_b)
    {
        ref_poly = poly_a;
        inc_poly = poly_b;
        ref_index = best_edge_a;
    }
    else
    {
        ref_poly = poly_b;
        inc_poly = poly_a;
        ref_index = best_edge_b;
        normal = pv2_reflect(normal);
        manifold->normal = normal;
    }

    int inc_index = ph_find_incident_edge(inc_poly, normal);

    // Reference edge vertices
    pv2 ref_v1 = ref_poly->vertices[ref_index];
    pv2 ref_v2 = ref_poly->vertices[(ref_index + 1) % ref_poly->count];

    // Incident edge vertices
    pv2 inc_v1 = inc_poly->vertices[inc_index];
    pv2 inc_v2 = inc_poly->vertices[(inc_index + 1) % inc_poly->count];

    // Reference edge tangent and normal
    pv2 ref_edge = pv2_sub(ref_v2, ref_v1);
    pv2 ref_tangent = pv2_normalize(ref_edge);
    pv2 ref_normal = pv2_perp(ref_tangent);

    if (pv2_dot(ref_normal, normal) < 0.0f)
        ref_normal = pv2_reflect(ref_normal);

    // Clip incident edge against reference side planes
    pv2 clip_in[2] = { inc_v1, inc_v2 };
    pv2 clip_out[2];

    // First clip against -tangent plane (side1)
    pv2 side_normal1 = pv2_reflect(ref_tangent); // -tangent
    pfloat offset1 = pv2_dot(side_normal1, ref_v1);
    int num = ph_clip_segment_to_line(clip_in, clip_out, side_normal1, offset1);

    if (num < 2)
        return false;

    // Then clip against +tangent plane (side2)
    pfloat offset2 = pv2_dot(ref_tangent, ref_v2);
    num = ph_clip_segment_to_line(clip_out, clip_in, ref_tangent, offset2);

    if (num < 2)
        return false;

    // Keep points that are behind the reference face plane (penetrating)
    for (int i = 0; i < num; i++)
    {
        pfloat separation = pv2_dot(ref_normal, pv2_sub(clip_in[i], ref_v1));

        if (separation <= 0.0f)
        {
            ph_contact_t* contact = &manifold->contacts[manifold->count];
            contact->point = clip_in[i];
            contact->depth = -separation;
            manifold->count++;

            if (manifold->count >= 2)
                break;
        }
    }

    return (manifold->count > 0);
}

static pv2 ph_closest_point_on_segment(pv2 a, pv2 b, pv2 p)
{
    pv2 ab = pv2_sub(b, a);
    pv2 ap = pv2_sub(p, a);

    pfloat ab_len2 = pv2_dot(ab, ab);

    if (ab_len2 < PM_EPSILON)
        return a;

    pfloat t = pv2_dot(ap, ab) / ab_len2;
    t = pf_max(0.0f, pf_min(1.0f, t));

    return pv2_add(a, pv2_scale(ab, t));
}

bool ph_manifold_poly_circle(ph_poly_t *poly, ph_circle_t *circle, ph_manifold_t* manifold)
{
    PH_ASSERT(poly);
    PH_ASSERT(circle);
    PH_ASSERT(manifold);

    ph_sat_t result = { 0 };

    if (!ph_sat_poly_circle(poly, circle, &result))
        return false;

    manifold->normal = result.normal;
    manifold->overlap = result.overlap;
    manifold->count = 1;

    pv2 closest = poly->vertices[0];
    pfloat min_dist2 = PM_FLOAT_MAX;

    // Check all edges
    for (int i = 0; i < poly->count; i++) {
        int next = (i + 1) % poly->count;

        pv2 point = ph_closest_point_on_segment(
            poly->vertices[i],
            poly->vertices[next],
            circle->center
        );

        pv2 diff = pv2_sub(point, circle->center);
        float dist2 = pv2_dot(diff, diff);

        if (dist2 < min_dist2)
        {
            min_dist2 = dist2;
            closest = point;
        }
    }

    // Use the SAT overlap for contact depth when available. This represents
    // the minimum translational distance (MTD) separating the shapes along
    // the collision normal. It handles cases where the circle is contained
    // within the polygon as well as edge/vertex contacts.
    manifold->contacts[0].depth = result.overlap;
    manifold->contacts[0].point = closest;

    return true;
}

bool ph_manifold_circle_circle(const ph_circle_t* circle_a, const ph_circle_t* circle_b, ph_manifold_t* manifold)
{
    PH_ASSERT(circle_a);
    PH_ASSERT(circle_b);
    PH_ASSERT(manifold);

    ph_sat_t result = { 0 };

    if (!ph_sat_circle_circle(circle_a, circle_b, &result))
        return false;

    manifold->normal = result.normal;
    manifold->overlap = result.overlap;
    manifold->count = 1;

    // Compute contact point as midpoint between the two surface points
    pv2 diff = pv2_sub(circle_b->center, circle_a->center);
    pfloat dist2 = pv2_len2(diff);

    pfloat dist = pf_sqrt(dist2);
    pv2 normal = pv2_zero();

    if (dist > PM_EPSILON)
        normal = pv2_scale(diff, 1.0f / dist);
    else
        normal = pv2_make(1.0f, 0.0f);

    /* Ensure manifold normal is well defined */
    manifold->normal = normal;

    pv2 pa = pv2_add(circle_a->center, pv2_scale(normal, circle_a->radius));
    pv2 pb = pv2_sub(circle_b->center, pv2_scale(normal, circle_b->radius));

    pv2 contact = pv2_scale(pv2_add(pa, pb), 0.5f);

    manifold->contacts[0].point = contact;
    manifold->contacts[0].depth = manifold->overlap;

    return true;
}

/*
    The basic idea here is to represent the rays in parametric form and
    solve a linear equation to get the parameters where they intersect.
    In this application we are only interested in the case where both of them
    are contained in the interval [0, 1]
*/
bool ph_ray_line(const ph_ray_t* ray, pv2 s1, pv2 s2, ph_raycast_t* raycast)
{
    pv2 r1 = ray->origin;
    pv2 r2 = pv2_add(ray->origin, pv2_scale(ray->dir, ray->len));

    pv2 v = pv2_sub(r2, r1);
    pv2 w = pv2_sub(s2, s1);

    ph_m2 m =
    {
        -v.x, w.x,
        -v.y, w.y,
    };

    pfloat det = ph_m2_det(m);

    if (pf_equal(det, 0.0f))
        return false;

    ph_m2 m_inv = ph_m2_inverse(m, det);

    pv2 c = pv2_sub(r1, s1);
    pv2 p = ph_m2_map(m_inv, c);

    bool hit = 0.0f <= p.x && p.x <= 1.0f &&
               0.0f <= p.y && p.y <= 1.0f;

    if (hit && raycast)
    {
        raycast->normal = pv2_normalize(pv2_perp(w)); //TODO: ensure correct orientation
        raycast->dist = p.x * ray->len;
    }

    return hit;
}

/*
    The idea behind this function is to use ph_ray_line on each of the edges
    that make up the polygon. The function can exit early if the raycast is NULL.
    Otherwise all edges of the polygon will be tested.
*/
bool ph_ray_poly(const ph_ray_t* ray, const ph_poly_t* poly, ph_raycast_t* raycast)
{
    pv2 min_normal = pv2_zero();
    pfloat min_dist = PM_FLOAT_MAX;

    bool has_hit = false;

    for (int i = 0; i < poly->count; i++)
    {
        int next = (i + 1) % poly->count;

        pv2 s1 = poly->vertices[i];
        pv2 s2 = poly->vertices[next];

        bool hit = ph_ray_line(ray, s1, s2, raycast);

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

    return false;
}

/*
    The idea behind this function is to represent a constraint, that determines
    whether the ray intersects the circle, as a polynomial. The discriminant of
    the quadratic fomula is used to test various cases corresponding to the
    location and direction of the ray, and the circle.

    Source: Real-Time Collision Detection by Christer Ericson
*/
bool ph_ray_circle(const ph_ray_t* ray, const ph_circle_t* circle, ph_raycast_t* raycast)
{
    pfloat r = circle->radius;
    pv2 m = pv2_sub(ray->origin, circle->center);
    pfloat b = pv2_dot(m, ray->dir);
    pfloat c = pv2_dot(m, m) - r * r;

    if (c > 0.0f && b > 0.0f)
        return false;

    pfloat discr = b * b - c;

    if (discr < 0.0f)
        return false;

    if (raycast)
    {
        pfloat dist = -b - pf_sqrt(discr);

        if (dist < 0.0f)
            dist = 0.0f;

        pv2 p = pv2_add(ray->origin, pv2_scale(ray->dir, dist));

        raycast->dist = dist;
        raycast->normal = pv2_normalize(pv2_sub(p, circle->center));
    }

    return true;
}

pv2 ph_ray_at(const ph_ray_t* ray, pfloat dist)
{
    return pv2_add(ray->origin, pv2_scale(ray->dir, dist));
}

ph_poly_t ph_transform_poly(const pt2* transform, const ph_poly_t* poly)
{
    pv2 vertices[poly->count];

    for (int i = 0; i < poly->count; i++)
    {
        vertices[i] = pt2_map(transform, poly->vertices[i]);
    }

    return ph_make_poly(vertices, poly->count, false);
}

ph_circle_t ph_transform_circle(const pt2* transform,
                                const ph_circle_t* circle)
{
    return ph_make_circle(pt2_map(transform, circle->center), circle->radius);
}

pb2 ph_poly_to_aabb(const ph_poly_t* poly)
{
    return pb2_enclosing(poly->vertices, poly->count);
}

pb2 ph_circle_to_aabb(const ph_circle_t* circle)
{
    pv2 half_radius = pv2_make(circle->radius / 2.0f, circle->radius / 2.0f);

    pv2 min = pv2_sub(circle->center, half_radius);
    pv2 max = pv2_add(circle->center, half_radius);

    return pb2_make_minmax(min, max);
}

/*=============================================================================
 * Internal function definitions
 *============================================================================*/

static void ph_init_result(ph_sat_t* result)
{
    PH_ASSERT(result);

    result->overlap = PM_FLOAT_MAX;
    result->normal  = pv2_zero();
    result->mtv  = pv2_zero();
}

static pfloat ph_calc_overlap(pfloat min1, pfloat max1, pfloat min2, pfloat max2)
{
    if (max1 < min2 || max2 < min1)
    {
        return 0.0f;
    }

    return pf_min(max1, max2) - pf_max(min1, min2);
}

static void ph_project_poly(const ph_poly_t* poly,
                            pv2 axis,
                            pfloat* min,
                            pfloat* max)
{
    PH_ASSERT(min);
    PH_ASSERT(max);

    pfloat dot = pv2_dot(poly->vertices[0], axis);
    *min = dot;
    *max = dot;

    // Find the minimum and maximum distance of the polygon along the normal
    for (int i = 1; i < poly->count; i++)
    {
        dot = pv2_dot(poly->vertices[i], axis);

        if (dot < *min)
            *min = dot;

        if (dot > *max)
            *max = dot;
    }
}

static void ph_project_circle(const ph_circle_t *circle, pv2 axis, pfloat *min, pfloat *max)
{
    pfloat proj = pv2_dot(axis, circle->center);
    *min = proj - circle->radius;
    *max = proj + circle->radius;
}

static int ph_closest_vertex(const ph_poly_t *poly, pv2 point)
{
    int closest = 0;
    pfloat min_dist2 = PM_FLOAT_MAX;

    for (int i = 0; i < poly->count; i++)
    {
        pv2 diff = pv2_sub(poly->vertices[i], point);
        pfloat dist2 = pv2_dot(diff, diff);

        if (dist2 < min_dist2)
        {
            min_dist2 = dist2;
            closest = i;
        }
    }

    return closest;
}

// Determinant of 2D matrix
static pfloat ph_m2_det(ph_m2 m)
{
    return m.a11 * m.a22 - m.a21 * m.a12;
}

// Inverse of 2D matrix
static ph_m2 ph_m2_inverse(ph_m2 m, pfloat det)
{
    pfloat inv_det = 1.0f / det;
    return (ph_m2) { m.a22 * inv_det, -m.a12 * inv_det, -m.a21 * inv_det, m.a11 * inv_det };
}

// Map 2D vector by 2D matrix
static pv2 ph_m2_map(ph_m2 m, pv2 v)
{
    return (pv2){ m.a11 * v.x + m.a12 * v.y, m.a21 * v.x + m.a22 * v.y };
}

#endif // PICO_HIT_IMPLEMENTATION

/*
The MIT License (MIT)

Copyright (C) 2022 - 2026 by James McLean
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
