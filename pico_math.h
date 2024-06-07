/**
    @file pico_math.h
    @brief A 2D math library for games

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Notice:
    ---------
    This library has undergone a major revision with release 2.0. Although the
    functionality is more or less the same, the naming conventions differ
    substantially. To summarize, 'pm_v2', 'pm_t2', and 'pm_b2' have all been
    replaced by 'pv2', 'pt2', and 'pb2'. These changes affect both type
    defintions as well as function names. With scalar functions 'pm_' has been
    replaced by 'pf_', for example 'pm_equal' is now 'pf_equal'. The type
    'pm_float' has been replaced by 'pfloat'. The purpose of these changes is
    largely to make type and function names more specific and compact. The old
    (and no longer maintained) version can be found at
    https://github.com/empyreanx/pico_headers_deprecated

    Features:
    ---------
    - Written in C99
    - Single header library for easy build system integration
    - Arithmetic for 2D vectors, transforms, and AABBs
    - Functions for creating and manipulating affine transformations
    - Strikes a solid balance between simplicity and performance
    - Extensive test suite
    - Permissive license (zlib or public domain)

    Summary:
    --------

    This library provides functions that act on three 2D types: vectors (pv2),
    transforms (pt2), and axis-align bounding boxes (pb2). The library also
    provides some scalar functions as well as a random number generator.

    This library aims to strike a balance between performance and simplicity.
    Most functions return by value. All vectors are passed by value. Otherwise,
    transforms and AABBs are passed by pointer. There is no dynamic memory
    allocation.

    Vector functions comprise basic vector creation and manipulation, as well
    as computing lengths, dot products, projections, and more.

    Transformation functions include functions for computing multiplications,
    determinants, inverses, as well as extracting and inserting transformation
    parameters. There are also functions for applying rotations, scaling, and
    translations to a given transform. The provided functions are sufficient for
    implementing a scene graph.

    This library provides linear interpolation for transforms, vectors, and
    scalars. Interpolating transforms can be used in a variety of contexts, for
    example, interpolated rendering when using a fixed timestep, or smoothing
    when performing networked physics.

    Bounding box functions provide tests for intersection of AABBs and
    determnining if a point is contained within a given AABB. There are
    functions for computing unions and intersections of AABBs as well as
    for computing the minimum enclosing AABB for a set of points.

    The random number generator uses the xoshiro128** algorithm, which is
    substantially better than `rand()` in terms of the quality of its output
    without sacrificing too much performance.

    Please see the unit tests for some concrete examples.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_MATH_IMPLEMENTATION
    > #include "pico_ml.h"

    to a source file (once), then simply include the header normally.
*/

#ifndef PICO_MATH_H
#define PICO_MATH_H

#include <float.h>   // FLT_MIN/MAX, DBL_MIN/MAX
#include <math.h>    // sqrt(f), cos(f), sin(f), atan2(f)...
#include <stdbool.h> // bool, true, false
#include <stdint.h>  // uint32_t

