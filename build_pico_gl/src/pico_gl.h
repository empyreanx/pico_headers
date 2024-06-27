/**
    @file pico_gl.h
    @brief A powerful OpenGL-based graphics library written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in C99
    - Single header library for easy build system integration
    - Embeds GLAD for seamless OpenGL function loading
    - Simple texture and shader creation
    - Default shader and uniforms
    - Rendering of dynamic vertex arrays and static vertex buffers
    - Render to texture
    - Simplified shader uniform setters
    - State stack
    - Simple and concise API
    - Permissive license (zlib or public domain)

    Summary:
    --------

    This library is an advanced 2D renderer built on top of OpenGL. It currently
    supports OpenGL 3.0+ and OpenGL ES 3+ as well.

    The basic workflow is to initialize the library, create a context, load any
    shaders and/or textures needed, specify some geometry (vertices) and draw
    the buffer or array of vertices. A vertex consists of position, color, and
    uv coordinates.

    There is a default shader that makes sensible assumptions about how vertices
    and textures will be used. It is likely to be sufficient unless performing
    advanced VFX.

    The default shader exposes two uniform variables: 1) a projection matrix;
    and 2) an affine transformation matrix. These two matrices are multipled
    together to form the mapping from vertices to the screen.

    State including blend mode, the projection and transform matrices, and
    the viewport parameters are managed using the state stack. This enables
    state changes to be isolated. Push the current state on the top of the
    stack, make some local changes, and pop the stack to restore the original
    state.

    Uniforms can be set using a simple, fast, and concise API.

    Please see the examples for more details.

    To use this library in your project, add

    > #define PICO_GL_IMPLEMENTATION
    > #include "pico_gl.h"

    to a source file.

    Constants:
    --------

    - PICO_GL_UNIFORM_NAME_LENGTH (default: 32)
    - PICO_GL_MAX_UNIFORMS (default: 32)
    - PICO_GL_MAX_STATES (default: 32)

    Must be defined before PICO_GL_IMPLEMENTATION

    Todo:
    -----
    - MSAA flag for examples
    - Better version handling
    - Shader examples
    - Scissor
    - Indexed buffers
    - Multiple texture units
*/
#ifndef PICO_GL_H
#define PICO_GL_H

#include <stdbool.h> // bool, true, false
#include <stdarg.h>  // va_list, va_start, va_end
#include <stddef.h>  // size_t, NULL
#include <stdint.h>  // uint32_t, int32_t, uint64_t
#include <stdio.h>   // printf, vprintf

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OpenGL compatible size type
 */
typedef uint32_t pgl_size_t;

/**
 * @brief Runtime error codes
 */
typedef enum
{
    PGL_NO_ERROR,                      //!< No error
    PGL_INVALID_ENUM,                  //!< Invalid enumeration value
    PGL_INVALID_VALUE,                 //!< Invalid value
    PGL_INVALID_OPERATION,             //!< Invalid operation
    PGL_OUT_OF_MEMORY,                 //!< Out of memory
    PGL_INVALID_FRAMEBUFFER_OPERATION, //!< Invalid framebuffer operation
    PGL_FRAMEBUFFER_INCOMPLETE,        //!< Framebuffer is incomplete
    PGL_SHADER_COMPILATION_ERROR,      //!< Shader compilation error
    PGL_SHADER_LINKING_ERROR,          //!< Shader linking error
    PGL_INVALID_TEXTURE_SIZE,          //!< Invalid texture dimensions
    PGL_INVALID_TEXTURE_FORMAT,        //!< Invalid texture format
    PGL_INVALID_ATTRIBUTE_COUNT,       //!< Invalid number of attributes
    PGL_INVALID_UNIFORM_COUNT,         //!< Invalid number of uniforms
    PGL_INVALID_UNIFORM_NAME,          //!< Invalid uniform name
    PGL_UNKNOWN_ERROR,                 //!< Unknown error
    PGL_ERROR_COUNT
} pgl_error_t;

/**
 * @brief OpenGL versions used by the library
 */
typedef enum
{
    PGL_GL3,
    PGL_GLES3,
    PGL_VERSION_UNSUPPORTED
} pgl_version_t;

/**
 * @brief Pixel formats
 */
typedef enum
{
    PGL_RED,         //!< (red, 0, 0, 1)
    PGL_RGB,         //!< (red, green, blue, 1)
    PGL_RGBA,        //!< (red, green, blue, alpha)
    PGL_BGR,         //!< (blue, green, red, 1)
    PGL_BGRA,        //!< (blue, green, red, alpha)
    PGL_FORMAT_COUNT
} pgl_format_t;

/**
 * @brief Blend factors
 */
typedef enum
{
    PGL_ZERO,                //!< (0, 0, 0, 0)
    PGL_ONE,                 //!< (1, 1, 1, 1)
    PGL_SRC_COLOR,           //!< (src.r, src.g, src.b, src.a)
    PGL_ONE_MINUS_SRC_COLOR, //!< (1, 1, 1, 1) - (src.r, src.g, src.b, src.a)
    PGL_DST_COLOR,           //!< (dst.r, dst.g, dst.b, dst.a)
    PGL_ONE_MINUS_DST_COLOR, //!< (1, 1, 1, 1) - (dst.r, dst.g, dst.b, dst.a)
    PGL_SRC_ALPHA,           //!< (src.a, src.a, src.a, src.a)
    PGL_ONE_MINUS_SRC_ALPHA, //!< (1, 1, 1, 1) - (src.a, src.a, src.a, src.a)
    PGL_DST_ALPHA,           //!< (dst.a, dst.a, dst.a, dst.a)
    PGL_ONE_MINUS_DST_ALPHA, //!< (1, 1, 1, 1) - (dst.a, dst.a, dst.a, dst.a)
    PGL_FACTOR_COUNT
} pgl_blend_factor_t;

/**
 * @brief Blend equations
 */
typedef enum
{
    PGL_FUNC_ADD,              //!< result = src * src_factor + dst * dst_factor
    PGL_FUNC_SUBTRACT,         //!< result = src * src_factor - dst * dst_factor
    PGL_FUNC_REVERSE_SUBTRACT, //!< result = dst * dst_factor - src * src_factor
    PGL_MIN,                   //!< result = min(src, dst)
    PGL_MAX,                   //!< result = max(src, dst)
    PGL_EQ_COUNT
} pgl_blend_eq_t;

/**
 * @brief Blend mode
 *
 * Completely describes a blend operation.
 */
typedef struct
{
    pgl_blend_factor_t color_src; //!< Color source blending factor
    pgl_blend_factor_t color_dst; //!< Color dsestination blending factor
    pgl_blend_eq_t     color_eq;  //!< Equation for blending colors
    pgl_blend_factor_t alpha_src; //!< Alpha source blending factor
    pgl_blend_factor_t alpha_dst; //!< Alpha destination blending factor
    pgl_blend_eq_t     alpha_eq;  //!< Equation for blending alpha values
} pgl_blend_mode_t;

/**
 * @brief Drawing primitives
 */
typedef enum
{
    PGL_POINTS,         //!< Array of points
    PGL_LINES,          //!< Each adjacent pair of points forms a line
    PGL_LINE_STRIP,     //!< Array of points where every pair forms a lines
    PGL_TRIANGLES,      //!< Each adjacent triple forms an individual triangle
    PGL_TRIANGLE_STRIP, //!< Array of points where every triple forms a triangle
} pgl_primitive_t;

/**
 * @brief A vertex describes a point and the data associated with it (color and
 * texture coordinates)
 */
typedef struct
{
    float pos[3];
    float color[4];
    float uv[2];
} pgl_vertex_t;

/**
 * @brief 2D floating point vector
 */
typedef float pgl_v2f_t[2];

/**
 * @brief 3D floating point vector
 */
typedef float pgl_v3f_t[3];

/**
 * @brief 4D floating point vector
 */
typedef float pgl_v4f_t[4];

/**
 * @brief 2D integer vector
 */
typedef int32_t pgl_v2i_t[2];

/**
 * @brief 3D integer vector
 */
typedef int32_t pgl_v3i_t[3];

/**
 * @brief 4D integer vector
 */
typedef int32_t pgl_v4i_t[4];

/**
 * @brief 2x2 floating point matrix
 */
typedef float pgl_m2_t[4];

/**
 * @brief 3x3 floating point matrix
 */
typedef float pgl_m3_t[9];

/**
 * @brief 4x4 floating point matrix
 */
typedef float pgl_m4_t[16];

/**
 * @brief Contains core data/state for an instance of the renderer
 */
typedef struct pgl_ctx_t pgl_ctx_t;

/**
 * @brief Contains shader data/state
 */
typedef struct pgl_shader_t pgl_shader_t;

/**
 * @brief Contains texture data/state
 */
typedef struct pgl_texture_t pgl_texture_t;

/**
 * @brief Contains vertex buffer data/state
 */
typedef struct pgl_buffer_t pgl_buffer_t;

/**
 * @brief Defines an OpenGL (GLAD) function loader
 *
 * @param name The name of the function to load
 */
typedef void* (*pgl_loader_fn)(const char* name);

/**
 * @brief Loads all supported OpenGL functions via GLAD
 *
 * IMPORTANT: A valid OpenGL context must exist for this function to succeed.
 * This function must be called before any other PGL functions.
 *
 * @param loader_fp Function loader (can be NULL except for GLES contexts)
 * @param gles      Set to \em true if using OpenGL ES
 *
 * @returns -1 on failure and 0 otherwise
 */
int pgl_global_init(pgl_loader_fn loader_fp, bool gles);

/**
 * @brief Returns the current error code
 */
pgl_error_t pgl_get_error(pgl_ctx_t* ctx);

/**
 * @brief Returns the string associated with the specified error code
 *
 * @param code The error code to query
 *
 * @returns The string representation of the error code
 */
const char* pgl_get_error_str(pgl_error_t code);

/**
 * @brief Returns the current OpenGL version in use by the library.
 *
 * Currently the library does not use OpenGL features outside of 3.0 and ES 3.0.
 * This unlikely to change unless there is an incompatibility with more recent
 * versions or the library is ported to OpenGL 2.1.
 *
 * @returns PGL_VERSION_UNSUPPORTED if the version of OpenGL is not supported
 */
pgl_version_t pgl_get_version();

/**
 * @brief Prints system info
 */
void pgl_print_info();

