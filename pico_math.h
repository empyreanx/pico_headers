/**
    @file pico_math.h
    @brief A 2D math library for games

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

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

    This library provides functions that act on three 2D types: vectors (pm_v2),
    transforms (pm_t2), and axis-align bounding boxes (pm_b2). The library also
    provides some scalar functions as well as a random number generator.

    The library aims to strike a balance between performance and simplicity.
    Most functions return by value, whereas most arguments are passed by
    pointer. An exception to this are vector arithmetic functions (add, sub,
    scale), and transform functions (translate and scale) which have arguments
    passed by value. There is no dynamic memory allocation.

    Vector functions comprise basic vector creation and manipulation.Transforms,
    on the other hand, are more advanced. The functions provided are sufficient
    for implementing a scene graph.

    Another feature of this library is that it provides linear interpolation for
    transforms, vectors, and scalars. Interpolating transforms can be used in a
    variety of contexts, for example, interpolated rendering when using a fixed
    timestep, or smoothing when doing networked physics.

    Bounding box functions provide tests for intersection of AABBs and
    determnining if a point is contained within a given AABB. There are
    functions for computing unions and intersections of AABBs as well as
    for computing the minimum enclosing AABB for a set of points.

    The random number generator uses the Mersenne Twister algorithm, which is
    substantially better than `rand()` in terms of the quality of its output.
    It is slower than `rand()` but still has pretty decent performance.

    Please see the unit tests for some concrete examples.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PM_IMPLEMENTATION
    > #include "pico_ml.h"

    to a source file (once), then simply include the header normally.

    Todo:
    -----
    - pm_b2_zero
    - pm_b2_transform
    - pm_v2_floor/pm_v2_ceil
    - pm_v2_min/pm_v2_max
    - pm_t2_make2(pos, angle, scale)
*/

#ifndef PICO_ML_H
#define PICO_ML_H

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
    #define PM_INLINE inline
#endif

// Types

#ifdef PM_USE_DOUBLE
    /// @brief A double precision floating point number
    typedef double pm_flt;

    #define PM_EPSILON 1e-7

    #define PM_PI   3.14159265358979323846264338327950288
    #define PM_PI2 (2.0 * PM_PI)
    #define PM_E    2.71828182845904523536028747135266250

    #define pm_sqrt  sqrt
    #define pm_cos   cos
    #define pm_sin   sin
    #define pm_acos  acos
    #define pm_asin  asin
    #define pm_atan2 atan2
    #define pm_abs   fabs
    #define pm_fmod  fmod
    #define pm_exp   exp
    #define pm_pow   pow
    #define pm_floor floor
    #define pm_ceil  ceil
    #define pm_log2  log2

#else
    /// @brief A single precision floating point number
    typedef float pm_flt;

    #define PM_EPSILON 1e-5f

    #define PM_PI   3.14159265359f
    #define PM_PI2 (2.0f * PM_PI)
    #define PM_E    2.71828182845f

    #define pm_sqrt  sqrtf
    #define pm_cos   cosf
    #define pm_sin   sinf
    #define pm_acos  acosf
    #define pm_asin  asinf
    #define pm_atan2 atan2f
    #define pm_abs   fabsf
    #define pm_fmod  fmodf
    #define pm_exp   expf
    #define pm_pow   powf
    #define pm_floor floorf
    #define pm_ceil  ceilf
    #define pm_log2  log2f
#endif

/**
 * @brief A 2D vector
 */
typedef struct
{
    pm_flt x, y;
} pm_v2;

/**
 * @brief A 2D transform
 */
typedef struct
{
    pm_flt t00, t10, t01, t11, tx, ty;
} pm_t2;

/**
 * @brief A 2D axis-aligned-bounding-box (AABB)
 */
typedef struct
{
    pm_flt x, y, w, h;
} pm_b2;

/*==============================================================================
 * Scalar functions and macros
 *============================================================================*/

/**
 * @brief Computes the minimum of the two numbers
 */
#define pm_min(a, b) (a < b ? a : b)

/**
 * @brief Computes the maximum of the two number
 */
#define pm_max(a, b) (a > b ? a : b)

/**
 * @brief Clamps the value to the given range
 */
#define pm_clamp(val, min, max) ((val < min) ? min : ((val > max) ? max : val))

/**
 * @brief Computes the sign of the number
 *
 * @returns:
 * -1 if `val` is less than zero
 *  0 if `val` is equal to zero
 *  1 if `val` is greater than zero
 */