#ifdef __cplusplus
extern "C" {
#endif

// Common

#if defined(_MSC_VER)
    #define PM_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define PM_INLINE static inline __attribute((always_inline))
#else
    #define PM_INLINE
#endif

/*==============================================================================
 * Types, constants, and function aliases
 *============================================================================*/

#ifdef PICO_MATH_DOUBLE
    /// @brief A double precision floating point number
    typedef double pfloat;

    #define PM_EPSILON 1e-7

    #define PM_PI   3.14159265358979323846264338327950288
    #define PM_PI2 (2.0 * PM_PI)
    #define PM_E    2.71828182845904523536028747135266250

    #define PM_FLOAT_MIN DBL_MIN
    #define PM_FLOAT_MAX DBL_MAX

    #define pf_sqrt  sqrt
    #define pf_cos   cos
    #define pf_sin   sin
    #define pf_acos  acos
    #define pf_asin  asin
    #define pf_atan2 atan2
    #define pf_abs   fabs
    #define pf_fmod  fmod
    #define pf_exp   exp
    #define pf_pow   pow
    #define pf_floor floor
    #define pf_ceil  ceil
    #define pf_log2  log2
    #define pf_max   fmax
    #define pf_min   fmin

#else
    /// @brief A single precision floating point number
    typedef float pfloat;

    #define PM_EPSILON 1e-5f

    #define PM_PI   3.14159265359f
    #define PM_PI2 (2.0f * PM_PI)
    #define PM_E    2.71828182845f

    #define PM_FLOAT_MIN FLT_MIN
    #define PM_FLOAT_MAX FLT_MAX

    #define pf_sqrt  sqrtf
    #define pf_cos   cosf
    #define pf_sin   sinf
    #define pf_acos  acosf
    #define pf_asin  asinf
    #define pf_atan2 atan2f
    #define pf_abs   fabsf
    #define pf_fmod  fmodf
    #define pf_exp   expf
    #define pf_pow   powf
    #define pf_floor floorf
    #define pf_ceil  ceilf
    #define pf_log2  log2f
    #define pf_max   fmaxf
    #define pf_min   fminf
#endif

/*==============================================================================
 * Data structures
 *============================================================================*/

/**
 * @brief A 2D vector
 */
typedef struct
{
    pfloat x, y;
} pv2;

/**
 * @brief A 2D transform
 */
typedef struct
{
    pfloat t00, t10, t01, t11, tx, ty;
} pt2;

/**
 * @brief A 2D axis-aligned-bounding-box (AABB)
 */
typedef struct
{
    pv2 min, max;
} pb2;

/*==============================================================================
 * Scalar functions and macros
 *============================================================================*/

/**
 * @brief Clamps the value to the given range
 */
PM_INLINE pfloat pf_clamp(pfloat val, pfloat min, pfloat max)
 {
    return ((val < min) ? min : ((val > max) ? max : val));
 }

/**
 * @brief Computes the sign of the number
 *
 * @returns:
 * -1 if `val` is less than zero
 *  0 if `val` is equal to zero
 *  1 if `val` is greater than zero
 */
PM_INLINE pfloat pf_sign(pfloat val)
{
    return ((0 == val) ? 0.0f : ((val > 0.0f) ? 1.0f : -1.0f));
}

/**
 * @brief Returns `true` if the values are within epsilon of one another
 */
PM_INLINE bool pf_equal(pfloat c1, pfloat c2)
{
    return pf_abs(c1 - c2) < PM_EPSILON;
}

/**
 * @brief Linearly interpolates the two values
 * @param a One endpoint
 * @param b Another endpoint
 * @param alpha A number in [0, 1] that specifies the position between the
 * endpoints
 */
PM_INLINE pfloat pf_lerp(pfloat a, pfloat b, pfloat alpha)
{
    return a + (b - a) * alpha;
}

/**
 * @brief Linearly interpolates between two angles
 *
 * @param angle1 The first endpoint
 * @param angle2 The second endpoint
 * @param alpha The normalized distance between angle1 and angle2
 */
pfloat pf_lerp_angle(pfloat angle1, pfloat angle2, pfloat alpha);

/**
 * @brief Clamps the angle to be in [0, 2 * PI]
 */
PM_INLINE pfloat pf_normalize_angle(pfloat angle)
{
    while (angle >= PM_PI2)
        angle -= PM_PI2;

    while (angle < 0.0f)
        angle += PM_PI2;

    return angle;
}

/*==============================================================================
 * Vectors
 *============================================================================*/

/**
 * @brief Constructs a vector
 */
#define pv2_make(x, y) ((const pv2){ x, y })

/**
 * @brief Returns true if the vectors are equal (within epsilon)
 */
PM_INLINE bool pv2_equal(pv2 v1, pv2 v2)
{
    return pf_equal(v1.x, v2.x) &&
           pf_equal(v1.y, v2.y);
}

/**
 * @brief Adds two vectors
 * @param v1 First vector
 * @param v2 Second vector
 */
PM_INLINE pv2 pv2_add(pv2 v1, pv2 v2)
{
    return pv2_make(v1.x + v2.x, v1.y + v2.y);
}

/**
 * @brief Subtracts two vectors
 * @param v1 First vector
 * @param v2 Second vector
 */
PM_INLINE pv2 pv2_sub(pv2 v1, pv2 v2)
{
    return pv2_make(v1.x - v2.x, v1.y - v2.y);
}

/**
 * @brief Scales a vector
 * @param v Vector to scale
 * @param c The scale factor
 */
PM_INLINE pv2 pv2_scale(pv2 v, pfloat c)
{
    return pv2_make(v.x * c, v.y * c);
}

/**
 * @brief Dot product
 */
PM_INLINE pfloat pv2_dot(pv2 v1, pv2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

/**
 * @brief Returns the square of the length of the vector
 */
PM_INLINE pfloat pv2_len2(pv2 v)
{
    return pv2_dot(v, v);
}

/**
 * @brief Returns the length of the vector
 */
PM_INLINE pfloat pv2_len(pv2 v)
{
    return pf_sqrt(pv2_len2(v));
}

/**
 * @brief Normalizes a vector (sets its length to one)
 * @param v The vector to normalize
 * @returns The normalized vector
 */
PM_INLINE pv2 pv2_normalize(pv2 v)
{
    pfloat c = pv2_len(v);

    if (c < PM_EPSILON)
        return pv2_make(0.0f, 0.0f);
    else
        return pv2_scale(v, 1.0f / c);
}

/**
  * @brief Negates a vector (scales it by -1.0)
  * @param v The vector to negate
  * @returns The negated vecotor
  */
PM_INLINE pv2 pv2_reflect(pv2 v)
{
    return pv2_scale(v, -1.0f);
}

/**
 * @brief Construct a vector that is perpendicular to the specified vector
 * @param v The vector to be made perpendicular
 * @returns The perpendicular vector
 */
PM_INLINE pv2 pv2_perp(pv2 v)
{
    return pv2_make(-v.y, v.x);
}

/**
 * @brief A 2D analog of the 3D cross product
 */
PM_INLINE pfloat pv2_cross(pv2 v1, pv2 v2)
{
    pv2 perp = pv2_perp(v1);
    return pv2_dot(perp, v2);
}

/**
 * @brief Returns the angle the vector with respect to the current basis
 */
PM_INLINE pfloat pv2_angle(pv2 v)
{
    return pf_atan2(v.y, v.x);
}

/**
 * @brief Projects a vector onto another
 * @param v1 The vector to be projected
 * @param v2 The vector to project onto
 * @returns The projection of v1 onto v2
 */
PM_INLINE pv2 pv2_proj(pv2 v1, pv2 v2)
{
    pfloat d = pv2_dot(v1, v2) / pv2_dot(v2, v2);
    return pv2_scale(v2, d);
}

/**
 * @brief Returns the distance between the two vectors
 */
PM_INLINE pfloat pv2_dist(pv2 v1, pv2 v2)
{
    pv2 v = pv2_sub(v1, v2);
    return pv2_len(v);
}

/**
 * @brief Linearly interpolates between two vectors
 * @param v1 The first endpoint
 * @param v2 The second endpoint
 * @param alpha The normalized distance between v1 and v2
 */
PM_INLINE pv2 pv2_lerp(pv2 v1, pv2 v2, pfloat alpha)
{
    pv2 out;
    out.x = pf_lerp(v1.x, v2.x, alpha);
    out.y = pf_lerp(v1.y, v2.y, alpha);
    return out;
}

/**
 * @brief Returns the zero vector
 */
#define pv2_zero() (pv2_make(0.0f, 0.0f))

/**
 * @brief Contructs a vector in polar coordinates
 */
PM_INLINE pv2 pv2_polar(pfloat angle, pfloat len)
{
    return pv2_make(len * pf_cos(angle), len * pf_sin(angle));
}

/**
 * @brief Computes the component-wise minimum of two vectors
 */
PM_INLINE pv2 pv2_min(pv2 v1, pv2 v2)
{
    return pv2_make(pf_min(v1.x, v2.x), pf_min(v1.y, v2.y));
}

/**
 * @brief Computes the component-wise maximum of two vectors
 */
PM_INLINE pv2 pv2_max(pv2 v1, pv2 v2)
{
    return pv2_make(pf_max(v1.x, v2.x), pf_max(v1.y, v2.y));
}

/**
 * @brief Computes the component-wise floor of the specified vector
 */
PM_INLINE pv2 pv2_floor(pv2 v)
{
    return pv2_make(pf_floor(v.x), pf_floor(v.y));
}

/**
 * @brief Computes the component-wise ceiling of the specified vector
 */
PM_INLINE pv2 pv2_ceil(pv2 v)
{
    return pv2_make(pf_ceil(v.x), pf_ceil(v.y));
}

/*==============================================================================
 * 2D Affine Transforms
 *============================================================================*/

/**
 * @brief Constructs a 2D transform
 */
#define pt2_make(t00, t01, tx, t10, t11, ty) ((const pt2){ t00, t10, t01, t11, tx, ty })

/**
 * @brief Return the identity transform
 */
PM_INLINE pt2 pt2_identity(void)
{
    return pt2_make(1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f);
}

/**
 * @brief Returns true if the transforms are equal (within epsilon)
 */
bool pt2_equal(const pt2* t1, const pt2* t2);

/**
 * @brief Gets the translation components of the transform
 * @param t Pointer to the transform
 */
PM_INLINE pv2 pt2_get_pos(const pt2* t)
{
    return pv2_make(t->tx, t->ty);
}

/**
 * @brief Sets the translation components of the transform
 * @param t Pointer to the transform
 * @param pos The position vector
 */
PM_INLINE void pt2_set_pos(pt2* t, pv2 pos)
{
    t->tx = pos.x;
    t->ty = pos.y;
}

/**
 * @brief Gets the angle of rotation of the transform
 */
PM_INLINE pfloat pt2_get_angle(const pt2* t)
{
    return pf_normalize_angle(pf_atan2(t->t10, t->t00));
}

/**
 * @brief Sets the scale of the transform
 *
 * Scalings are now assumed to be pre-multiplied. This change was made because
 * the common case is usually a tranlation to the origin, followed by scaling,
 * then a rotation and finally, another translation.
 *
 * @param t The transform
 * @param scale The vector containing scale factors in the x/y directions
 */
void pt2_set_scale(pt2* t, pv2 scale);

/**
 * @brief Gets the scale of the transform
 *
 * Scalings are now assumed to be pre-multiplied. This change was made because
 * the common case is usually a tranlation to the origin, followed by scaling,
 * then a rotation and finally, another translation.
 *
 * @param t The transform
 */
pv2 pt2_get_scale(const pt2* t);

/**
 * @brief Sets the angle of the transform
 */
void pt2_set_angle(pt2* t, pfloat angle);

/**
 * @brief Transforms a vector
 * @param t The transform
 * @param v The vector to be transformed
 */
PM_INLINE pv2 pt2_map(const pt2* t, pv2 v)
{
    pv2 out;
    out.x = t->t00 * v.x + t->t01 * v.y + t->tx;
    out.y = t->t10 * v.x + t->t11 * v.y + t->ty;
    return out;
}

/**
 * @brief Returns the determinant of the transform
 */
PM_INLINE pfloat pt2_det(const pt2* t)
{
    return t->t00 * t->t11 - t->t01 * t->t10;
}

/**
 * @brief Calculates the inverse of the transform
 * @param t The transform to invert
 */
pt2 pt2_inv(const pt2* t);

/**
 * @brief Composes two transformations
 */
pt2 pt2_mult(const pt2* t1, const pt2* t2);

/**
 * @brief Linearly interpolates two transforms
 */
pt2 pt2_lerp(const pt2* t1, const pt2* t2, pfloat alpha);

/**
 * @brief Constructs a scaling transform
 * @param scale The scaling components
 */
PM_INLINE pt2 pt2_scaling(pv2 scale)
{
     return pt2_make(scale.x, 0.0f,    0.0f,
                     0.0f,    scale.y, 0.0f);
}

/**
 * @brief Constructs a rotation transform
 * @param angle The angle of rotation
 */
PM_INLINE pt2 pt2_rotation(pfloat angle)
{
    pfloat c = pf_cos(angle);
    pfloat s = pf_sin(angle);

    return pt2_make(c, -s, 0.0f,
                    s,  c, 0.0f);
}

/**
 * @brief Constructs a translation transform
 * @param pos The translation coordinates
 */
PM_INLINE pt2 pt2_translation(pv2 pos)
{
    return pt2_make(1.0f, 0.0f, pos.x,
                    0.0f, 1.0f, pos.y);
}

/**
 * @brief Scales a transform
 * @param t The transform to scale
 * @param scale The scaling parameters
 */
PM_INLINE void pt2_scale(pt2* t, pv2 scale)
{
    pt2 scaling = pt2_scaling(scale);
    *t = pt2_mult(&scaling, t);
}

/**
 * @brief Applies a rotation to a transform
 * @param t The transform to rotate
 * @param angle The angle to rotate by
 */
PM_INLINE void pt2_rotate(pt2* t, pfloat angle)
{
    pt2 rotation = pt2_rotation(angle);
    *t = pt2_mult(&rotation, t);
}

/**
 * @brief Applies a translation a transform
 * @param t The transform to translate
 * @param pos The translation components
 */
PM_INLINE void pt2_translate(pt2* t, pv2 pos)
{
    pt2 translation = pt2_translation(pos);
    *t = pt2_mult(&translation, t);
}

/*==============================================================================
 * 2D Box (AABB)
 *============================================================================*/

#define pb2_make_minmax(min, max) ((const pb2){ min, max })

/**
 * @brief Constructs a 2D box (rectangle)
 */
#define pb2_make(x, y, w, h) ((const pb2){ { x, y }, { x + w, y + h } })

/**
 * @brief Returns an AABB that has zero size and coordinates
 */
#define pb2_zero() (pb2_make(0.0f, 0.0f, 0.0f, 0.0f))

/**
 * @brief Returns the position of an AABB
 */
PM_INLINE pv2 pb2_get_pos(const pb2* b)
{
    return b->min;
}

/**
 * @brief Returns the dimensions of an AABB
 */
PM_INLINE pv2 pb2_get_size(const pb2* b)
{
    return pv2_sub(b->max, b->min);
}

/**
 * @brief Sets the position of an AABB
 * @param b The AABB
 * @param pos The new position
 */
PM_INLINE void pb2_set_pos(pb2* b, pv2 pos)
{
    pv2 size = pb2_get_size(b);
    *b = pb2_make(pos.x, pos.y, size.x, size.y);
}

/**
 * @brief Sets the dimensions of an AABB
 * @param b The AABB
 * @param size The new size
 */
PM_INLINE void pb2_set_size(pb2* b, pv2 size)
{
    pv2 pos = pb2_get_pos(b);
    *b = pb2_make(pos.x, pos.y, size.x, size.y);
}

/**
 * @brief Returns `true` if the bounding boxes are equal (within epsilon)
 */
bool pb2_equal(const pb2* b1, const pb2* b2);

/**
 * @brief Computes the union of `b1` and `b2
 */
pb2 pb2_combine(const pb2* b1, const pb2* b2);

/**
 * @brief Computes the intersection of `b1` and `b2`
 */
pb2 pb2_overlap(const pb2* b1, const pb2* b2);

/**
 * @brief Return `true` if the two bounding boxes intersect
 */
PM_INLINE bool pb2_overlaps(const pb2* b1, const pb2* b2)
{
    return b1->max.x >= b2->min.x &&
           b1->max.y >= b2->min.y &&
           b2->max.x >= b1->min.x &&
           b2->max.y >= b1->min.y;
}

/**
 * @brief Returns `true` if the first box is contained within the second.
 */
PM_INLINE bool pb2_contains(const pb2* b1, const pb2* b2)
{
    return b2->min.x >= b1->min.x &&
           b2->min.y >= b1->min.y &&
           b2->max.x <= b1->max.x &&
           b2->max.y <= b1->max.y;
}

/**
 * @brief Returns `true` if the box contains the point `v`
 */
PM_INLINE bool pb2_contains_point(const pb2* b, pv2 v)
{
    pfloat x = v.x;
    pfloat y = v.y;

    return x > b->min.x &&
           y > b->min.y &&
           x < b->max.x &&
           y < b->max.y;
}

/**
 * @brief Returns the area of the box
 */
PM_INLINE pfloat pb2_area(const pb2* b)
{
    return (b->max.x - b->min.x) * (b->max.y - b->min.y);
}

/**
 * @brief Computes the center of the box
 */
PM_INLINE pv2 pb2_center(const pb2* b)
{
    pv2 offset = pv2_scale(pv2_sub(b->max, b->min), 1.0f / 2.0f);
    return pv2_add(offset, b->min);
}

/**
 * @brief Computes the minimum box containing all of the vertices
 * @param verts The vertices
 * @param count The number of vertices
 */
pb2 pb2_enclosing(const pv2 verts[], int count);

/**
 * @brief Computes the minimum AABB obtained by transforming the vertices of
 * the specified AABB
 */
pb2 pb2_transform(const pt2* t, const pb2* b);

/**
 * @brief The pseudo random number generator (RNG) state
 */
typedef struct prng_t
{
    uint32_t s[4];
} prng_t;

/**
 * @brief Initialize and seed the RNG
 * @param rng A reference to the RNG
 * @param seed The seed (choosing the same seed will yield identical sequences)
 */
void prng_seed(prng_t* rng, uint64_t seed);

/**
 * @brief Generates a pseudo random number in [0, UINT32_MAX]
 * @param rng A reference to the RNG
 */
uint32_t prng_random(prng_t* rng);

/**
 * @brief Generates a psuedo random number in [0, 1]
 */
pfloat pf_random(prng_t* rng);

#ifdef __cplusplus
}
#endif

#endif // PICO_MATH_H

#ifdef PICO_MATH_IMPLEMENTATION

pfloat pf_lerp_angle(pfloat angle1, pfloat angle2, pfloat alpha)
{
    const pv2 v1 = pv2_make(pf_cos(angle1), pf_sin(angle1));
    const pv2 v2 = pv2_make(pf_cos(angle2), pf_sin(angle2));

    // Calculuate cosine of angle between the two vectors
    pfloat dot = pf_clamp(pv2_dot(v1, v2), -1.0f, 1.0f);

    // LERP if the cosine is too close to its limits
    if (pf_equal(dot, 1.0f) || pf_equal(dot, -1.0f))
    {
        pv2 tmp = pv2_lerp(v1, v2, alpha);
        return pf_normalize_angle(pf_atan2(tmp.y, tmp.x));
    }

    // Calculate angle
    pfloat angle = pf_acos(dot) * alpha;

    // Gram-Schmidt(construct a new vector 'v0' that is orthogonal to 'v1')
    pv2 v0  = pv2_sub(v2, pv2_scale(v1, dot));
    v0 = pv2_normalize(v0);

    // Calcuate vector in new coordinate system
    pv2 tmp1 = pv2_scale(v1, pf_cos(angle));
    pv2 tmp2 = pv2_scale(v0, pf_sin(angle));

    pv2 tmp = pv2_add(tmp1, tmp2);

    // Calculate new angle
    return pf_normalize_angle(pf_atan2(tmp.y, tmp.x));
}

bool pt2_equal(const pt2* t1, const pt2* t2)
{
    return pf_equal(t1->t00, t2->t00) &&
           pf_equal(t1->t10, t2->t10) &&
           pf_equal(t1->t01, t2->t01) &&
           pf_equal(t1->t11, t2->t11) &&
           pf_equal(t1->tx,  t2->tx)  &&
           pf_equal(t1->ty,  t2->ty);
}

void pt2_set_scale(pt2* t, pv2 scale)
{
    pfloat angle = pt2_get_angle(t);

    pfloat c = pf_cos(angle);
    pfloat s = pf_sin(angle);

    pfloat sx = scale.x;
    pfloat sy = scale.y;

    t->t00 = sx * c; t->t01 = sy * -s;
    t->t10 = sx * s; t->t11 = sy *  c;
}

pv2 pt2_get_scale(const pt2* t)
{
    pfloat angle = pt2_get_angle(t);
    pfloat cos_sign = pf_sign(pf_cos(angle));

    pv2 out;

    if (0.0f == cos_sign) //TODO: pf_equal?
    {
        out.x =  t->t10;
        out.y = -t->t01;
        return out;
    }

    pv2 v1 = pv2_make(t->t00, t->t10);
    pv2 v2 = pv2_make(t->t01, t->t11);

    out.x = pf_sign(t->t00) * cos_sign * pv2_len(v1);
    out.y = pf_sign(t->t11) * cos_sign * pv2_len(v2);

    return out;
}

void pt2_set_angle(pt2* t, pfloat angle)
{
    pfloat c = pf_cos(angle);
    pfloat s = pf_sin(angle);

    pv2 scale = pt2_get_scale(t);

    pfloat sx = scale.x;
    pfloat sy = scale.y;

    t->t00 = sx * c; t->t01 = sy * -s;
    t->t10 = sx * s; t->t11 = sy *  c;
}

pt2 pt2_inv(const pt2* t)
{
    pfloat det = pt2_det(t);

    if (0.0f == det) // Intentionally not using epsilon because determinants
    {                // can be really small and still be valid
        return pt2_identity();
    }

    pfloat inv_det = 1.0f / det;

    pt2 out;

    out.t00 =  t->t11 * inv_det; out.t01 = -t->t01 * inv_det;
    out.t10 = -t->t10 * inv_det; out.t11 =  t->t00 * inv_det;

    out.tx = (t->t01 * t->ty - t->t11 * t->tx) * inv_det;
    out.ty = (t->t10 * t->tx - t->t00 * t->ty) * inv_det;

    return out;
}

pt2 pt2_mult(const pt2* t1, const pt2* t2)
{
    pt2 out;

    out.t00 = t1->t00 * t2->t00 + t1->t01 * t2->t10;
    out.t10 = t1->t10 * t2->t00 + t1->t11 * t2->t10;

    out.t01 = t1->t00 * t2->t01 + t1->t01 * t2->t11;
    out.t11 = t1->t10 * t2->t01 + t1->t11 * t2->t11;

    out.tx = t1->t00 * t2->tx + t1->t01 * t2->ty + t1->tx;
    out.ty = t1->t10 * t2->tx + t1->t11 * t2->ty + t1->ty;

    return out;
}

pt2 pt2_lerp(const pt2* t1, const pt2* t2, pfloat alpha)
{
    pv2 scale1 = pt2_get_scale(t1);
    pv2 scale2 = pt2_get_scale(t2);

    pfloat angle1 = pt2_get_angle(t1);
    pfloat angle2 = pt2_get_angle(t2);

    pv2 pos1 = pt2_get_pos(t1);
    pv2 pos2 = pt2_get_pos(t2);

    pv2 scale = pv2_lerp(scale1, scale2, alpha);
    pv2 pos = pv2_lerp(pos1, pos2, alpha);
    pfloat angle = pf_lerp_angle(angle1, angle2, alpha);

    pfloat c = pf_cos(angle);
    pfloat s = pf_sin(angle);

    pfloat sx = scale.x;
    pfloat sy = scale.y;

    pfloat tx = pos.x;
    pfloat ty = pos.y;

    pt2 out;

    out.t00 = sx * c; out.t01 = sy * -s; out.tx = tx;
    out.t10 = sx * s; out.t11 = sy *  c; out.ty = ty;

    return out;
}

bool pb2_equal(const pb2* b1, const pb2* b2)
{
    return pv2_equal(b1->min, b2->min) && pv2_equal(b1->max, b2->max);
}

pb2 pb2_combine(const pb2* b1, const pb2* b2)
{
    pv2 min = pv2_min(b1->min, b2->min);
    pv2 max = pv2_max(b1->max, b2->max);
    return pb2_make_minmax(min, max);
}

pb2 pb2_overlap(const pb2* b1, const pb2* b2)
{
    if (!pb2_overlaps(b1, b2))
        return pb2_make(0.0f, 0.0f, 0.0f, 0.0f);

    pv2 min = pv2_max(b1->min, b2->min);
    pv2 max = pv2_min(b1->max, b2->max);
    return pb2_make_minmax(min, max);
}

pb2 pb2_enclosing(const pv2 verts[], int count)
{
    if (0 == count)
        return pb2_make(0.0f, 0.0f, 0.0f, 0.0f);

    pv2 min = verts[0];
    pv2 max = verts[0];

    for (int i = 1; i < count; i++)
    {
        min = pv2_min(min, verts[i]);
        max = pv2_max(max, verts[i]);
    }

    return pb2_make_minmax(min, max);
}

pb2 pb2_transform(const pt2* t, const pb2* b)
{
    pv2 pos  = pb2_get_pos(b);
    pv2 size = pb2_get_size(b);

    pfloat w = size.x;
    pfloat h = size.y;

    pv2 verts[4];

    verts[0] = pos;
    verts[1] = pv2_make(pos.x,     pos.y + h);
    verts[2] = pv2_make(pos.x + w, pos.y + h);
    verts[3] = pv2_make(pos.x + w, pos.y);

    verts[0] = pt2_map(t, verts[0]);
    verts[1] = pt2_map(t, verts[1]);
    verts[2] = pt2_map(t, verts[2]);
    verts[3] = pt2_map(t, verts[3]);

    return pb2_enclosing(verts, 4);
}

/*
 * Implementation of the xoshiro128** algorithm
 * https://en.wikipedia.org/wiki/Xorshift
 */

void prng_seed(prng_t* rng, uint64_t seed)
{
    for (int i = 0; i < 2; i++)
    {
        uint64_t result  = (seed += 0x9E3779B97f4A7C15);
        result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
        result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
        result = result  ^ (result >> 31);

        int j = i * 2;

        rng->s[j]     = (uint32_t)result;
        rng->s[j + 1] = (uint32_t)(result >> 32);
    }
}

static uint32_t rng_rol32(uint32_t x, int k)
{
	return (x << k) | (x >> (32 - k));
}

uint32_t prng_random(prng_t* rng)
{
	uint32_t *s = rng->s;
	uint32_t const result = rng_rol32(s[1] * 5, 7) * 9;
	uint32_t const t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = rng_rol32(s[3], 45);

	return result;
}

pfloat pf_random(prng_t* rng)
{
    return (pfloat)prng_random(rng) / (pfloat)UINT32_MAX;
}

#endif // PICO_MATH_IMPLEMENTATION

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2021 James McLean

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be appreciated
    but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.

    ----------------------------------------------------------------------------

    (B) Public Domain (www.unlicense.org)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to the
    public domain. We make this dedication for the benefit of the public at
    large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all present
    and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