/**
 * @brief Creates an instance of the renderer
 *
 * @param w       The drawable width of the window in pixels
 * @param h       The drawable height of the window in pixels
 * @param depth   Depth test is enabled if true
 * @param samples The number of MSAA (anti-alising) samples (disabled if 0)
 * @param srgb    Enables support for the sRGB colorspace
 * @param mem_ctx User data provided to custom allocators
 *
 * @returns The context pointer or \em NULL on error
 */
pgl_ctx_t* pgl_create_context(uint32_t w, uint32_t h, bool depth,
                              uint32_t samples, bool srgb,
                              void* mem_ctx);

/**
 * @brief Destroys a renderer context, releasing it's resources
 *
 * @param ctx A pointer to the context
 */
void pgl_destroy_context(pgl_ctx_t* ctx);

/**
 * @brief Resizes the drawable dimensions
 *
 * @param ctx       A pointer to the context
 * @param w         The drawable width of the window in pixels
 * @param h         The drawable height of the window in pixels
 * @pawram reset_vp Resets the viewport if true
 */
void pgl_resize(pgl_ctx_t* ctx, uint32_t w, uint32_t h, bool reset_vp);

/**
 * @brief Creates a shader program
 *
 * If `vert_src` and `frag_src` are both `NULL`, then the shader is compiled
 * from the default vertex and fragment sources. If either `vert_src` or
 * `frag_src` are `NULL`, then the shader is compiled from the (respective)
 * default vertex or fragment source, together with the non-`NULL` argument.
 *
 * @param ctx      The relevant context
 * @param vert_src Vertex shader source
 * @param frag_src Fragment shader source

 * @returns Pointer to the shader program, or \em NULL on error
 */
pgl_shader_t* pgl_create_shader(pgl_ctx_t* ctx, const char* vert_src,
                                                const char* frag_src);

/**
 * @brief Destroys a shader program
 *
 * @param shader Pointer to the shader program
 */
void pgl_destroy_shader(pgl_shader_t* shader);

/**
 * @brief Activates a shader program for rendering
 *
 * This function sets the context's currently active shader. If `shader` is
 * `NULL` the current shader is deactivated.
 *
 * @param ctx    The relevant context
 * @param shader The shader program to activate, or `NULL` to deactivate
 */
void pgl_bind_shader(pgl_ctx_t* ctx, pgl_shader_t* shader);

/**
 * @brief Return the implementation specific shader ID
 *
 * @param shader The target shader
 *
 * @returns An unsigned 64-bit ID value
 */
uint64_t pgl_get_shader_id(const pgl_shader_t* shader);

/**
 * @brief Creates an empty texture
 *
 * @param ctx    The relevant context
 * @param target If true, the texture can be used as a render target
 * @param w      The texture's width
 * @param h      The texture's height
 * @param srgb   True if the internal format is sRGB
 * @param smooth High (true) or low (false) quality filtering
 * @param repeat Repeats or clamps when uv coordinates exceed 1.0
 */
pgl_texture_t* pgl_create_texture(pgl_ctx_t* ctx,
                                  bool target,
                                  pgl_format_t fmt, bool srgb,
                                  int32_t w, int32_t h,
                                  bool smooth, bool repeat);

/**
 * @brief Creates a texture from a bitmap
 *
 * @param ctx    The relevant context
 * @param w      The texture's width
 * @param h      The texture's height
 * @param srgb   True if the internal format is sRGB
 * @param smooth High (true) or low (false) quality filtering
 * @param repeat Repeats or clamps when uv coordinates exceed 1.0
 * @param bitmap Pixel data in RGBA format
 */
pgl_texture_t* pgl_texture_from_bitmap(pgl_ctx_t* ctx,
                                       pgl_format_t fmt, bool srgb,
                                       int32_t w, int32_t h,
                                       bool smooth, bool repeat,
                                       const uint8_t* bitmap);

/**
 * @brief Uploads data from a bitmap into a texture
 *
 * @param ctx     The relevant context
 * @param texture The target texture
 * @param w       The texture's width
 * @param h       The texture's height
 * @param bitmap  The pixel data in RGBA
 */
int pgl_upload_texture(pgl_ctx_t* ctx,
                       pgl_texture_t* texture,
                       int32_t w, int32_t h,
                       const uint8_t* bitmap);

/**
 * @brief Updates a region of an existing texture
 *
 * @param ctx     The relevant context
 * @param texture The texture to update
 * @param x       The x offset of the region
 * @param y       The y offset of the region
 * @param w       The width of the region
 * @param h       The height of the region
 * @param bitmap  The pixel data in RGBA
 */
void pgl_update_texture(pgl_ctx_t* ctx,
                        pgl_texture_t* texture,
                        int x, int y,
                        int w, int h,
                        const uint8_t* bitmap);

/**
 * @brief Generate mipmaps for the specified texture
 *
 * Generates a sequence of smaller pre-filtered / pre-optimized textures
 * intended to reduce the effects of aliasing when rendering smaller versions
 * of the texture.
 *
 * @param texture Pointer to the target texture
 * @param linear  Determines the selection of the which mipmap to blend
 *
 * @returns 0 on success and -1 on failure
 */
int pgl_generate_mipmap(pgl_texture_t* texture, bool linear);

/**
 * @brief Destroys a texture
 *
 * @param texture Texture to destroy
 */
void pgl_destroy_texture(pgl_texture_t* texture);

/**
 * @brief Gets texture size
 *
 * @param texture The texture
 * @param w       Pointer to width (output)
 * @param h       Pointer to height (output)
 */
void pgl_get_texture_size(const pgl_texture_t* texture, int* w, int* h);

/**
 * Gets maximum texture size as reported by OpenGL
 *
 * @param w Pointer to width (output)
 * @param h Poineter to height (output)
 */
void pgl_get_max_texture_size(int* w, int* h);

/**
 * @brief Return the implementation specific texture ID
 *
 * @param texture The target texture
 *
 * @returns An unsigned 64-bit ID value
 */
uint64_t pgl_get_texture_id(const pgl_texture_t* texture);

/**
 * @brief Activates a texture for rendering
 *
 * This function sets the context's currently texture. If `texture` is
 * `NULL` the current texture is deactivated.
 *
 * @param ctx     The relevant context
 * @param texture The texture to activate, or `NULL` to deactivate
 */
void pgl_bind_texture(pgl_ctx_t* ctx, pgl_texture_t* texture);

/**
 * @brief Draw to texture
 *
 * The function set a texture to be the target for rendering until the texture
 * if replaced by another or set to `NULL`.
 *
 * @param ctx     The relevant context
 * @param texture The render target
 *
 * @returns 0 on success and -1 on failure
 */
int pgl_set_render_target(pgl_ctx_t* ctx, pgl_texture_t* texture);

/**
 * @brief Clears the framebuffer to the specified color
 *
 * All of the parameters are in `[0.0, 1.0]`
 */
void pgl_clear(float r, float g, float b, float a);

/**
 * Draws primitives according to a vertex array
 *
 * @param ctx       The relevant context
 * @param primitive The type of geometry to draw
 * @param vertices  A vertex array
 * @param count     The number of vertices
 * @param texture   The texture to draw from (can be `NULL`)
 * @param shader    The shader used to draw the array (cannot be `NULL`)
 */
void pgl_draw_array(pgl_ctx_t* ctx,
                    pgl_primitive_t primitive,
                    const pgl_vertex_t* vertices,
                    pgl_size_t count,
                    pgl_texture_t* texture,
                    pgl_shader_t* shader);

/**
 * Draws primvities according to vertex and index arrays
 *
 * @param ctx          The relevant context
 * @param primitive    The type of geometry to draw
 * @param vertices     An array of vertices
 * @param vertex_count The number of vertices
 * @param indices      An array of indices
 * @param index_ count The number of indicies
 * @param texture      The texture to draw from (can be `NULL`)
 * @param shader       The shader used to draw the array (cannot be `NULL`)
 */
void pgl_draw_indexed_array(pgl_ctx_t* ctx,
                            pgl_primitive_t primitive,
                            const pgl_vertex_t* vertices, pgl_size_t vertex_count,
                            const uint32_t* indices, pgl_size_t index_count,
                            pgl_texture_t* texture,
                            pgl_shader_t* shader);

/**
 * @brief Creates a buffer in VRAM to store an array of vertices that can then
 * be rendered without having upload the vertices every time they are drawn
 *
 * @param ctx       The relevant context
 * @param primitive The type of geometry to draw
 * @param vertices  A vertex array
 * @param count     The number of vertices to store in the buffer
 *
 * @returns A pointer to the buffer or `NULL` on error
 */
pgl_buffer_t* pgl_create_buffer(pgl_ctx_t* ctx,
                                pgl_primitive_t primitive,
                                const pgl_vertex_t* vertices,
                                pgl_size_t count);

/**
 * @brief Substitutes the data in a buffer with new data
 *
 * @param ctx      The relevant context
 * @param buffer   The buffer to write to
 * @param vertices A vertex array
 * @param count    The number of vertices to substitute.
 */
void pgl_sub_buffer_data(pgl_ctx_t *ctx,
                         pgl_buffer_t* buffer,
                         const pgl_vertex_t* vertices,
                         pgl_size_t count,
                         pgl_size_t offset);

/**
 * @brief Destroys a previously created buffer
 */
void pgl_destroy_buffer(pgl_buffer_t* buffer);

/**
 * @brief Draw a previously created buffer
 *
 * @param ctx     The relevant context
 * @param buffer  The buffer to draw
 * @param start   The base vertex index
 * @param count   The number of vertices to draw from `start`
 * @param texture The texture to draw from (can be `NULL`)
 * @param shader  The shader used to draw the array (cannot be `NULL`)
 */
void pgl_draw_buffer(pgl_ctx_t* ctx,
                     const pgl_buffer_t* buffer,
                     pgl_size_t start, pgl_size_t count,
                     pgl_texture_t* texture,
                     pgl_shader_t* shader);

/**
 * @brief Turns matrix transposition on/off
 */
void pgl_set_transpose(pgl_ctx_t* ctx, bool enabled);

/**
 * @brief Set the blending mode
 *
 * @param ctx  The relevant context
 * @param mode The blending mode (@see pgl_blend_mode_t)
 */
void pgl_set_blend_mode(pgl_ctx_t* ctx, pgl_blend_mode_t mode);

/**
 * @brief Resets the blend mode to standard alpha blending
 *
 * @param ctx The relevant context
 */