#define pm_sign(val) ((0 == val) ? 0 : ((val > 0) ? 1 : -1))

/**
 * @brief Returns `true` if the values are within epsilon of one another
 */
PM_INLINE bool pm_equal(pm_flt c1, pm_flt c2)
{
    return pm_abs(c1 - c2) < PM_EPSILON;
}

/**
 * @brief Linearly interpolates the two values
 * @param a One endpoint
 * @param b Another endpoint
 * @param alpha A number in [0, 1] that specifies the position between the
 * endpoints
 */
PM_INLINE pm_flt pm_lerp(pm_flt a, pm_flt b, pm_flt alpha)
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
pm_flt pm_lerp_angle(pm_flt angle1, pm_flt angle2, pm_flt alpha);

/**
 * @brief Clamps the angle to be in [0, 2 * PI]
 */
PM_INLINE pm_flt pm_normalize_angle(pm_flt angle)
{
    while (angle >= PM_PI2)
        angle -= PM_PI2;

    while (angle < 0.0f)
        angle += PM_PI2;

    return angle;
}

/**
 * @brief Returns `true` if the value is a power of two
 */
PM_INLINE bool pm_is_pow2(uint32_t c)
{
    return (c & (c - 1)) == 0;
}

/**
 * @brief Returns the next power of two even if the argument is not a value of
 * two
 */
PM_INLINE uint32_t pm_next_pow2(uint32_t c)
{
    uint32_t n = (uint32_t)pm_ceil(pm_log2(c + 1));
    return (1UL << n);
}

/*==============================================================================
 * Vectors
 *============================================================================*/

/**
 * @brief Constructs a vector
 */
PM_INLINE pm_v2 pm_v2_make(pm_flt x, pm_flt y)
{
    pm_v2 out;
    out.x = x;
    out.y = y;
    return out;
}

/**
 * @brief Returns true if the vectors are equal (within epsilon)
 */
PM_INLINE bool pm_v2_equal(const pm_v2* v1, const pm_v2* v2)
{
    return pm_equal(v1->x, v2->x) &&
           pm_equal(v1->y, v2->y);
}

/**
 * @brief Adds two vectors
 * @param v1 First vector
 * @param v2 Second vector
 */
PM_INLINE pm_v2 pm_v2_add(pm_v2 v1, pm_v2 v2)
{
    return pm_v2_make(v1.x + v2.x, v1.y + v2.y);
}

/**
 * @brief Subtracts two vectors
 * @param v1 First vector
 * @param v2 Second vector
 */
PM_INLINE pm_v2 pm_v2_sub(pm_v2 v1, pm_v2 v2)
{
    return pm_v2_make(v1.x - v2.x, v1.y - v2.y);
}

/**
 * @brief Scales a vector
 * @param v Vector to scale
 * @param c The scale factor
 */
PM_INLINE pm_v2 pm_v2_scale(pm_v2 v, pm_flt c)
{
    return pm_v2_make(v.x * c, v.y * c);
}

/**
 * @brief Dot product
 */
PM_INLINE pm_flt pm_v2_dot(const pm_v2* v1, const pm_v2* v2)
{
    return v1->x * v2->x + v1->y * v2->y;
}

/**
 * @brief Returns the square of the length of the vector
 */
PM_INLINE pm_flt pm_v2_len2(const pm_v2* v)
{
    return pm_v2_dot(v, v);
}

/**
 * @brief Returns the length of the vector
 */
PM_INLINE pm_flt pm_v2_len(const pm_v2* v)
{
    return pm_sqrt(pm_v2_len2(v));
}

/**
 * @brief Normalizes a vector (sets its length to one)
 * @param v The vector to normalize
 * @returns The normalized vector
 */
PM_INLINE void pm_v2_normalize(pm_v2* v)
{
    pm_flt c = pm_v2_len(v);

    if (c < PM_EPSILON)
        *v = pm_v2_make(0.0f, 0.0f);
    else
        *v = pm_v2_scale(*v, 1.0f / c);
}

/**
 * @brief Construct a vector that is perpendicular to the specified vector
 * @param v The vector to be made perpendicular
 * @returns The perpendicular vector
 */
PM_INLINE pm_v2 pm_v2_perp(const pm_v2* v)
{
    return pm_v2_make(-v->y, v->x);
}

/**
 * @brief A 2D analog of the 3D cross product
 */