void pgl_reset_blend_mode(pgl_ctx_t* ctx);

/**
 * @brief Sets the context's global tranformation matrix
 *
 * @param ctx    The relevant context
 * @param matrix The global transform matrix
 */
void pgl_set_transform(pgl_ctx_t* ctx, const pgl_m4_t matrix);

/**
 * @brief 3D variant of `pgl_set_transform`
 *
 * @param ctx    The relevant context
 * @param matrix The 3D global transform matrix
 */
void pgl_set_transform_3d(pgl_ctx_t* ctx, const pgl_m3_t matrix);

/**
 * @brief Resets the context's transform to the identity matrix
 *
 * @param ctx The relevant context
 */
void pgl_reset_transform(pgl_ctx_t* ctx);

/**
 * @brief Sets a context's global projecton matrix
 *
 * @param ctx    The relevant context
 * @param matrix The global projection matrix
 */
void pgl_set_projection(pgl_ctx_t* ctx, const pgl_m4_t matrix);

/**
 * @brief 3D variant of `pgl_set_projection`
 *
 * @param ctx    The relevant context
 * @param matrix The 3D global projection matrix
 */
void pgl_set_projection_3d(pgl_ctx_t* ctx, const pgl_m3_t matrix);

/**
 * @brief Resets the context's project to the identity matrix
 *
 * @param ctx The relevant context
 */
void pgl_reset_projection(pgl_ctx_t* ctx);

/**
 * @brief Sets the location and dimensions of the rendering viewport
 *
 * @param ctx The relevant context
 * @param x   The left edge of the viewport
 * @param y   The bottom edge of the viewport
 * @param w   The width of the viewport
 * @param h   The height of the viewort
 */
void pgl_set_viewport(pgl_ctx_t* ctx, int32_t x, int32_t y,
                                      int32_t w, int32_t h);

/**
 * @brief Reset the viewport to the drawable dimensions of the context
 *
 * @param ctx The relevant context
 */
void pgl_reset_viewport(pgl_ctx_t* ctx);

/**
 @brief Sets the line primitive width
 */
void pgl_set_line_width(pgl_ctx_t* ctx, float line_width);

/**
 * @brief Resets the line width to 1.0f
 */
void pgl_reset_line_width(pgl_ctx_t* ctx);

/**
 * @brief Resets the current state of the context
 *
 * Resets the global transform, projection, blend mode, and viewport.
 *
 * @param ctx The relevant context
 */
void pgl_reset_state(pgl_ctx_t* ctx);

/**
 * @brief Pushes the current state onto the state stack, allowing it to be
 * restored later
 *
 * @param ctx The relevant context
 */
void pgl_push_state(pgl_ctx_t* ctx);

/**
 * @brief Pops a state off of the state stack and makes it the current state
 *
 * @param ctx The relevant context
 */
void pgl_pop_state(pgl_ctx_t* ctx);

/**
 * @brief Removes all states from the state stack
 *
 * @param ctx The relevant context
 */
void pgl_clear_stack(pgl_ctx_t* ctx);

/**
 * @brief Sets a boolean uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param value  The boolean value
 */
void pgl_set_bool(pgl_shader_t* shader, const char* name, bool value);

/**
 * @brief Sets an integer uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param value  The integer value
 */
void pgl_set_1i(pgl_shader_t* shader, const char* name, int32_t a);

/**
 * @brief Sets a 2D integer uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param a      The first value
 * @param b      The second value
 */
void pgl_set_2i(pgl_shader_t* shader, const char* name, int32_t a, int32_t b);

/**
 * @brief Sets a 3D integer uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param a      The first value
 * @param b      The second value
 * @param c      The third value
 */
void pgl_set_3i(pgl_shader_t* shader, const char* name, int32_t a, int32_t b, int32_t c);

/**
 * @brief Sets a 4D integer uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param a      The first value
 * @param b      The second value
 * @param c      The third value
 * @param d      The fourth value
 */
void pgl_set_4i(pgl_shader_t* shader, const char* name, int32_t a, int32_t b,
                                                        int32_t c, int32_t d);
/**
 * @brief Sets a 2D integer uniform by vector
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param vec    The vector
 */
void pgl_set_v2i(pgl_shader_t* shader, const char* name, const pgl_v2i_t vec);

/**
 * @brief Sets a 3D integer uniform by vector
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param vec    The vector
 */
void pgl_set_v3i(pgl_shader_t* shader, const char* name, const pgl_v3i_t vec);

/**
 * @brief Sets a 4D integer uniform by vector
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param vec    The vector
 */
void pgl_set_v4i(pgl_shader_t* shader, const char* name, const pgl_v4i_t vec);

/**
 * @brief Sets an floating point uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param x      The float value
 */
void pgl_set_1f(pgl_shader_t* shader, const char* name, float x);

/**
 * @brief Sets a 2D floating point uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param x      The first value
 * @param y      The second value
 */
void pgl_set_2f(pgl_shader_t* shader, const char* name, float x, float y);

/**
 * @brief Sets a 3D floating point uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param x      The first value
 * @param y      The second value
 * @param z      The third value
 */
void pgl_set_3f(pgl_shader_t* shader, const char* name, float x, float y, float z);

/**
 * @brief Sets a 4D floating point uniform
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param x      The first value
 * @param y      The second value
 * @param w      The third value
 * @param z      The fourth value
 */
void pgl_set_4f(pgl_shader_t* shader, const char* name, float x, float y,
                                                        float z, float w);

/**
 * @brief Sets a 2D floating point uniform by vector
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param vec    The vector
 */
void pgl_set_v2f(pgl_shader_t* shader, const char* name, const pgl_v2f_t vec);

/**
 * @brief Sets a 3D floating point uniform by vector
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param vec    The vector
 */
void pgl_set_v3f(pgl_shader_t* shader, const char* name, const pgl_v3f_t vec);

/**
 * @brief Sets a 4D floating point uniform by vector
 *
 * @param shader The uniform's shader program
 * @param name   The name of the uniform
 * @param vec    The vector
 */
void pgl_set_v4f(pgl_shader_t* shader, const char* name, const pgl_v4f_t vec);

/**
 * @brief Sends an array of floating point numbers
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param array The array of floats
 * @param count The size of the array
 */
void pgl_set_a1f(pgl_shader_t* shader,
                 const char* name,
                 const float array[],
                 pgl_size_t count);

/**
 * @brief Sends an array of 2D floating point vectors
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param array The array of vectors
 * @param count The size of the array
 */
void pgl_set_a2f(pgl_shader_t* shader,
                 const char* name,
                 const pgl_v2f_t array[],
                 pgl_size_t count);

/**
 * @brief Sends an array of 3D floating point vectors
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param array The array of vectors
 * @param count The size of the array
 */
void pgl_set_a3f(pgl_shader_t* shader,
                 const char* name,
                 const pgl_v3f_t array[],
                 pgl_size_t count);

/**
 * @brief Sends an array of 4D floating point vectors
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param array The array of vectors
 * @param count The size of the array
 */
void pgl_set_a4f(pgl_shader_t* shader,
                 const char* name,
                 const pgl_v4f_t array[],
                 pgl_size_t count);

/**
 * @brief Sets a 2x2 floating point matrix
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param matrix The matrix
 */
void pgl_set_m2(pgl_shader_t* shader, const char* name, const pgl_m2_t matrix);

/**
 * @brief Sets a 3x3 floating point matrix
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param matrix The matrix
 */
void pgl_set_m3(pgl_shader_t* shader, const char* name, const pgl_m3_t matrix);

/**
 * @brief Sets a 4x4 floating point matrix
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param matrix The matrix
 */
void pgl_set_m4(pgl_shader_t* shader, const char* name, const pgl_m4_t matrix);

/**
 * @brief Sets a 2D sampler uniform
 *
 * @param shader The uniform's shader program
 * @param name The name of the uniform
 * @param value The sampler's texture unit
 */
void pgl_set_s2d(pgl_shader_t* shader, const char* name, int32_t value);

#endif // PICO_GL_H

#ifdef __cplusplus
}
#endif

#ifdef  PICO_GL_IMPLEMENTATION

#include "glad.h"

#if !defined(PICO_GL_MALLOC) || !defined(PICO_GL_FREE)
#include <stdlib.h>
#define PICO_GL_MALLOC(size, ctx) (malloc(size))
#define PICO_GL_FREE(ptr, ctx)    (free(ptr))
#endif

#ifdef NDEBUG
    #define PICO_GL_ASSERT(...)
#else
    #ifndef PICO_GL_ASSERT
        #include <assert.h>
        #define PICO_GL_ASSERT(expr) (assert(expr))
    #endif
#endif

#ifndef PICO_GL_LOG
    #include <stdio.h>
    #define  PICO_GL_LOG(...) (pgl_log(__VA_ARGS__))
#endif

#include <string.h>  // memset, strcmp

#ifndef PICO_GL_UNIFORM_NAME_LENGTH
#define PICO_GL_UNIFORM_NAME_LENGTH 32
#endif

#ifndef PICO_GL_MAX_UNIFORMS
#define PICO_GL_MAX_UNIFORMS 32
#endif

#ifndef PICO_GL_MAX_STATES
#define PICO_GL_MAX_STATES 32
#endif

/*=============================================================================
 * Internal aliases
 *============================================================================*/

#define PGL_MALLOC              PICO_GL_MALLOC
#define PGL_FREE                PICO_GL_FREE
#define PGL_ASSERT              PICO_GL_ASSERT
#define PGL_LOG                 PICO_GL_LOG
#define PGL_UNIFORM_NAME_LENGTH PICO_GL_UNIFORM_NAME_LENGTH
#define PGL_MAX_UNIFORMS        PICO_GL_MAX_UNIFORMS
#define PGL_MAX_STATES          PICO_GL_MAX_STATES

/*=============================================================================
 * Internal PGL enum to GL enum maps
 *============================================================================*/

#ifdef NDEBUG
    #define PGL_CHECK(expr) (expr)
#else
    #define PGL_CHECK(expr) do { \
        expr; pgl_log_error(__FILE__, __LINE__, #expr); \
    }  while(false)
#endif

static bool pgl_initialized = false;

static const char* pgl_error_msg_map[] =
{
    "No error",
    "Invalid enumeration value",
    "Invalid value",
    "Invalid operation",
    "Out of memory",
    "Invalid framebuffer operation",
    "Framebuffer is incomplete",
    "Shader compilation error",
    "Shader linking error",
    "Invalid texture dimensions",
    "Invalid texture format",
    "Invalid number of attributes",
    "Invalid number of uniforms",
    "Invalid uniform name",
    "Unknown error",
    0
};

static const GLenum pgl_format_map[] =
{
    GL_RED,
    GL_RGB,
    GL_RGBA,
    GL_BGR,
    GL_BGRA
};

static const GLenum pgl_blend_factor_map[] =
{
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA
};

static const GLenum pgl_blend_eq_map[] =
{
    GL_FUNC_ADD,
    GL_FUNC_SUBTRACT,
    GL_FUNC_REVERSE_SUBTRACT,
    GL_MIN,
    GL_MAX
};

static const GLenum pgl_primitive_map[] =
{
    GL_POINTS,
    GL_LINES,
    GL_LINE_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP
};

/*=============================================================================
 * Internal struct declarations
 *============================================================================*/

typedef uint32_t pgl_hash_t;

typedef struct
{
    bool    active;
    GLint   location;
    GLsizei size;
    GLvoid* offset;
} pgl_attribute_t;

typedef struct
{
	char       name[PGL_UNIFORM_NAME_LENGTH];
	GLsizei    size;
	GLenum     type;
	GLint      location;
	pgl_hash_t hash;
} pgl_uniform_t;

typedef struct
{
    int32_t x, y, w, h;
} pgl_viewport_t;

typedef struct
{
    pgl_blend_mode_t blend_mode;
    pgl_m4_t         transform;
    pgl_m4_t         projection;
    pgl_viewport_t   viewport;
    float            line_width;
} pgl_state_t;

typedef struct
{
    pgl_size_t size;
    pgl_state_t state;
    pgl_state_t array[PGL_MAX_STATES];
} pgl_state_stack_t;

/*=============================================================================
 * Internal function declarations
 *============================================================================*/

static void pgl_set_error(pgl_ctx_t* ctx, pgl_error_t code);

static const char* pgl_get_default_vert_shader();
static const char* pgl_get_default_frag_shader();

static pgl_state_stack_t* pgl_get_active_stack(pgl_ctx_t* ctx);
static pgl_state_t* pgl_get_active_state(pgl_ctx_t* ctx);

static void pgl_reset_last_state(pgl_ctx_t* ctx);

static void pgl_apply_blend(pgl_ctx_t* ctx, const pgl_blend_mode_t* mode);
static void pgl_apply_transform(pgl_ctx_t* ctx, const pgl_m4_t matrix);
static void pgl_apply_projection(pgl_ctx_t* ctx, const pgl_m4_t matrix);
static void pgl_apply_viewport(pgl_ctx_t* ctx, const pgl_viewport_t* viewport);

static void pgl_before_draw(pgl_ctx_t* ctx, pgl_texture_t* texture, pgl_shader_t* shader);
static void pgl_after_draw(pgl_ctx_t* ctx);

static int pgl_load_uniforms(pgl_shader_t* shader);
static const pgl_uniform_t* pgl_find_uniform(const pgl_shader_t* shader, const char* name);

static void pgl_bind_attributes();

static void pgl_log(const char* fmt, ...);
static void pgl_log_error(const char* file, unsigned line, const char* expr);
static pgl_error_t pgl_map_error(GLenum id);
static pgl_hash_t pgl_hash_str(const char* str);
static bool pgl_str_equal(const char* str1, const char* str2);
static bool pgl_mem_equal(const void* ptr1, const void* ptr2, size_t size);

/*=============================================================================
 * Internal constants
 *============================================================================*/

// 64-bit FNV1a Constants
//static const pgl_hash_t PGL_OFFSET_BASIS = 0xCBF29CE484222325;
//static const pgl_hash_t PGL_PRIME = 0x100000001B3;

static const pgl_hash_t PGL_OFFSET_BASIS = 0x811C9DC5;
static const pgl_hash_t PGL_PRIME = 0x1000193;

/*=============================================================================
 * Shaders GL3
 *============================================================================*/

#define PGL_GL_HDR "" \
"#version 330 core\n"

#define PGL_GLES_HDR "" \
"#version 310 es\n"

#define PGL_GL_VERT_BODY "" \
"layout (location = 0) in vec3 a_pos;\n" \
"layout (location = 1) in vec4 a_color;\n" \
"layout (location = 2) in vec2 a_uv;\n" \
"\n" \
"out vec4 color;\n" \
"out vec2 uv;\n" \
"\n" \
"uniform mat4 u_transform;\n" \
"uniform mat4 u_projection;\n" \
"\n" \
"void main()\n" \
"{\n" \
"   gl_Position = u_projection * u_transform * vec4(a_pos, 1);\n" \
"   color = a_color;\n" \
"   uv = a_uv;\n" \
"}\n"

#define PGL_GL_FRAG_BODY "" \
"#ifdef GL_ES\n" \
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n" \
"   precision highp float;\n" \
"#else\n" \
"   precision mediump float;\n" \
"#endif\n" \
"#endif\n" \
"out vec4 frag_color;\n" \
"\n" \
"in vec4 color;\n" \
"in vec2 uv;\n" \
"\n" \
"uniform sampler2D u_tex;\n" \
"\n" \
"void main()\n" \
"{\n" \
"   frag_color = texture(u_tex, uv) * color;\n" \
"}\n"

static const GLchar* PGL_GL_VERT_SRC = PGL_GL_HDR PGL_GL_VERT_BODY;
static const GLchar* PGL_GL_FRAG_SRC = PGL_GL_HDR PGL_GL_FRAG_BODY;

static const GLchar* PGL_GLES_VERT_SRC = PGL_GLES_HDR PGL_GL_VERT_BODY;
static const GLchar* PGL_GLES_FRAG_SRC = PGL_GLES_HDR PGL_GL_FRAG_BODY;

/*=============================================================================
 * Public API implementation
 *============================================================================*/

struct pgl_ctx_t
{
    pgl_error_t       error_code;
    pgl_shader_t*     shader;
    pgl_texture_t*    texture;
    pgl_texture_t*    target;
    pgl_state_t       last_state;
    pgl_state_stack_t stack;
    pgl_state_stack_t target_stack;
    GLuint            vao;
    GLuint            vbo;
    GLuint            ebo;
    uint32_t          w, h;
    uint32_t          samples;
    bool              srgb;
    bool              depth;
    bool              transpose;
    void*             mem_ctx;
};

struct pgl_shader_t
{
    pgl_ctx_t* ctx;

    GLuint program;

    pgl_size_t uniform_count;
    pgl_uniform_t uniforms[PGL_MAX_UNIFORMS];

    pgl_attribute_t pos;
    pgl_attribute_t color;
    pgl_attribute_t uv;
};

struct pgl_texture_t
{
    GLuint       id;
    pgl_ctx_t*   ctx;
    bool         target;
    int32_t      w, h;
    pgl_format_t fmt;
    bool         srgb;
    bool         smooth;
    bool         mipmap;
    GLuint       fbo;
    GLuint       fbo_msaa;
    GLuint       rbo_msaa;
    GLuint       depth_id;
    GLuint       depth_rbo_msaa;
};

struct pgl_buffer_t
{
    GLenum  primitive;
    GLuint  vao;
    GLuint  vbo;
    GLsizei count;
};

pgl_error_t pgl_get_error(pgl_ctx_t* ctx)
{
    return ctx->error_code;
}

const char* pgl_get_error_str(pgl_error_t code)
{
    PGL_ASSERT(code < PGL_ERROR_COUNT);

    if (code < PGL_ERROR_COUNT)
        return pgl_error_msg_map[(pgl_size_t)code];
    else
        return NULL;
}

pgl_version_t pgl_get_version()
{
    if (!pgl_initialized)
    {
        PGL_LOG("Library hasn't been initialized: call pgl_global_init");
        return PGL_VERSION_UNSUPPORTED;
    }

    if (GLAD_GL_VERSION_3_3)
        return PGL_GL3;

    if (GLAD_GL_ES_VERSION_3_1)
        return PGL_GLES3;

    return PGL_VERSION_UNSUPPORTED;
}

void pgl_print_info()
{
    if (!pgl_initialized)
    {
        PGL_LOG("Library hasn't been initialized: call pgl_global_init");
        return;
    }

    const unsigned char* vendor = glGetString(GL_VENDOR);
    const unsigned char* renderer = glGetString(GL_RENDERER);
    const unsigned char* gl_version = glGetString(GL_VERSION);
    const unsigned char* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

    int tex_w, tex_h;
    pgl_get_max_texture_size(&tex_w, &tex_h);

    PGL_LOG("OpenGL info:");
    PGL_LOG("Vendor: %s", vendor);
    PGL_LOG("Renderer: %s", renderer);
    PGL_LOG("GL Version: %s", gl_version);
    PGL_LOG("GLSL Version: %s", glsl_version);
    PGL_LOG("Max texture size: %ix%i", tex_w, tex_h);
}

int pgl_global_init(pgl_loader_fn loader_fp, bool gles)
{
    if (NULL == loader_fp && gles)
    {
        PGL_LOG("Loader must be explicitly specified for an GLES context");
        return -1;
    }

    if (gles)
    {
        if (!gladLoadGLES2Loader((GLADloadproc)loader_fp))
        {
            PGL_LOG("GLAD GLES2 loader failed");
            return -1;
        }

        pgl_initialized = true;

        return 0;
    }

    if (NULL == loader_fp)
    {
        if (!gladLoadGL())
        {
            PGL_LOG("GLAD GL3 loader failed");
            return -1;
        }
    }
    else
    {
        if (!gladLoadGLLoader((GLADloadproc)loader_fp))
        {
            PGL_LOG("GLAD GL3 loader failed");
            return -1;
        }
    }

    PGL_CHECK(glEnable(GL_BLEND));

    pgl_initialized = true;

    return 0;
}