PM_INLINE pm_flt pm_v2_cross(const pm_v2* v1, const pm_v2* v2)
{
    pm_v2 perp = pm_v2_perp(v1);
    return pm_v2_dot(&perp, v2);
}

/**
 * @brief Returns the angle the vector with respect to the current basis
 */
PM_INLINE pm_flt pm_v2_angle(const pm_v2* v)
{
    return pm_atan2(v->y, v->x);
}

/**
 * @brief Projects a vector onto another
 * @param v1 The vector to be projected
 * @param v2 The vector to project onto
 * @returns The projection of v1 onto v2
 */
PM_INLINE pm_v2 pm_v2_proj(const pm_v2* v1, const pm_v2* v2)
{
    pm_flt d = pm_v2_dot(v1, v2) / pm_v2_dot(v2, v2);
    return pm_v2_scale(*v2, d);
}

/**
 * @brief Returns the distance between the two vectors
 */
PM_INLINE pm_flt pm_v2_dist(const pm_v2* v1, const pm_v2* v2)
{
    pm_v2 v = pm_v2_sub(*v1, *v2);
    return pm_v2_len(&v);
}

/**
 * @brief Linearly interpolates between two vectors
 * @poram v1 The first endpoint
 * @param v2 The second endpoint
 * @param alpha The normalized distance between v1 and v2
 */
PM_INLINE pm_v2 pm_v2_lerp(const pm_v2* v1, const pm_v2* v2, pm_flt alpha)
{
    pm_v2 out;
    out.x = pm_lerp(v1->x, v2->x, alpha);
    out.y = pm_lerp(v1->y, v2->y, alpha);
    return out;
}

/**
 * @brief Contructs a vector in polar coordinates
 */
PM_INLINE pm_v2 pm_v2_polar(pm_flt len, pm_flt angle)
{
    return pm_v2_make(len * pm_cos(angle), len * pm_sin(angle));
}

/*==============================================================================
 * 2D Affine Transforms
 *============================================================================*/

/**
 * @brief Constructs a 2D transform
 */
PM_INLINE pm_t2 pm_t2_make(pm_flt t00, pm_flt t01, pm_flt tx,
                           pm_flt t10, pm_flt t11, pm_flt ty)
{
    pm_t2 out;
    out.t00 = t00; out.t01 = t01; out.tx  = tx;
    out.t10 = t10; out.t11 = t11; out.ty  = ty;
    return out;
}

/**
 * @brief Return the identity transform
 */
PM_INLINE pm_t2 pm_t2_identity()
{
    return pm_t2_make(1.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f);
}

/**
 * @brief Returns true if the transforms are equal (within epsilon)
 */
bool pm_t2_equal(const pm_t2* t1, const pm_t2* t2);

/**
 * @brief Gets the translation components of the transform
 * @param t Pointer to the transform
 */
PM_INLINE pm_v2 pm_t2_get_pos(const pm_t2* t)
{
    return pm_v2_make(t->tx, t->ty);
}

/**
 * @brief Sets the translation components of the transform
 * @param t Pointer to the transform
 * @param pos The position vector
 */
PM_INLINE void pm_t2_set_pos(pm_t2* t, const pm_v2* pos)
{
    t->tx = pos->x;
    t->ty = pos->y;
}

/**
 * @brief Gets the angle of rotation of the transform
 */
PM_INLINE pm_flt pm_t2_get_angle(const pm_t2* t)
{
    return pm_normalize_angle(pm_atan2(t->t10, t->t11));
}

/**
 * @brief Sets the scale of the transform
 * @param t The transform
 * @param scale The vector containing scale factors in the x/y directions
 */
void pm_t2_set_scale(pm_t2* t, const pm_v2* scale);

/**
 * @brief Gets the scale of the transform
 * @param t The transform
 */
pm_v2 pm_t2_get_scale(const pm_t2* t);

/**
 * @brief Sets the angle of the transform
 */
void pm_t2_set_angle(pm_t2* t, pm_flt angle);

/**
 * @brief Transforms a vector
 * @param t The transform
 * @param v The vector to be transformed
 */
PM_INLINE pm_v2 pm_t2_map(const pm_t2* t, pm_v2 v)
{
    pm_v2 out;
    out.x = t->t00 * v.x + t->t01 * v.y + t->tx;
    out.y = t->t10 * v.x + t->t11 * v.y + t->ty;
    return out;
}