pgl_ctx_t* pgl_create_context(uint32_t w, uint32_t h, bool depth,
                              uint32_t samples, bool srgb, void* mem_ctx)
{
    if (!pgl_initialized)
    {
        PGL_LOG("Library hasn't been initialized: call pgl_global_init");
        return NULL;
    }

    pgl_ctx_t* ctx = PGL_MALLOC(sizeof(pgl_ctx_t), mem_ctx);

    if (!ctx)
        return NULL;

    memset(ctx, 0, sizeof(pgl_ctx_t));

    ctx->w = w;
    ctx->h = h;
    ctx->samples = samples;
    ctx->srgb = srgb;
    ctx->depth = depth;
    ctx->mem_ctx = mem_ctx;

    // Create VBO/EBO
    PGL_CHECK(glGenVertexArrays(1, &ctx->vao));
    PGL_CHECK(glBindVertexArray(ctx->vao));
    PGL_CHECK(glGenBuffers(1, &ctx->vbo));
    PGL_CHECK(glGenBuffers(1, &ctx->ebo));
    PGL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->ebo));
    PGL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW));
    PGL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo));
    PGL_CHECK(glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW));
    pgl_bind_attributes();
    PGL_CHECK(glBindVertexArray(0));

    if (samples > 0)
    {
        GLint max_samples = 0;
        PGL_CHECK(glGetIntegerv(GL_MAX_SAMPLES, &max_samples));

        if (samples > (uint32_t)max_samples)
            ctx->samples = (uint32_t)max_samples;

        PGL_CHECK(glEnable(GL_MULTISAMPLE));
    }

    pgl_clear_stack(ctx);
    pgl_reset_state(ctx);

    return ctx;
}

void pgl_destroy_context(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    PGL_CHECK(glDeleteBuffers(1, &ctx->vbo));
    PGL_CHECK(glDeleteBuffers(1, &ctx->ebo));
    PGL_CHECK(glDeleteVertexArrays(1, &ctx->vao));
    PGL_FREE(ctx, ctx->mem_ctx);
}

void pgl_resize(pgl_ctx_t* ctx, uint32_t w, uint32_t h, bool reset_vp)
{
    PGL_ASSERT(ctx);

    ctx->w = w;
    ctx->h = h;

    if (reset_vp)
        pgl_reset_viewport(ctx);
}

pgl_shader_t* pgl_create_shader(pgl_ctx_t* ctx, const char* vert_src,
                                                const char* frag_src)
{
    PGL_ASSERT(ctx);

    if (!vert_src && !frag_src)
    {
        vert_src = pgl_get_default_vert_shader();
        frag_src = pgl_get_default_frag_shader();
    }
    else if (!vert_src)
    {
        vert_src = pgl_get_default_vert_shader();
    }
    else if (!frag_src)
    {
        frag_src = pgl_get_default_frag_shader();
    }

    // Create shaders
    GLuint vs, fs, program;

    PGL_CHECK(vs = glCreateShader(GL_VERTEX_SHADER));
    PGL_CHECK(fs = glCreateShader(GL_FRAGMENT_SHADER));

    GLsizei length;
    GLchar msg[2048];

    // Compile vertex shader
    GLint is_compiled = GL_FALSE;

    PGL_CHECK(glShaderSource(vs, 1, &vert_src, NULL));
    PGL_CHECK(glCompileShader(vs));
    PGL_CHECK(glGetShaderiv(vs, GL_COMPILE_STATUS, &is_compiled));

    if (GL_FALSE == is_compiled)
    {
        PGL_CHECK(glGetShaderInfoLog(vs, sizeof(msg), &length, msg));
        PGL_CHECK(glDeleteShader(vs));
        PGL_LOG("Error compiling vertex shader: %s", msg);
        pgl_set_error(ctx, PGL_SHADER_COMPILATION_ERROR);
        return NULL;
    }

    // Compile fragment shader
    is_compiled = GL_FALSE;

    PGL_CHECK(glShaderSource(fs, 1, &frag_src, NULL));
    PGL_CHECK(glCompileShader(fs));
    PGL_CHECK(glGetShaderiv(fs, GL_COMPILE_STATUS, &is_compiled));

    if (GL_FALSE == is_compiled)
    {
        PGL_CHECK(glGetShaderInfoLog(fs, sizeof(msg), &length, msg));
        PGL_CHECK(glDeleteShader(vs));
        PGL_CHECK(glDeleteShader(fs));
        PGL_LOG("Error compiling fragment shader: %s", msg);
        pgl_set_error(ctx, PGL_SHADER_COMPILATION_ERROR);
        return NULL;
    }

    // Link program
    GLint is_linked = GL_FALSE;

    PGL_CHECK(program = glCreateProgram());

    PGL_CHECK(glAttachShader(program, vs));
    PGL_CHECK(glAttachShader(program, fs));
    PGL_CHECK(glLinkProgram(program));
    PGL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &is_linked));

    PGL_CHECK(glDetachShader(program, vs));
    PGL_CHECK(glDetachShader(program, fs));
    PGL_CHECK(glDeleteShader(vs));
    PGL_CHECK(glDeleteShader(fs));

    if (GL_FALSE == is_linked)
    {
        PGL_CHECK(glGetProgramInfoLog(program, sizeof(msg), &length, msg));
        PGL_CHECK(glDeleteProgram(program));
        PGL_LOG("Error linking shader program: %s", msg);
        pgl_set_error(ctx, PGL_SHADER_LINKING_ERROR);
        return NULL;
    }

    pgl_shader_t* shader = PGL_MALLOC(sizeof(pgl_shader_t), ctx->mem_ctx);

    if (!shader)
    {
        pgl_set_error(ctx, PGL_OUT_OF_MEMORY);
        return NULL;
    }

    memset(shader, 0, sizeof(pgl_shader_t));

    shader->program = program;
    shader->ctx = ctx;

    pgl_bind_shader(ctx, shader);
    pgl_load_uniforms(shader);

    return shader;
}

void pgl_destroy_shader(pgl_shader_t* shader)

{
    PGL_ASSERT(shader);

    pgl_bind_shader(shader->ctx, NULL);
    PGL_CHECK(glDeleteProgram(shader->program));
    PGL_FREE(shader, shader->ctx->mem_ctx);
}

uint64_t pgl_get_shader_id(const pgl_shader_t* shader)
{
    PGL_ASSERT(shader);

    return shader->program;
}

void pgl_bind_shader(pgl_ctx_t* ctx, pgl_shader_t* shader)
{
    PGL_ASSERT(ctx);

    if (ctx->shader == shader)
        return;

    if (NULL != shader)
    {
        PGL_CHECK(glUseProgram(shader->program));
        ctx->shader = shader;
    }
    else
    {
        PGL_CHECK(glUseProgram(0));
        ctx->shader = NULL;
    }
}

static void pgl_set_texture_params(GLuint tex_id, bool smooth, bool repeat)
{
    PGL_CHECK(glBindTexture(GL_TEXTURE_2D, tex_id));

    PGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                              smooth ? GL_LINEAR : GL_NEAREST));

    PGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                              smooth ? GL_LINEAR : GL_NEAREST));

    PGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                              repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));

    PGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                              repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));
}

pgl_texture_t* pgl_create_texture(pgl_ctx_t* ctx,
                                  bool target,
                                  pgl_format_t fmt, bool srgb,
                                  int32_t w, int32_t h,
                                  bool smooth, bool repeat)
{
    PGL_ASSERT(ctx);

    // Check texture dimensions
    if (w <= 0 || h <= 0)
    {
        PGL_LOG("Texture dimensions must be positive (w: %i, h: %i)", w, h);
        pgl_set_error(ctx, PGL_INVALID_TEXTURE_SIZE);
        return NULL;
    }

    GLint max_size;
    PGL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size));

    if (w > max_size || h > max_size)
    {
        PGL_LOG("Texture dimensions exceed max size (w: %i, h: %i)", w, h);
        pgl_set_error(ctx, PGL_INVALID_TEXTURE_SIZE);
        return NULL;
    }

    // Allocate texture
    pgl_texture_t* tex = PGL_MALLOC(sizeof(pgl_texture_t), ctx->mem_ctx);

    if (!tex)
    {
        pgl_set_error(ctx, PGL_OUT_OF_MEMORY);
        return NULL;
    }

    PGL_CHECK(glGenTextures(1, &tex->id));

    tex->ctx = ctx;
    tex->w = w;
    tex->h = h;
    tex->fmt = fmt;
    tex->srgb = srgb;
    tex->target = target;
    tex->smooth = smooth;

    if (-1 == pgl_upload_texture(ctx, tex, w, h, NULL))
    {
        PGL_CHECK(glDeleteTextures(1, &tex->id));
        PGL_FREE(tex, ctx->mem_ctx);
        return NULL;
    }

    pgl_set_texture_params(tex->id, smooth, repeat);

    // Generate objects for render target
    if (target)
    {
        // Generate framebuffer
        PGL_CHECK(glGenFramebuffers(1, &tex->fbo));

        // Create depth texture
        if (ctx->depth)
        {
            PGL_CHECK(glGenTextures(1, &tex->depth_id));

            pgl_set_texture_params(tex->depth_id, smooth, repeat);

            PGL_CHECK(glBindTexture(GL_TEXTURE_2D, tex->depth_id));
            PGL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
                                   GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL));
        }

        // Generate multi-sample buffers for MSAA
        if (ctx->samples > 0)
        {
            PGL_CHECK(glGenFramebuffers(1, &tex->fbo_msaa));
            PGL_CHECK(glGenRenderbuffers(1, &tex->rbo_msaa));

            // Generate depth buffer for MSAA
            if (ctx->depth)
                PGL_CHECK(glGenRenderbuffers(1, &tex->depth_rbo_msaa));
        }
    }

    // TODO: Consider breaking up this function
    // Create attachments
    if (target)
    {
        // Bind framebuffer
        PGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, tex->fbo));

        // Create framebuffer attachment
        PGL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                         GL_COLOR_ATTACHMENT0,
                                         GL_TEXTURE_2D, tex->id, 0));


        // Create framebuffer depth attachment
        if (ctx->depth)
        {

            PGL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                             GL_DEPTH_ATTACHMENT,
                                             GL_TEXTURE_2D, tex->depth_id, 0));
        }

        // Create attachments for MSAA
        if (ctx->samples > 0)
        {
            // Bind MSAA render buffer
            PGL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, tex->rbo_msaa));

            // Create multi-sample buffer storage
            PGL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                       ctx->samples,
                                                       ctx->srgb ?
                                                       GL_SRGB8_ALPHA8 : GL_RGBA,
                                                       tex->w, tex->h));

            PGL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));

            // Bind MSAA framebuffer
            PGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, tex->fbo_msaa));

            // Create color attachment
            PGL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                                GL_COLOR_ATTACHMENT0,
                                                GL_RENDERBUFFER, tex->rbo_msaa));

            // Create attachment for MSAA with depth test
            if (ctx->depth)
            {
                // Bind MSAA depth buffer
                PGL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, tex->depth_rbo_msaa));

                // Create multi-sample depth buffer storage
                PGL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                           ctx->samples,
                                                           GL_DEPTH_COMPONENT24,
                                                           tex->w, tex->h));

                // Create depth attachment
                PGL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                                    GL_DEPTH_ATTACHMENT,
                                                    GL_RENDERBUFFER,
                                                    tex->depth_rbo_msaa));
            }
        }

        // Check to see if framebuffer is complete
        GLenum status;
        PGL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            PGL_LOG("Framebuffer incomplete");
            pgl_set_error(ctx, PGL_FRAMEBUFFER_INCOMPLETE);

            // Framebuffer is incomplete, so release resources

            PGL_CHECK(glDeleteTextures(1, &tex->id));
            PGL_CHECK(glDeleteFramebuffers(1, &tex->fbo));

            if (ctx->depth)
            {
                PGL_CHECK(glDeleteTextures(1, &tex->depth_id));
            }

            if (ctx->samples > 0)
            {
                PGL_CHECK(glDeleteRenderbuffers(1, &tex->rbo_msaa));
                PGL_CHECK(glDeleteFramebuffers(1, &tex->fbo_msaa));

                if (ctx->depth)
                {
                    PGL_CHECK(glDeleteRenderbuffers(1, &tex->depth_rbo_msaa));
                }
            }

            PGL_FREE(tex, ctx->mem_ctx);

            return NULL;

        }

        // Ensure framebuffer objects are not bound
        PGL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
        PGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    return tex;
}

pgl_texture_t* pgl_texture_from_bitmap(pgl_ctx_t* ctx,
                                       pgl_format_t fmt, bool srgb,
                                       int32_t w, int32_t h,
                                       bool smooth, bool repeat,
                                       const unsigned char* bitmap)

{
    PGL_ASSERT(ctx);
    PGL_ASSERT(bitmap);

    pgl_texture_t* tex = pgl_create_texture(ctx, false, fmt, srgb, w, h, smooth, repeat);

    if (!tex)
        return NULL;

    if (-1 == pgl_upload_texture(ctx, tex, w, h, bitmap))
    {
        PGL_CHECK(glDeleteTextures(1, &tex->id));
        PGL_FREE(tex, ctx->mem_ctx);
        return NULL;
    }

    return tex;
}

int pgl_upload_texture(pgl_ctx_t* ctx,
                       pgl_texture_t* texture,
                       int32_t w, int32_t h,
                       const unsigned char* bitmap)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(texture);

    pgl_bind_texture(ctx, texture);

    PGL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, texture->srgb ?
                                             GL_SRGB8_ALPHA8 : GL_RGBA,
                                             w, h, 0,
                                             pgl_format_map[texture->fmt],
                                             GL_UNSIGNED_BYTE, bitmap));

    return 0;
}

void pgl_update_texture(pgl_ctx_t* ctx,
                       pgl_texture_t* texture,
                       int x, int y,
                       int w, int h,
                       const uint8_t* bitmap)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(texture);
    PGL_ASSERT(bitmap);

    pgl_bind_texture(ctx, texture);

    GLenum fmt = pgl_format_map[texture->fmt];

    PGL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt,
                              GL_UNSIGNED_BYTE, bitmap));
}

int pgl_generate_mipmap(pgl_texture_t* texture, bool linear)
{
    PGL_ASSERT(texture);

    if (texture->mipmap) // TODO: Return error? void?
        return 0;

    pgl_bind_texture(texture->ctx, texture);

    if (linear)
    {
        PGL_CHECK(glTexParameteri(GL_TEXTURE_2D,    GL_TEXTURE_MIN_FILTER,
                                  texture->smooth ? GL_LINEAR_MIPMAP_LINEAR :
                                                    GL_NEAREST_MIPMAP_LINEAR));
    }
    else
    {
        PGL_CHECK(glTexParameteri(GL_TEXTURE_2D,    GL_TEXTURE_MIN_FILTER,
                                  texture->smooth ? GL_LINEAR_MIPMAP_NEAREST :
                                                    GL_NEAREST_MIPMAP_NEAREST));
    }

    PGL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

    texture->mipmap = true;

    return 0;
}

void pgl_destroy_texture(pgl_texture_t* tex)
{
    PGL_ASSERT(tex);

    pgl_bind_texture(tex->ctx, NULL);
    PGL_CHECK(glDeleteTextures(1, &tex->id));

    if (tex->target)
    {
        PGL_CHECK(glDeleteFramebuffers(1, &tex->fbo));

        if (tex->ctx->depth)
        {
            PGL_CHECK(glDeleteTextures(1, &tex->depth_id));
        }

        if (tex->ctx->samples > 0)
        {
            PGL_CHECK(glDeleteRenderbuffers(1, &tex->rbo_msaa));
            PGL_CHECK(glDeleteFramebuffers(1, &tex->fbo_msaa));

            if (tex->ctx->depth)
            {
                PGL_CHECK(glDeleteRenderbuffers(1, &tex->depth_rbo_msaa));
            }
        }
    }

    PGL_FREE(tex, tex->ctx->mem_ctx);
}

void pgl_get_texture_size(const pgl_texture_t* texture, int* w, int* h)
{
    PGL_ASSERT(texture);

    if (w) *w = texture->w;
    if (h) *h = texture->h;
}

void pgl_get_max_texture_size(int* w, int* h)
{
    int max_size = 0;
    PGL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size));

    if (w) *w = max_size;
    if (h) *h = max_size;
}

uint64_t pgl_get_texture_id(const pgl_texture_t* texture)
{
    PGL_ASSERT(texture);
    return texture->id;
}

void pgl_bind_texture(pgl_ctx_t* ctx, pgl_texture_t* texture)
{
    PGL_ASSERT(ctx);

    if (ctx->texture == texture)
        return;

    if (NULL != texture)
    {
        PGL_CHECK(glBindTexture(GL_TEXTURE_2D, texture->id));
        ctx->texture = texture;
    }
    else
    {
        PGL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
        ctx->texture = NULL;
    }
}

int pgl_set_render_target(pgl_ctx_t* ctx, pgl_texture_t* target)
{
    PGL_ASSERT(ctx);

    if (ctx->target == target)
        return 0;

    if (ctx->target && ctx->samples > 0)
    {
        PGL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER,  ctx->target->fbo_msaa));
        PGL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER,  ctx->target->fbo));
        PGL_CHECK(glBlitFramebuffer(0, 0, ctx->target->w, ctx->target->h,
                                    0, 0, ctx->target->w, ctx->target->h,
                                    GL_COLOR_BUFFER_BIT,  GL_LINEAR));
    }

    if (!target)
    {
        PGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        ctx->target = NULL;
        pgl_reset_last_state(ctx);

        return 0;
    }

    if (ctx->samples > 0)
        PGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, target->fbo_msaa));
    else
        PGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, target->fbo));

    ctx->target = target;

    pgl_clear_stack(ctx);
    pgl_reset_state(ctx);
    pgl_reset_last_state(ctx);
    pgl_set_viewport(ctx, 0, 0, target->w, target->h);

    return 0;
}

void pgl_clear(float r, float g, float b, float a)
{
    PGL_CHECK(glClearColor(r, g, b, a));
    PGL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void pgl_draw_array(pgl_ctx_t* ctx,
                   pgl_primitive_t primitive,
                   const pgl_vertex_t* vertices,
                   pgl_size_t count,
                   pgl_texture_t* texture,
                   pgl_shader_t* shader)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(vertices);
    PGL_ASSERT(shader);

    pgl_before_draw(ctx, texture, shader);

    PGL_CHECK(glBindVertexArray(ctx->vao));

    PGL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo));
    PGL_CHECK(glBufferData(GL_ARRAY_BUFFER, count * sizeof(pgl_vertex_t), vertices, GL_STATIC_DRAW));

    PGL_CHECK(glDrawArrays(pgl_primitive_map[primitive], 0, count));
    PGL_CHECK(glBindVertexArray(0));

    pgl_after_draw(ctx);
}

void pgl_draw_indexed_array(pgl_ctx_t* ctx,
                            pgl_primitive_t primitive,
                            const pgl_vertex_t* vertices, pgl_size_t vertex_count,
                            const uint32_t* indices, pgl_size_t index_count,
                            pgl_texture_t* texture,
                            pgl_shader_t* shader)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(vertices);
    PGL_ASSERT(indices);
    PGL_ASSERT(shader);

    pgl_before_draw(ctx, texture, shader);

    PGL_CHECK(glBindVertexArray(ctx->vao));

    PGL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo));
    PGL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(pgl_vertex_t), vertices, GL_STATIC_DRAW));

    PGL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->ebo));
    PGL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), indices, GL_STATIC_DRAW));

    PGL_CHECK(glDrawElements(pgl_primitive_map[primitive], index_count, GL_UNSIGNED_INT, 0));
    PGL_CHECK(glBindVertexArray(0));

    pgl_after_draw(ctx);
}

pgl_buffer_t* pgl_create_buffer(pgl_ctx_t* ctx,
                                pgl_primitive_t primitive,
                                const pgl_vertex_t* vertices,
                                pgl_size_t count)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(vertices);

    pgl_buffer_t* buffer = PGL_MALLOC(sizeof(pgl_buffer_t), ctx->mem_ctx);

    if (!buffer)
    {
        pgl_set_error(ctx, PGL_OUT_OF_MEMORY);
        return NULL;
    }

    PGL_CHECK(glGenVertexArrays(1, &buffer->vao));
    PGL_CHECK(glGenBuffers(1, &buffer->vbo));

    PGL_CHECK(glBindVertexArray(buffer->vao));

    PGL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo));
    PGL_CHECK(glBufferData(GL_ARRAY_BUFFER, count * sizeof(pgl_vertex_t), vertices, GL_DYNAMIC_DRAW));

    pgl_bind_attributes();
    PGL_CHECK(glBindVertexArray(0));

    buffer->primitive = pgl_primitive_map[primitive];
    buffer->count = count;

    return buffer;
}

void pgl_sub_buffer_data(pgl_ctx_t* ctx,
                         pgl_buffer_t* buffer,
                         const pgl_vertex_t* vertices,
                         pgl_size_t count,
                         pgl_size_t offset)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(vertices);
    PGL_ASSERT(count + offset <= (pgl_size_t)buffer->count);

    PGL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo));
    PGL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, count * sizeof(pgl_vertex_t), vertices));

    buffer->count = count;
}