/**
 * @brief Returns the determinant of the transform
 */
PM_INLINE pm_flt pm_t2_det(const pm_t2* t)
{
    return t->t00 * t->t11 - t->t01 * t->t10;
}

/**
 * @brief Calculates the inverse of the transform
 * @param t The transform to invert
 */
pm_t2 pm_t2_inv(const pm_t2* t);

/**
 * @brief Composes two transformations
 */
pm_t2 pm_t2_mult(const pm_t2* t1, const pm_t2* t2);

/**
 * @brief Linearly interpolates two transforms
 */
pm_t2 pm_t2_lerp(const pm_t2* t1, const pm_t2* t2, pm_flt alpha);

/**
 * @brief Constructs a scaling transform
 * @param scale The scaling components
 */
PM_INLINE pm_t2 pm_t2_scaling(pm_v2 scale)
{
     return pm_t2_make(scale.x, 0.0f,    0.0f,
                       0.0f,    scale.y, 0.0f);
}

/**
 * @brief Constructs a rotation transform
 * @param angle The angle of rotation
 */
PM_INLINE pm_t2 pm_t2_rotation(pm_flt angle)
{
    pm_flt c = pm_cos(angle);
    pm_flt s = pm_sin(angle);

    return pm_t2_make(c, -s, 0.0f,
                      s,  c, 0.0f);
}

/**
 * @brief Constructs a translation transform
 * @param pos The translation coordinates
 */
PM_INLINE pm_t2 pm_t2_translation(pm_v2 pos)
{
    return pm_t2_make(1.0f, 0.0f, pos.x,
                      0.0f, 1.0f, pos.y);
}

/**
 * @brief Scales a transform
 * @param t The transform to scale
 * @param scale The scaling parameters
 */
PM_INLINE void pm_t2_scale(pm_t2* t, pm_v2 scale)
{
    pm_t2 scaling = pm_t2_scaling(scale);
    *t = pm_t2_mult(&scaling, t);
}

/**
 * @brief Applies a rotation to a transform
 * @param t The transform to rotate
 * @param angle The angle to rotate by
 */
PM_INLINE void pm_t2_rotate(pm_t2* t, pm_flt angle)
{
    pm_t2 rotation = pm_t2_rotation(angle);
    *t = pm_t2_mult(&rotation, t);
}

/**
 * @brief Applies a translation a transform
 * @param t The transform to translate
 * @param pos The translation components
 */
PM_INLINE void pm_t2_translate(pm_t2* t, pm_v2 pos)
{
    pm_t2 translation = pm_t2_translation(pos);
    *t = pm_t2_mult(&translation, t);
}

/*==============================================================================
 * 2D Box (Rectangle)
 *============================================================================*/

/**
 * @brief Constructs a 2D box (rectangle)
 */
PM_INLINE pm_b2 pm_b2_make(pm_flt x, pm_flt y, pm_flt w, pm_flt h)
{
    pm_b2 out;
    out.x = x; out.y = y;
    out.w = w; out.h = h;
    return out;
}

/**
 * @brief Returns `true` if the bounding boxes are equal (within epsilon)
 */
bool pm_b2_equal(const pm_b2* b1, const pm_b2* b2);

/**
 * @brief Computes the union of `b1` and `b2
 */
pm_b2 pm_b2_union(const pm_b2* b1, const pm_b2* b2);

/**
 * @brief Return `true` if the two bounding boxes intersect
 */
PM_INLINE bool pm_b2_intersects(const pm_b2* b1, const pm_b2* b2)
{
    return b1->x + b1->w >= b2->x && // x + w
           b1->y + b1->h >= b2->y && // y + h
           b2->x + b2->w >= b1->x && // x + w
           b2->y + b2->h >= b2->y;   // y + h
}

/**
 * @brief Computes the intersection of `b1` and `b2`
 */
pm_b2 pm_b2_intersection(const pm_b2* b1, const pm_b2* b2);

/**
 * @brief Returns `true` if the box contains the point `v`
 */
PM_INLINE bool pm_b2_contains(const pm_b2* b, pm_v2 v)
{
    pm_flt x = v.x;
    pm_flt y = v.y;

    return x > b->x && y > b->y && x < b->x + b->w && y < b->y + b->h;
}

/**
 * @brief Returns the area of the box
 */
PM_INLINE pm_flt pm_b2_area(const pm_b2* b)
{
    return b->w * b->h;
}