void pgl_destroy_buffer(pgl_buffer_t* buffer)
{
    PGL_ASSERT(buffer);

    PGL_CHECK(glDeleteVertexArrays(1, &buffer->vao));
    PGL_CHECK(glDeleteBuffers(1, &buffer->vbo));

    PGL_FREE(buffer, buffer->ctx->mem_ctx);
}

void pgl_draw_buffer(pgl_ctx_t* ctx,
                     const pgl_buffer_t* buffer,
                     pgl_size_t start, pgl_size_t count,
                     pgl_texture_t* texture,
                     pgl_shader_t* shader)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(buffer);
    PGL_ASSERT(shader);

    PGL_ASSERT(start + count <= (pgl_size_t)buffer->count);

    pgl_before_draw(ctx, texture, shader);

    PGL_CHECK(glBindVertexArray(buffer->vao));
    PGL_CHECK(glDrawArrays(buffer->primitive, start, count));
    PGL_CHECK(glBindVertexArray(0));

    pgl_after_draw(ctx);
}

void pgl_set_transpose(pgl_ctx_t* ctx, bool enabled)
{
    PGL_ASSERT(ctx);
    ctx->transpose = enabled;
}

void pgl_set_blend_mode(pgl_ctx_t* ctx, pgl_blend_mode_t mode)
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);
    state->blend_mode = mode;
}

void pgl_reset_blend_mode(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);

    pgl_blend_mode_t mode = {
        PGL_SRC_ALPHA,
        PGL_ONE_MINUS_SRC_ALPHA,
        PGL_FUNC_ADD,
        PGL_ONE,
        PGL_ONE_MINUS_SRC_ALPHA,
        PGL_FUNC_ADD
    };

    state->blend_mode = mode;
}

void pgl_set_transform(pgl_ctx_t* ctx, const pgl_m4_t matrix)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(matrix);

    pgl_state_t* state = pgl_get_active_state(ctx);
    memcpy(state->transform, matrix, sizeof(pgl_m4_t));
}

void pgl_set_transform_3d(pgl_ctx_t* ctx, const pgl_m3_t matrix)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(matrix);

    if (ctx->transpose)
    {
        const pgl_m4_t matrix4 =
        {
            matrix[0], matrix[1], 0.0f, matrix[2],
            matrix[3], matrix[4], 0.0f, matrix[5],
            0.0f,      0.0f,      1.0f, 0.0f,
            matrix[6], matrix[7], 0.0f, matrix[8]
        };

        pgl_set_transform(ctx, matrix4);
    }
    else
    {
        const pgl_m4_t matrix4 =
        {
            matrix[0], matrix[3], 0.0f, matrix[6],
            matrix[1], matrix[4], 0.0f, matrix[7],
            0.0f,      0.0f,      1.0f, 0.0f,
            matrix[2], matrix[5], 0.0f, matrix[8]
        };

        pgl_set_transform(ctx, matrix4);
    }
}

void pgl_reset_transform(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);

    const pgl_m4_t matrix =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy(state->transform, matrix, sizeof(pgl_m4_t));
}

void pgl_set_projection(pgl_ctx_t* ctx, const pgl_m4_t matrix)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(matrix);

    pgl_state_t* state = pgl_get_active_state(ctx);
    memcpy(state->projection, matrix, sizeof(pgl_m4_t));
}

void pgl_set_projection_3d(pgl_ctx_t* ctx, const pgl_m3_t matrix)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(matrix);

    if (ctx->transpose)
    {
        const pgl_m4_t matrix4 =
        {
            matrix[0], matrix[1], 0.0f, matrix[2],
            matrix[3], matrix[4], 0.0f, matrix[5],
            0.0f,      0.0f,      1.0f, 0.0f,
            matrix[6], matrix[7], 0.0f, matrix[8]
        };

        pgl_set_projection(ctx, matrix4);
    }
    else
    {
        const pgl_m4_t matrix4 =
        {
            matrix[0], matrix[3], 0.0f, matrix[6],
            matrix[1], matrix[4], 0.0f, matrix[7],
            0.0f,      0.0f,      1.0f, 0.0f,
            matrix[2], matrix[5], 0.0f, matrix[8]
        };

        pgl_set_projection(ctx, matrix4);
    }
}

void pgl_reset_projection(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);

    const pgl_m4_t matrix =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy(state->projection, matrix, sizeof(pgl_m4_t));
}

void pgl_set_viewport(pgl_ctx_t* ctx, int32_t x, int32_t y,
                                      int32_t w, int32_t h)
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);
    state->viewport = (pgl_viewport_t){ x, y, w, h };
}

void pgl_reset_viewport(pgl_ctx_t* ctx) // TODO: get GL viewport?
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);
    state->viewport = (pgl_viewport_t){ 0, 0, ctx->w, ctx->h };
}

void pgl_set_line_width(pgl_ctx_t* ctx, float width)
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);
    state->line_width = width;
}

void pgl_reset_line_width(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_t* state = pgl_get_active_state(ctx);
    state->line_width = 1.0f;
}

void pgl_reset_state(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);
    pgl_reset_blend_mode(ctx);
    pgl_reset_transform(ctx);
    pgl_reset_projection(ctx);
    pgl_reset_viewport(ctx);
    pgl_reset_line_width(ctx);
}

void pgl_push_state(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    // TODO: Make into growable array

    pgl_state_stack_t* stack = pgl_get_active_stack(ctx);
    PGL_ASSERT(stack->size < PGL_MAX_STATES);

    stack->array[stack->size] = stack->state;
    stack->size++;
}

void pgl_pop_state(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_stack_t* stack = pgl_get_active_stack(ctx);

    PGL_ASSERT(stack->size > 0);

    stack->state = stack->array[stack->size];
    stack->size--;
}

void pgl_clear_stack(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_stack_t* stack = pgl_get_active_stack(ctx);
    stack->size = 0;
}

void pgl_set_bool(pgl_shader_t* shader, const char* name, bool value)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform1i(uniform->location, value));
}

void pgl_set_1i(pgl_shader_t* shader, const char* name, int32_t a)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform1i(uniform->location, a));
}

void pgl_set_2i(pgl_shader_t* shader, const char* name, int32_t a, int32_t b)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform2i(uniform->location, a, b));
}

void pgl_set_3i(pgl_shader_t* shader, const char* name, int32_t a, int32_t b,
                                                        int32_t c)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform3i(uniform->location, a, b, c));
}

void pgl_set_4i(pgl_shader_t* shader, const char* name, int32_t a, int32_t b,
                                                        int32_t c, int32_t d)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform4i(uniform->location, a, b, c, d));
}

void pgl_set_v2i(pgl_shader_t* shader, const char* name, const pgl_v2i_t vec)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_set_2i(shader, name, vec[0], vec[1]);
}

void pgl_set_v3i(pgl_shader_t* shader, const char* name, const pgl_v3i_t vec)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_set_3i(shader, name, vec[0], vec[1], vec[2]);
}


void pgl_set_v4i(pgl_shader_t* shader, const char* name, const pgl_v4i_t vec)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_set_4i(shader, name, vec[0], vec[1], vec[2], vec[3]);
}

void pgl_set_1f(pgl_shader_t* shader, const char* name, float x)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform1f(uniform->location, x));
}

void pgl_set_2f(pgl_shader_t* shader, const char* name, float x, float y)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform2f(uniform->location, x, y));
}

void pgl_set_3f(pgl_shader_t* shader, const char* name, float x, float y, float z)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform3f(uniform->location, x, y, z));
}

void pgl_set_4f(pgl_shader_t* shader, const char* name, float x, float y,
                                                        float z, float w)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform4f(uniform->location, x, y, z, w));
}

void pgl_set_v2f(pgl_shader_t* shader, const char* name, const pgl_v2f_t vec)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(vec);

    pgl_set_2f(shader, name, vec[0], vec[1]);
}

void pgl_set_v3f(pgl_shader_t* shader, const char* name, const pgl_v3f_t vec)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(vec);

    pgl_set_3f(shader, name, vec[0], vec[1], vec[2]);
}

void pgl_set_v4f(pgl_shader_t* shader, const char* name, const pgl_v4f_t vec)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(vec);

    pgl_set_4f(shader, name, vec[0], vec[1], vec[2], vec[3]);
}

void pgl_set_a1f(pgl_shader_t* shader,
                const char* name,
                const float* values,
                pgl_size_t count)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(values);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform1fv(uniform->location, count, values));
}

void pgl_set_a2f(pgl_shader_t* shader,
                const char* name,
                const pgl_v2f_t* vec,
                pgl_size_t count)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(vec);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    float values[2 * count];

    pgl_size_t i, j;

    for (i = 0, j = 0; j < count; i += 2, j++)
    {
        values[i]     = vec[j][0];
        values[i + 1] = vec[j][1];
    }

    PGL_CHECK(glUniform2fv(uniform->location, count, values));
}

void pgl_set_a3f(pgl_shader_t* shader,
                const char* name,
                const pgl_v3f_t* vec,
                pgl_size_t count)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(vec);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    float values[3 * count];

    pgl_size_t i, j;

    for (i = 0, j = 0; j < count; i += 3, j++)
    {
        values[i]     = vec[j][0];
        values[i + 1] = vec[j][1];
        values[i + 2] = vec[j][2];
    }

    PGL_CHECK(glUniform3fv(uniform->location, count, values));
}

void pgl_set_a4f(pgl_shader_t* shader,
                const char* name,
                const pgl_v4f_t* vec,
                pgl_size_t count)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(vec);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    float values[4 * count];

    pgl_size_t i, j;

    for (i = 0, j = 0; j < count; i += 4, j++)
    {
        values[i]     = vec[j][0];
        values[i + 1] = vec[j][1];
        values[i + 2] = vec[j][2];
        values[i + 3] = vec[j][3];
    }

    PGL_CHECK(glUniform4fv(uniform->location, count, values));
}

void pgl_set_m2(pgl_shader_t* shader, const char* name, const pgl_m2_t matrix)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(matrix);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniformMatrix2fv(uniform->location, uniform->size, shader->ctx->transpose, (float*)matrix));
}

void pgl_set_m3(pgl_shader_t* shader, const char* name, const pgl_m3_t matrix)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(matrix);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniformMatrix3fv(uniform->location, uniform->size, shader->ctx->transpose, (float*)matrix));
}

void pgl_set_m4(pgl_shader_t* shader, const char* name, const pgl_m4_t matrix)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);
    PGL_ASSERT(matrix);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniformMatrix4fv(uniform->location, uniform->size, shader->ctx->transpose, (float*)matrix));
}

void pgl_set_s2d(pgl_shader_t* shader, const char* name, int32_t value)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name);

    pgl_bind_shader(shader->ctx, shader);
    const pgl_uniform_t* uniform = pgl_find_uniform(shader, name);

    if (!uniform)
        return;

    PGL_CHECK(glUniform1i(uniform->location, value));
}

/*=============================================================================
 * Internal API implementation
 *============================================================================*/

static void pgl_set_error(pgl_ctx_t* ctx, pgl_error_t code)
{
    PGL_ASSERT(ctx);
    ctx->error_code = code;
}

static const char* pgl_get_default_vert_shader()
{
    pgl_version_t ver = pgl_get_version();

    switch (ver)
    {
        case PGL_GL3:
            return PGL_GL_VERT_SRC;

        case PGL_GLES3:
            return PGL_GLES_VERT_SRC;

        default:
            return NULL;
    }
}

static const char* pgl_get_default_frag_shader()
{
    pgl_version_t ver = pgl_get_version();

    switch (ver)
    {
        case PGL_GL3:
            return PGL_GL_FRAG_SRC;

        case PGL_GLES3:
            return PGL_GLES_FRAG_SRC;

        default:
            return NULL;
    }
}

static void pgl_reset_last_state(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_t* last = &ctx->last_state;

    memset(last, 0, sizeof(pgl_state_t));

    pgl_blend_mode_t mode =
    {
        PGL_FACTOR_COUNT, PGL_FACTOR_COUNT, PGL_EQ_COUNT,
        PGL_FACTOR_COUNT, PGL_FACTOR_COUNT, PGL_EQ_COUNT
    };

    last->blend_mode = mode;
}

static pgl_state_stack_t* pgl_get_active_stack(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    pgl_state_stack_t* stack = &ctx->stack;

    if (ctx->target)
        stack = &ctx->target_stack;

    return stack;
}

static pgl_state_t* pgl_get_active_state(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    return &pgl_get_active_stack(ctx)->state;
}

static void pgl_apply_blend(pgl_ctx_t* ctx, const pgl_blend_mode_t* mode)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(mode);

    if (pgl_mem_equal(mode, &ctx->last_state.blend_mode, sizeof(pgl_blend_mode_t)))
        return;

    PGL_CHECK(glBlendFuncSeparate(pgl_blend_factor_map[mode->color_src],
                                  pgl_blend_factor_map[mode->color_dst],
                                  pgl_blend_factor_map[mode->alpha_src],
                                  pgl_blend_factor_map[mode->alpha_dst]));

    PGL_CHECK(glBlendEquationSeparate(pgl_blend_eq_map[mode->color_eq],
                                      pgl_blend_eq_map[mode->alpha_eq]));
}

static void pgl_apply_transform(pgl_ctx_t* ctx, const pgl_m4_t matrix)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(matrix);

    if (pgl_mem_equal(matrix, ctx->last_state.transform, sizeof(pgl_m4_t)))
        return;

    pgl_set_m4(ctx->shader, "u_transform", matrix);
}

static void pgl_apply_projection(pgl_ctx_t* ctx, const pgl_m4_t matrix)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(matrix);

    if (pgl_mem_equal(matrix, &ctx->last_state.projection, sizeof(pgl_m4_t)))
        return;

    pgl_set_m4(ctx->shader, "u_projection", matrix);
}

static void pgl_apply_viewport(pgl_ctx_t* ctx, const pgl_viewport_t* viewport)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(viewport);

    if (viewport->w <= 0 && viewport->h <= 0)
        return;

    if (pgl_mem_equal(viewport, &ctx->last_state.viewport, sizeof(pgl_viewport_t)))
        return;

    PGL_CHECK(glViewport(viewport->x, viewport->y, viewport->w, viewport->h));
}

static void pgl_apply_line_width(pgl_ctx_t* ctx, float line_width)
{
    PGL_ASSERT(ctx);

    if (line_width == ctx->last_state.line_width)
        return;

    PGL_CHECK(glLineWidth(line_width));
}

static void pgl_before_draw(pgl_ctx_t* ctx, pgl_texture_t* texture, pgl_shader_t* shader)
{
    PGL_ASSERT(ctx);
    PGL_ASSERT(shader);

    pgl_bind_texture(ctx, texture);
    pgl_bind_shader(ctx, shader);

    pgl_state_t* state = pgl_get_active_state(ctx);

    pgl_apply_viewport(ctx, &state->viewport);
    pgl_apply_blend(ctx, &state->blend_mode);
    pgl_apply_transform(ctx, state->transform);
    pgl_apply_projection(ctx, state->projection);
    pgl_apply_line_width(ctx, state->line_width);

    glEnable(GL_BLEND);

    if (ctx->depth)
        PGL_CHECK(glEnable(GL_DEPTH_TEST));
    else
        PGL_CHECK(glDisable(GL_DEPTH_TEST));

    if (ctx->srgb)
        PGL_CHECK(glEnable(GL_FRAMEBUFFER_SRGB));
    else
        PGL_CHECK(glDisable(GL_FRAMEBUFFER_SRGB));
}

static void pgl_after_draw(pgl_ctx_t* ctx)
{
    PGL_ASSERT(ctx);

    ctx->last_state = *pgl_get_active_state(ctx);
}

static int pgl_load_uniforms(pgl_shader_t* shader)
{
    PGL_ASSERT(shader);

    // Get number of active uniforms
    GLint uniform_count;
    glGetProgramiv(shader->program, GL_ACTIVE_UNIFORMS, &uniform_count);
    shader->uniform_count = uniform_count;

    // Validate number of uniforms
    PGL_ASSERT(uniform_count < PGL_MAX_UNIFORMS);

    if (uniform_count >= PGL_MAX_UNIFORMS)
    {
        pgl_set_error(shader->ctx, PGL_INVALID_UNIFORM_COUNT);
        return -1;
    }

    // Loop through active uniforms and add them to the uniform array
    for (GLint i = 0; i < uniform_count; i++)
    {
        pgl_uniform_t uniform;
        GLsizei name_length;

        // Uniform index
        GLint index = i;

        // Get uniform information
        PGL_CHECK(glGetActiveUniform(shader->program, index,
                                     PGL_UNIFORM_NAME_LENGTH, // Max name length
                                     &name_length, &uniform.size, &uniform.type,
                                     uniform.name));

        // Validate name length
        PGL_ASSERT(name_length <= PGL_UNIFORM_NAME_LENGTH);

        if (name_length > PGL_UNIFORM_NAME_LENGTH)
        {
            pgl_set_error(shader->ctx, PGL_INVALID_UNIFORM_NAME);
            return -1;
        }

        // Get uniform location
        uniform.location = glGetUniformLocation(shader->program, uniform.name);

        // Hash name for fast lookups
        uniform.hash = pgl_hash_str(uniform.name);

        // Store uniform in the array
        shader->uniforms[i] = uniform;
    }

	return 0;
}

static const pgl_uniform_t* pgl_find_uniform(const pgl_shader_t* shader, const char* name)
{
    PGL_ASSERT(shader);
    PGL_ASSERT(name && strlen(name) > 0);

	pgl_size_t uniform_count = shader->uniform_count;
	const pgl_uniform_t* uniforms = shader->uniforms;

	uint64_t hash = pgl_hash_str(name);

	for (pgl_size_t i = 0; i < uniform_count; i++)
	{
		const pgl_uniform_t* uniform = &uniforms[i];

		if (uniform->hash == hash && pgl_str_equal(name, uniform->name))
		{
			return uniform;
        }
	}

    return NULL;
}

static void pgl_bind_attributes()
{
    // Position
    PGL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                                    sizeof(pgl_vertex_t),
                                    (GLvoid*)offsetof(pgl_vertex_t, pos)));

    PGL_CHECK(glEnableVertexAttribArray(0));

    // Color
    PGL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                                    sizeof(pgl_vertex_t),
                                    (GLvoid*)offsetof(pgl_vertex_t, color)));

    PGL_CHECK(glEnableVertexAttribArray(1));

    // UV
    PGL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                                    sizeof(pgl_vertex_t),
                                    (GLvoid*)offsetof(pgl_vertex_t, uv)));

    PGL_CHECK(glEnableVertexAttribArray(2));

}

static void pgl_log(const char* fmt, ...)
{
    PGL_ASSERT(fmt);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}

static void pgl_log_error(const char* file, unsigned line, const char* expr)
{
    PGL_ASSERT(file);

    pgl_error_t code = pgl_map_error(glGetError());

    if (PGL_NO_ERROR == code)
        return;

    PGL_LOG("GL error: file: %s, line: %u, msg: \"%s\", expr: \"%s\"",
    file, line, pgl_error_msg_map[code], expr);
}

static pgl_error_t pgl_map_error(GLenum id)
{
    switch (id)
    {
        case GL_NO_ERROR:          return PGL_NO_ERROR;
        case GL_INVALID_ENUM:      return PGL_INVALID_ENUM;
        case GL_INVALID_VALUE:     return PGL_INVALID_VALUE;
        case GL_INVALID_OPERATION: return PGL_INVALID_OPERATION;
        case GL_OUT_OF_MEMORY:     return PGL_OUT_OF_MEMORY;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
        return PGL_INVALID_FRAMEBUFFER_OPERATION;
    }

    return PGL_UNKNOWN_ERROR;
}
static bool pgl_str_equal(const char* str1, const char* str2)
{
    return (0 == strcmp(str1, str2));
}

static bool pgl_mem_equal(const void* ptr1, const void* ptr2, size_t size)
{
    return (0 == memcmp(ptr1, ptr2, size));
}

// FNV-1a
static pgl_hash_t pgl_hash_str(const char* str)
{
    PGL_ASSERT(str);

    pgl_hash_t hash = PGL_OFFSET_BASIS;

    while ('\0' != str[0])
    {
        hash ^= (pgl_hash_t)str[0];
        hash *= PGL_PRIME;
        str++;
    }

    return hash;
}

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif // __GNUC__

#include "glad.c"

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif // __GNUC__

#endif // PICO_GL_IMPLEMENTATION

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