/**
 * @brief Computes the center of the box
 */
PM_INLINE pm_v2 pm_b2_center(const pm_b2* b)
{
    return pm_v2_make(b->x + b->w / 2.0f, b->y + b->h / 2.0f);
}

/**
 * @brief Computes the minimum box containing all of the vertices
 * @param vertices The vertices
 * @param count The number of vertices
 */
pm_b2 pm_b2_min(const pm_v2* vertices, int count);

#define PM_STATE_VECTOR_LEN 624

/**
 * @brief The pseudo random number generator (RNG) state
 */
typedef struct
{
  uint32_t mt[PM_STATE_VECTOR_LEN];
  int index;
} pm_rng_t;

/**
 * @brief Initialize and seed the RNG
 * @param rng A reference to the RNG
 * @param seed The seed (choosing the same seed will yield identical sequences)
 */
void pm_rng_seed(pm_rng_t* rng, uint32_t seed);

/**
 * @brief Generates a pseudo random number in [0, UINT32_MAX]
 * @param rng A reference to the RNG
 */
uint32_t pm_random(pm_rng_t* rng);

/**
 * @brief Generates a psuedo random number in [0, 1]
 */
pm_flt pm_random_float(pm_rng_t* rng);

#ifdef __cplusplus
}
#endif

#endif // PICO_ML_H

#ifdef PM_IMPLEMENTATION


pm_flt pm_lerp_angle(pm_flt angle1, pm_flt angle2, pm_flt alpha)
{
    const pm_v2 v1 = pm_v2_make(pm_cos(angle1), pm_sin(angle1));
    const pm_v2 v2 = pm_v2_make(pm_cos(angle2), pm_sin(angle2));

    // Calculuate cosine of angle between the two vectors
    pm_flt dot = pm_clamp(pm_v2_dot(&v1, &v2), -1.0f, 1.0f);

    // LERP if the cosine is too close to its limits
    if (pm_equal(dot, 1.0f) || pm_equal(dot, -1.0f))
    {
        pm_v2 tmp = pm_v2_lerp(&v1, &v2, alpha);
        return pm_normalize_angle(pm_atan2(tmp.y, tmp.x));
    }

    // Calculate angle
    pm_flt angle = pm_acos(dot) * alpha;

    // Gram-Schmidt(construct a new vector 'v0' that is orthogonal to 'v1')
    pm_v2 v0  = pm_v2_sub(v2, pm_v2_scale(v1, dot));
    pm_v2_normalize(&v0);

    // Calcuate vector in new coordinate system
    pm_v2 tmp1 = pm_v2_scale(v1, pm_cos(angle));
    pm_v2 tmp2 = pm_v2_scale(v0, pm_sin(angle));

    pm_v2 tmp = pm_v2_add(tmp1, tmp2);

    // Calculate new angle
    return pm_normalize_angle(pm_atan2(tmp.y, tmp.x));
}

bool pm_t2_equal(const pm_t2* t1, const pm_t2* t2)
{
    return pm_equal(t1->t00, t2->t00) &&
           pm_equal(t1->t10, t2->t10) &&
           pm_equal(t1->t01, t2->t01) &&
           pm_equal(t1->t11, t2->t11) &&
           pm_equal(t1->tx,  t2->tx)  &&
           pm_equal(t1->ty,  t2->ty);
}

void pm_t2_set_scale(pm_t2* t, const pm_v2* scale)
{
    pm_flt angle = pm_t2_get_angle(t);

    pm_flt c = pm_cos(angle);
    pm_flt s = pm_sin(angle);

    pm_flt sx = scale->x;
    pm_flt sy = scale->y;

    t->t00 = sx * c; t->t01 = sx * -s;
    t->t10 = sy * s; t->t11 = sy *  c;
}

pm_v2 pm_t2_get_scale(const pm_t2* t)
{
    pm_flt angle = pm_t2_get_angle(t);
    pm_flt cos_sign = pm_sign(pm_cos(angle));

    pm_v2 out;

    if (0.0f == cos_sign) //TODO: pm_equal?
    {
        out.x = -t->t01;
        out.y =  t->t10;
        return out;
    }

    pm_v2 v1 = pm_v2_make(t->t00, t->t01);
    pm_v2 v2 = pm_v2_make(t->t10, t->t11);

    out.x = pm_sign(t->t00) * cos_sign * pm_v2_len(&v1);
    out.y = pm_sign(t->t11) * cos_sign * pm_v2_len(&v2);

    return out;
}

void pm_t2_set_angle(pm_t2* t, pm_flt angle)
{
    pm_flt c = pm_cos(angle);
    pm_flt s = pm_sin(angle);

    pm_v2 scale = pm_t2_get_scale(t);

    pm_flt sx = scale.x;
    pm_flt sy = scale.y;

    t->t00 = sx * c; t->t01 = sx * -s;
    t->t10 = sy * s; t->t11 = sy *  c;
}

pm_t2 pm_t2_inv(const pm_t2* t)
{
    pm_flt det = pm_t2_det(t);

    if (0.0f == det) // Intentionally not using epsilon because determinants
    {                // can be really small and still be valid
        return pm_t2_identity();
    }

    pm_flt inv_det = 1.0f / det;

    pm_t2 out;

    out.t00 =  t->t11 * inv_det; out.t01 = -t->t01 * inv_det;
    out.t10 = -t->t10 * inv_det; out.t11 =  t->t00 * inv_det;

    out.tx = (t->t01 * t->ty - t->t11 * t->tx) * inv_det;
    out.ty = (t->t10 * t->tx - t->t00 * t->ty) * inv_det;

    return out;
}

pm_t2 pm_t2_mult(const pm_t2* t1, const pm_t2* t2)
{
    pm_t2 out;

    out.t00 = t1->t00 * t2->t00 + t1->t01 * t2->t10;
    out.t10 = t1->t10 * t2->t00 + t1->t11 * t2->t10;

    out.t01 = t1->t00 * t2->t01 + t1->t01 * t2->t11;
    out.t11 = t1->t10 * t2->t01 + t1->t11 * t2->t11;

    out.tx = t1->t00 * t2->tx + t1->t01 * t2->ty + t1->tx;
    out.ty = t1->t10 * t2->tx + t1->t11 * t2->ty + t1->ty;

    return out;
}

pm_t2 pm_t2_lerp(const pm_t2* t1, const pm_t2* t2, pm_flt alpha)
{
    pm_v2 scale1 = pm_t2_get_scale(t1);
    pm_v2 scale2 = pm_t2_get_scale(t2);

    pm_flt angle1 = pm_t2_get_angle(t1);
    pm_flt angle2 = pm_t2_get_angle(t2);

    pm_v2 pos1 = pm_t2_get_pos(t1);
    pm_v2 pos2 = pm_t2_get_pos(t2);

    pm_v2 scale = pm_v2_lerp(&scale1, &scale2, alpha);
    pm_v2 pos = pm_v2_lerp(&pos1, &pos2, alpha);
    pm_flt angle = pm_lerp_angle(angle1, angle2, alpha);

    pm_flt c = pm_cos(angle);
    pm_flt s = pm_sin(angle);

    pm_flt sx = scale.x;
    pm_flt sy = scale.y;

    pm_flt tx = pos.x;
    pm_flt ty = pos.y;

    pm_t2 out;

    out.t00 = sx * c; out.t01 = sx * -s; out.tx = tx;
    out.t10 = sy * s; out.t11 = sy *  c; out.ty = ty;

    return out;
}

bool pm_b2_equal(const pm_b2* b1, const pm_b2* b2)
{
    return pm_equal(b1->x, b2->x) &&
           pm_equal(b1->y, b2->y) &&
           pm_equal(b1->w, b2->w) &&
           pm_equal(b1->h, b2->h);
}

pm_b2 pm_b2_union(const pm_b2* b1, const pm_b2* b2)
{
    pm_flt x1 = pm_min(b1->x,  b2->x); // x
    pm_flt y1 = pm_min(b1->y,  b2->y); // y
    pm_flt x2 = pm_max(b1->x + b1->w, b2->x + b2->w); // x + w
    pm_flt y2 = pm_max(b1->y + b1->h, b2->y + b2->h); // y + h

    pm_b2 out;
    out.x = x1;
    out.y = y1;
    out.w = x2 - x1;
    out.h = y2 - y1;
    return out;
}

pm_b2 pm_b2_intersection(const pm_b2* b1, const pm_b2* b2)
{
    if (!pm_b2_intersects(b1, b2))
        return pm_b2_make(0.0f, 0.0f, 0.0f, 0.0f);

    pm_flt x1 = pm_max(b1->x,  b2->x); // x
    pm_flt y1 = pm_max(b1->y,  b2->y); // y
    pm_flt x2 = pm_min(b1->x + b1->w, b2->x + b2->w); // x + w
    pm_flt y2 = pm_min(b1->y + b1->h, b2->y + b2->h); // y + h

    pm_b2 out;
    out.x = x1;
    out.y = y1;
    out.w = x2 - x1;
    out.h = y2 - y1;
    return out;
}

// TODO: optimize this function (pm_v2_min/pm_2_max)
pm_b2 pm_b2_min(const pm_v2* vertices, int count)
{
    if (0 == count)
        return pm_b2_make(0.0f, 0.0f, 0.0f, 0.0f);

    pm_flt min_x = vertices[0].x;
    pm_flt min_y = vertices[0].y;
    pm_flt max_x = vertices[0].x;
    pm_flt max_y = vertices[0].y;

    for (int i = 1; i < count; i++)
    {
        if (vertices[i].x < min_x)
            min_x = vertices[i].x;

        if (vertices[i].y < min_y)
            min_y = vertices[i].y;

        if (vertices[i].x > max_x)
            max_x = vertices[i].x;

        if (vertices[i].y > max_y)
            max_y = vertices[i].y;
    }

    pm_b2 out;
    out.x = min_x;
    out.y = min_y;
    out.w = pm_abs(max_x - min_x);
    out.h = pm_abs(max_y - min_y);
    return out;
}


/*
 * Mersenne Twister random number generator. Based on
 * https://github.com/ESultanik/mtwister by Evan Sultanik (Public Domain).
 */

#define PM_STATE_VECTOR_LEN 624
#define PM_STATE_VECTOR_M   397
#define PM_UPPER_MASK       0x80000000
#define PM_LOWER_MASK       0x7fffffff
#define PM_TEMPERING_MASK_B 0x9d2c5680
#define PM_TEMPERING_MASK_C 0xefc60000

void pm_rng_seed(pm_rng_t* rng, uint32_t seed)
{
    rng->mt[0] = seed & 0xffffffff;

    for (rng->index = 1; rng->index < PM_STATE_VECTOR_LEN; rng->index++)
    {
        rng->mt[rng->index] = (69069 * rng->mt[rng->index - 1]) & 0xffffffff;
    }
}

uint32_t pm_random(pm_rng_t* rng)
{
    uint32_t y;
    static const uint32_t mag[2] = {0x0, 0x9908b0df};

    if (rng->index >= PM_STATE_VECTOR_LEN || rng->index < 0)
    {
        int kk;

        if (rng->index >= PM_STATE_VECTOR_LEN + 1 || rng->index < 0)
        {
            pm_rng_seed(rng, 4357);
        }

        for (kk = 0; kk < PM_STATE_VECTOR_LEN - PM_STATE_VECTOR_M; kk++)
        {
            y = (rng->mt[kk] & PM_UPPER_MASK) | (rng->mt[kk + 1] & PM_LOWER_MASK);
            rng->mt[kk] = rng->mt[kk + PM_STATE_VECTOR_M] ^ (y >> 1) ^ mag[y & 0x1];
        }

        for(; kk < PM_STATE_VECTOR_LEN - 1; kk++)
        {
            y = (rng->mt[kk] & PM_UPPER_MASK) | (rng->mt[kk + 1] & PM_LOWER_MASK);
            rng->mt[kk] = rng->mt[kk + (PM_STATE_VECTOR_M - PM_STATE_VECTOR_LEN)] ^ (y >> 1) ^ mag[y & 0x1];
        }

        y = (rng->mt[PM_STATE_VECTOR_LEN - 1] & PM_UPPER_MASK) | (rng->mt[0] & PM_LOWER_MASK);

        rng->mt[PM_STATE_VECTOR_LEN - 1] = rng->mt[PM_STATE_VECTOR_M - 1] ^ (y >> 1) ^ mag[y & 0x1];
        rng->index = 0;
    }

    y = rng->mt[rng->index++];
    y ^= (y >> 11);
    y ^= (y << 7)  & PM_TEMPERING_MASK_B;
    y ^= (y << 15) & PM_TEMPERING_MASK_C;
    y ^= (y >> 18);

    return y;
}

pm_flt pm_random_float(pm_rng_t* rng)
{
    return (pm_flt)pm_random(rng) / (pm_flt)UINT32_MAX;
}


#endif // PM_IMPLEMENTATION

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

