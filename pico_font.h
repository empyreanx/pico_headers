/**
    @file pico_font.h
    @brief A simple, dynamic (online) font atlas

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Introduction:
    -------------

    A single-header library that rasterizes and caches glyphs on demand into a
    multi-page texture atlas. Each page is a single-channel (alpha) bitmap with
    a fixed width and a height that grows on demand up to a configurable maximum.
    When a page is full, a new page is appended automatically. Glyph placement
    uses row-based shelf packing. The library also supports basic, single line
    drawing, but can be customized using the available metrics and measurement
    functions.

    Features:
    ---------
    - Written in C99
    - On-demand glyph rasterization via stb_truetype.h
    - Multi-page atlas with automatic page growth and creation
    - Hashtable glyph cache with automatic rehashing
    - UTF-8 text layout with kerning (rendering API agnostic)
    - Text measurement and font metrics
    - Dirty-page tracking for efficient GPU uploads

    Revision History:
    -----------------

    - 0.1 (2026/04/11):
        - On-demand glyph rasterization and storage
        - Simple text rendering

    Usage:
    ------

        #define PICO_FONT_IMPLEMENTATION
        #include "pico_font.h"

    You must also have stb_truetype.h available. Define STB_TRUETYPE_IMPLEMENTATION
    exactly once in your project after including this file.

    Quick Start:
    ------------

    ```c
    pf_atlas_t* atlas = pf_create_atlas(512, 512);   // page width, max page height
    pf_face_t*  face  = pf_create_face(atlas, ttf_data, 32.0f);  // 32px height

    // During rendering (call every frame for each string):
    float x = 10, y = 50;
    pf_draw_text(face, "Hello!", &x, &y, my_quad_callback, user_ptr);

    // The callback receives screen-space quads with atlas UVs and page index.
    // After pf_draw_text, upload any dirty atlas pages to the GPU:
    pf_upload_atlas(atlas, my_upload_callback, user_ptr);

    pf_destroy_face(face);
    pf_destroy_atlas(atlas);
    ```

    Full Example:
    -------------

    A more or less complete example can be found at: examples_pico_gfx/text.c

    Macro Overrides:
    ----------------

    - PICO_FONT_ASSERT  (default: assert)
    - PICO_FONT_CALLOC  (default: calloc)
    - PICO_FONT_REALLOC (default: realloc)
    - PICO_FONT_FRE     (default: free)
    - PICO_FONT_MEMSET  (default: memset)

    Constant Overrides:
    -------------------

    - PICO_FONT_GLYPH_PADDING    (default: 1)
    - PICO_FONT_CACHE_INIT_SIZE  (default: 256)
    - PICO_FONT_INIT_PAGE_HEIGHT (default: 64)
*/

#ifndef PICO_FONT_H
#define PICO_FONT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ---- Types ------------------------------------------------------------------

// Opaque handles
typedef struct pf_atlas_t pf_atlas_t;
typedef struct pf_face_t  pf_face_t;

/**
 * @brief UV coordinates and metrics for a cached glyph.
 */
typedef struct
{
    uint32_t codepoint;   // Unicode codepoint
    float    size;        // Font pixel height used when rasterizing
    int      glyph_index; // stbtt glyph index (0 = missing glyph)

    // Position inside page (pixels)
    int page_x, page_y;
    int page_w, page_h;

    // Atlas page index
    size_t page;

    // Offset from cursor to top-left of bitmap
    int offset_x, offset_y;

    // Horizontal advance in pixels (scaled)
    float advance_x;

    // UV corners (computed after placement)
    float u0, v0, u1, v1;
} pf_glyph_t;

/**
 * @brief Quad emitted by pf_draw_text
 */
typedef struct
{
    float x0, y0, x1, y1; // screen-space rectangle
    float u0, v0, u1, v1; // atlas UVs
    size_t page;          // atlas page index
} pf_quad_t;

/**
 * @brief Vertical font metrics for a face.
 */
typedef struct
{
    float ascent;      //!< Distance from baseline to top of tallest glyph.
    float descent;     //!< Distance from baseline to bottom (typically negative).
    float line_gap;    //!< Extra spacing between lines.
    float line_height; //!< Recommended line advance (ascent - descent + line_gap).
} pf_metrics_t;

/**
 * @brief Callback invoked for each glyph quad during text drawing.
 *
 * Return false to stop iteration, true to continue.
 */
typedef bool (*pf_draw_callback_fn)(const pf_quad_t* quad, void* user);

/**
 * @brief Callback invoked for each dirty atlas page during pf_upload_atlas.
 * @param page  page index
 * @param pixels single-channel bitmap (width * height bytes)
 * @param width, bitmap horizontal dimensions
 * @param height bitmap vertical dimensions
 * @return false to stop iteration, true to continue. The dirty flag is cleared
 * automatically after a successful (true) return.
 */
typedef bool (*pf_upload_callback_fn)(size_t page, const unsigned char* pixels,
                                      int width, int height, void* user);

// ---- API --------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocates and initializes a font atlas.
 *
 * Each atlas is composed of one or more bitmap pages that store rasterized
 * glyphs and their associated metadata. Pages start empty and grow vertically
 * (doubling in height) as glyphs are added, up to @p max_page_height. Once a
 * page is full, a new page is appended automatically.
 *
 * @param page_width       Width of each atlas page in pixels.
 * @param max_page_height  Maximum height each page may grow to in pixels.
 * @return A new atlas, or NULL on allocation failure.
 */
pf_atlas_t* pf_create_atlas(int page_width, int max_page_height);

/**
 * @brief Destroys a font atlas and frees all associated pages and glyphs.
 *
 * @param atlas  Atlas to destroy. May be NULL (no-op).
 */
void pf_destroy_atlas(pf_atlas_t* atlas);

/**
 * @brief Create a font face at a given pixel height.
 *
 * Initialises an stb_truetype font from raw TrueType data and binds it to
 * @p atlas for glyph caching. The face does not take ownership of
 * @p ttf_data; the caller must keep it alive for the lifetime of the face.
 *
 * @param atlas         Atlas that will store rasterized glyphs.
 * @param ttf_data      Pointer to the raw TrueType font file contents.
 * @param pixel_height  Desired font height in pixels.
 * @return A new face, or NULL on failure.
 */
pf_face_t* pf_create_face(pf_atlas_t* atlas,
                          const unsigned char* ttf_data,
                          float pixel_height);

/**
 * @brief Destroys a font face.
 *
 * Does not destroy the atlas it was bound to.
 *
 * @param face  Face to destroy. May be NULL (no-op).
 */
void pf_destroy_face(pf_face_t* face);

/**
 * @brief Get (or rasterize) a single glyph.
 *
 * If the glyph is already cached, returns it immediately. Otherwise it is
 * rasterized into the atlas and cached for future lookups.
 *
 * @param face       Face to use for rasterization.
 * @param codepoint  Unicode codepoint.
 * @return Pointer into the atlas glyph array, or NULL on failure.
 */
const pf_glyph_t* pf_get_glyph(pf_face_t* face, uint32_t codepoint);

/**
 * @brief Lay out and emit quads for a UTF-8 string.
 *
 * For each visible glyph the callback receives a screen-space quad with atlas
 * UVs and a page index. Kerning is applied automatically between adjacent
 * codepoints. The caller is responsible for line breaking; use pf_get_metrics()
 * to obtain the line height for advancing *y between lines.
 *
 * @param face  Face to use.
 * @param text  Null-terminated UTF-8 string.
 * @param x     In/out cursor X position. Updated after the last glyph.
 * @param y     In/out cursor Y position. Updated after the last glyph.
 * @param cb    Callback invoked for each visible glyph quad. May be NULL.
 * @param user  Opaque pointer forwarded to @p cb.
 */
void pf_draw_text(pf_face_t* face, const char* text,
                  float* x, float* y,
                  pf_draw_callback_fn cb, void* user);

/**
 * @brief Iterate over dirty pages and invoke the callback for each one.
 *
 * A page becomes dirty whenever a new glyph is rasterized into it. The dirty
 * flag is cleared for each page whose callback returns non-zero, so calling
 * this after drawing ensures the GPU copy stays in sync.
 *
 * @param atlas  The atlas to upload.
 * @param cb     Callback invoked once per dirty page.
 * @param user   Opaque pointer forwarded to @p cb.
 */
void pf_upload_atlas(pf_atlas_t* atlas, pf_upload_callback_fn cb, void* user);

/**
 * @brief Measure a UTF-8 string without drawing.
 *
 * Performs the same layout as pf_draw_text but does not emit quads.
 *
 * @param face        Face to use.
 * @param text        Null-terminated UTF-8 string.
 * @param out_width   Receives the bounding-box width. May be NULL.
 * @param out_height  Receives the bounding-box height. May be NULL.
 */
void pf_measure_text(pf_face_t* face, const char* text,
                     float* out_width, float* out_height);

/**
 * @brief Retrieve vertical font metrics for a face.
 *
 * @param face     Face to query.
 * @param metrics  Receives the metrics. Must not be NULL.
 */
void pf_get_metrics(const pf_face_t* face, pf_metrics_t* metrics);

/**
 * @brief Get the horizontal kerning adjustment between two codepoints.
 *
 * @param face  Face to query.
 * @param cp1   Left codepoint.
 * @param cp2   Right codepoint.
 * @return Kerning offset in pixels (typically negative for tighter pairs).
 */
float pf_get_kerning(const pf_face_t* face, uint32_t cp1, uint32_t cp2);

#ifdef __cplusplus
}
#endif

#endif // PICO_FONT_H

// ---- Implementation ---------------------------------------------------------

#ifdef PICO_FONT_IMPLEMENTATION

// Padding around each glyph in the atlas (pixels).
#ifndef PICO_FONT_GLYPH_PADDING
#define PICO_FONT_GLYPH_PADDING 1
#endif

// Initial hash table size for the glyph cache. Must be power of two.
#ifndef PICO_FONT_CACHE_INIT_SIZE
#define PICO_FONT_CACHE_INIT_SIZE 256
#endif

// Initial height for newly created atlas pages. Set to 0 to defer allocation.
#ifndef PICO_FONT_INIT_PAGE_HEIGHT
#define PICO_FONT_INIT_PAGE_HEIGHT 64
#endif

#ifdef NDEBUG
    #define PICO_FONT_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_FONT_ASSERT
        #include <assert.h>
        #define PICO_FONT_ASSERT(expr) (assert(expr))
    #endif
#endif

#if !defined(PICO_FONT_CALLOC)  || \
    !defined(PICO_FONT_REALLOC) || \
    !defined(PICO_FONT_FREE)
    #include <stdlib.h>
    #define PICO_FONT_CALLOC(num, size)  (calloc(num, size))
    #define PICO_FONT_REALLOC(ptr, size) (realloc(ptr, size))
    #define PICO_FONT_FREE(ptr)          (free(ptr))
#endif

#ifndef PICO_FONT_MEMSET
    #include <string.h>
    #define PICO_FONT_MEMSET memset
#endif

// Sentinel value returned by internal functions to signal failure.
#define PICO_FONT_ERROR ((size_t)-1)

#include "stb_truetype.h"

// ---- Internal types ---------------------------------------------------------

// Hash-table entry for glyph cache (open addressing, linear probe).
typedef struct
{
    uint32_t key;         // hash key; 0 = empty slot
    size_t   glyph_index; // index into pf_atlas_t.glyphs[]
} pf_cache_entry_t;

// Shelf-based packing state.
typedef struct
{
    int cursor_x;     // next free x in current shelf
    int cursor_y;     // top of current shelf
    int shelf_height; // height of current shelf row
} pf_shelf_t;

// A single page in the atlas. Each page has its own bitmap.
typedef struct
{
    unsigned char* pixels;
    int            width, height;
    bool           dirty; // set to true whenever pixels are modified
    pf_shelf_t     shelf;
} pf_atlas_page_t;

// Full atlas definition (opaque to users).
struct pf_atlas_t
{
    // page storage (dynamic array of pages)
    pf_atlas_page_t* pages;
    size_t           page_count;
    size_t           page_capacity;
    int              page_width;      // fixed width for all pages
    int              max_page_height; // maximum height a page may grow to

    // glyph storage (dynamic array)
    pf_glyph_t* glyphs;
    size_t      glyph_count;
    size_t      glyph_capacity;

    // hash table for fast lookup
    pf_cache_entry_t* cache;
    size_t            cache_size; // always power of two
};

// Full face definition (opaque to users).
struct pf_face_t
{
    stbtt_fontinfo info;
    float          size;     // requested pixel height
    float          scale;    // stbtt scale factor
    int            ascent;   // scaled ascent in pixels
    int            descent;
    int            line_gap;
    pf_atlas_t*    atlas;
};

// Measure callback state.
typedef struct
{
    float max_x;
    float max_y;
} pf_measure_state_t;

// ---- Forward declarations ---------------------------------------------------

// Compute a hash key from a codepoint and font size.
static uint32_t pf_hash_key(uint32_t cp, float size);

// Look up a glyph index in the cache by hash key.
static size_t pf_cache_lookup(const pf_atlas_t* atlas, uint32_t key);

// Insert a key/glyph-index pair into a cache table without growing.
static void pf_cache_insert_raw(pf_cache_entry_t* cache, size_t cache_size,
                                uint32_t key, size_t glyph_index);

// Insert a key/glyph-index pair, growing the cache if needed.
static int pf_cache_insert(pf_atlas_t* atlas, uint32_t key,
                            size_t glyph_index);

// Append a new page to the atlas and return its index.
static size_t pf_atlas_add_page(pf_atlas_t* atlas);

// Try to allocate a rectangle on a single atlas page using shelf packing.
static int pf_page_alloc(pf_atlas_page_t* page, int w, int h,
                         int* out_x, int* out_y);

// Grow a page's pixel buffer vertically up to max_height.
static int pf_page_grow(pf_atlas_page_t* page, int needed_height,
                        int max_height);

// Recompute v0/v1 for all glyphs on a page after it grows.
static void pf_page_recompute_uvs(pf_atlas_t* atlas, size_t page_index);

// Allocate a rectangle in the atlas, adding a new page if necessary.
static int pf_atlas_alloc(pf_atlas_t* atlas, int w, int h,
                          int* out_x, int* out_y, size_t* out_page);

// Append a glyph to the atlas glyph array and return its index.
static size_t pf_glyph_push(pf_atlas_t* atlas, const pf_glyph_t* g);

// Decode one UTF-8 codepoint from a string, advancing the pointer.
static uint32_t pf_utf8_decode(const char** str);

// Iterate over a UTF-8 string, resolving glyphs and emitting quads.
static void pf_walk_text(pf_face_t* face, const char* text,
                         float* x, float* y,
                         pf_draw_callback_fn cb, void* user);

// Quad callback used by pf_measure_text to track bounding box extents.
static bool pf_measure_cb(const pf_quad_t* quad, void* user);

// ---- Public API -------------------------------------------------------------

pf_atlas_t* pf_create_atlas(int page_width, int max_page_height)
{
    PICO_FONT_ASSERT(page_width > 0);
    PICO_FONT_ASSERT(max_page_height > 0);

    pf_atlas_t* atlas = (pf_atlas_t*)PICO_FONT_CALLOC(1, sizeof(pf_atlas_t));

    if (!atlas)
        return NULL;

    atlas->page_width      = page_width;
    atlas->max_page_height = max_page_height;

    atlas->cache_size = PICO_FONT_CACHE_INIT_SIZE;
    atlas->cache = (pf_cache_entry_t*)PICO_FONT_CALLOC(atlas->cache_size,
        sizeof(pf_cache_entry_t));

    if (!atlas->cache)
    {
        PICO_FONT_FREE(atlas);
        return NULL;
    }

    // Create the first page.
    if (pf_atlas_add_page(atlas) == PICO_FONT_ERROR)
    {
        PICO_FONT_FREE(atlas->cache);
        PICO_FONT_FREE(atlas);
        return NULL;
    }

    return atlas;
}

void pf_destroy_atlas(pf_atlas_t* atlas)
{
    if (!atlas)
        return;

    for (size_t i = 0; i < atlas->page_count; i++)
    {
        PICO_FONT_FREE(atlas->pages[i].pixels);
    }

    PICO_FONT_FREE(atlas->pages);
    PICO_FONT_FREE(atlas->glyphs);
    PICO_FONT_FREE(atlas->cache);
    PICO_FONT_FREE(atlas);
}

pf_face_t* pf_create_face(pf_atlas_t* atlas,
                          const unsigned char* ttf_data,
                          float pixel_height)
{
    PICO_FONT_ASSERT(atlas != NULL);
    PICO_FONT_ASSERT(ttf_data != NULL);
    PICO_FONT_ASSERT(pixel_height > 0.0f);

    pf_face_t* face = (pf_face_t*)PICO_FONT_CALLOC(1, sizeof(pf_face_t));

    if (!face)
        return NULL;

    face->atlas = atlas;
    face->size  = pixel_height;

    int offset = stbtt_GetFontOffsetForIndex(ttf_data, 0);
    if (offset < 0)
    {
        PICO_FONT_FREE(face);
        return NULL;
    }

    if (!stbtt_InitFont(&face->info, ttf_data, offset))
    {
        PICO_FONT_FREE(face);
        return NULL;
    }

    face->scale = stbtt_ScaleForPixelHeight(&face->info, pixel_height);

    int ascent, descent, gap;
    stbtt_GetFontVMetrics(&face->info, &ascent, &descent, &gap);
    face->ascent   = (int)(ascent  * face->scale + 0.5f);
    face->descent  = (int)(descent * face->scale - 0.5f);
    face->line_gap = (int)(gap     * face->scale + 0.5f);
    return face;
}

void pf_destroy_face(pf_face_t* face)
{
    if (!face)
        return;

    PICO_FONT_FREE(face);
}

const pf_glyph_t* pf_get_glyph(pf_face_t* face, uint32_t codepoint)
{
    PICO_FONT_ASSERT(face != NULL);

    pf_atlas_t* atlas = face->atlas;
    uint32_t key = pf_hash_key(codepoint, face->size);

    // Check cache first.
    size_t cached = pf_cache_lookup(atlas, key);
    if (cached != PICO_FONT_ERROR)
    {
        // Verify it's actually the right glyph (hash collision check).
        pf_glyph_t* glyph = &atlas->glyphs[cached];

        if (glyph->codepoint == codepoint && glyph->size == face->size)
        {
            return glyph;
        }
        /*
         * Collision: fall through to rasterize (rare). For simplicity we
         * linear-probe for a matching glyph in the table. A full collision
         * resolution would use a secondary key; this is acceptable for the
         * typical glyph counts involved.
         */
    }

    // Rasterize the glyph.
    int glyph_index = stbtt_FindGlyphIndex(&face->info, (int)codepoint);

    int advance_raw, lsb;
    stbtt_GetGlyphHMetrics(&face->info, glyph_index, &advance_raw, &lsb);

    pf_glyph_t glyph  = { 0 };
    glyph.codepoint   = codepoint;
    glyph.size        = face->size;
    glyph.glyph_index = glyph_index;
    glyph.advance_x   = advance_raw * face->scale;

    // Empty glyphs (space, control chars).
    if (stbtt_IsGlyphEmpty(&face->info, glyph_index))
    {
        glyph.page_w = 0;
        glyph.page_h = 0;

        size_t index = pf_glyph_push(atlas, &glyph);
        if (index == PICO_FONT_ERROR)
            return NULL;

        pf_cache_insert(atlas, key, index);
        return &atlas->glyphs[index];
    }

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(&face->info, glyph_index,
                            face->scale, face->scale,
                            &x0, &y0, &x1, &y1);
    int bw = x1 - x0;
    int bh = y1 - y0;

    if (bw <= 0 || bh <= 0)
    {
        size_t index = pf_glyph_push(atlas, &glyph);

        if (index == PICO_FONT_ERROR)
            return NULL;

        pf_cache_insert(atlas, key, index);
        return &atlas->glyphs[index];
    }

    // Allocate space in atlas.
    int ax, ay;
    size_t ap;
    if (pf_atlas_alloc(atlas, bw, bh, &ax, &ay, &ap) != 0)
    {
        return NULL; // atlas is completely full
    }

    // Render into the page's bitmap.
    pf_atlas_page_t* page = &atlas->pages[ap];
    stbtt_MakeGlyphBitmap(&face->info,
                          page->pixels + ay * page->width + ax,
                          bw, bh, page->width,
                          face->scale, face->scale,
                          glyph_index);
    page->dirty = true;

    glyph.page_x   = ax;
    glyph.page_y   = ay;
    glyph.page_w   = bw;
    glyph.page_h   = bh;
    glyph.page     = ap;
    glyph.offset_x = x0;
    glyph.offset_y = y0;

    float inv_w = 1.0f / (float)page->width;
    float inv_h = 1.0f / (float)page->height;

    glyph.u0 = (float)ax * inv_w;
    glyph.v0 = (float)ay * inv_h;
    glyph.u1 = (float)(ax + bw) * inv_w;
    glyph.v1 = (float)(ay + bh) * inv_h;

    size_t index = pf_glyph_push(atlas, &glyph);

    if (index == PICO_FONT_ERROR)
        return NULL;

    pf_cache_insert(atlas, key, index);
    return &atlas->glyphs[index];
}

void pf_draw_text(pf_face_t* face, const char* text,
                  float* x, float* y,
                  pf_draw_callback_fn cb, void* user)
{
    PICO_FONT_ASSERT(face != NULL);
    PICO_FONT_ASSERT(x != NULL);
    PICO_FONT_ASSERT(y != NULL);

    if (!text)
        return;

    pf_walk_text(face, text, x, y, cb, user);
}

void pf_upload_atlas(pf_atlas_t* atlas, pf_upload_callback_fn cb, void* user)
{
    if (!atlas || !cb)
        return;

    for (size_t i = 0; i < atlas->page_count; i++)
    {
        pf_atlas_page_t* page = &atlas->pages[i];

        if (!page->dirty)
            continue;

        if (!cb(i, page->pixels, page->width, page->height, user))
        {
            return;
        }

        page->dirty = false;
    }
}

void pf_measure_text(pf_face_t* face, const char* text,
                     float* out_width, float* out_height)
{
    PICO_FONT_ASSERT(face != NULL);

    if (!text)
    {
        if (out_width)
            *out_width  = 0;

        if (out_height)
            *out_height = 0;

        return;
    }

    float x = 0, y = 0;
    pf_measure_state_t state = { 0, 0 };
    pf_walk_text(face, text, &x, &y, pf_measure_cb, &state);

    // Account for trailing spaces by checking cursor.
    if (x > state.max_x)
        state.max_x = x;

    float line_height = (float)(face->ascent - face->descent + face->line_gap);

    if (out_width)
        *out_width  = state.max_x;

    if (out_height)
        *out_height = (state.max_x > 0) ? line_height : 0;
}

void pf_get_metrics(const pf_face_t* face, pf_metrics_t* metrics)
{
    PICO_FONT_ASSERT(face != NULL);
    PICO_FONT_ASSERT(metrics != NULL);

    metrics->ascent      = (float)face->ascent;
    metrics->descent     = (float)face->descent;
    metrics->line_gap    = (float)face->line_gap;
    metrics->line_height = (float)(face->ascent - face->descent + face->line_gap);
}

float pf_get_kerning(const pf_face_t* face, uint32_t cp1, uint32_t cp2)
{
    PICO_FONT_ASSERT(face != NULL);

    int g1 = stbtt_FindGlyphIndex(&face->info, (int)cp1);
    int g2 = stbtt_FindGlyphIndex(&face->info, (int)cp2);

    return stbtt_GetGlyphKernAdvance(&face->info, g1, g2) * face->scale;
}

// ---- Internal helpers -------------------------------------------------------

static uint32_t pf_hash_key(uint32_t cp, float size)
{
    uint32_t x = cp ^ (uint32_t)(size * 100.f);
    x = ((x >> 16)  ^ x) * 0x45d9f3b;
    x = ((x >> 16)  ^ x) * 0x45d9f3b;
    x = (x >> 16)   ^ x;
    return x;
}

static size_t pf_cache_lookup(const pf_atlas_t* atlas, uint32_t key)
{
    size_t mask = atlas->cache_size - 1;
    size_t index  = (size_t)(key) & mask;

    for (size_t i = 0; i < atlas->cache_size; i++)
    {
        size_t slot = (index + i) & mask;

        if (atlas->cache[slot].key == key)
            return atlas->cache[slot].glyph_index;

        if (atlas->cache[slot].key == 0)
            return PICO_FONT_ERROR; // empty slot → not found
    }

    return PICO_FONT_ERROR;
}

static void pf_cache_insert_raw(pf_cache_entry_t* cache, size_t cache_size,
                                uint32_t key, size_t glyph_index)
{
    PICO_FONT_ASSERT(key != 0);

    size_t mask = cache_size - 1;
    size_t index  = (size_t)(key) & mask;

    for (size_t i = 0; i < cache_size; i++)
    {
        size_t slot = (index + i) & mask;

        if (cache[slot].key == 0)
        {
            cache[slot].key         = key;
            cache[slot].glyph_index = glyph_index;
            return;
        }
    }
    // Should never happen if load factor is kept in check.
    PICO_FONT_ASSERT(false);
}

static int pf_cache_insert(pf_atlas_t* atlas, uint32_t key, size_t glyph_index)
{
    // Grow if load factor > 0.7
    size_t used = atlas->glyph_count; // approximate

    if (used * 10 > atlas->cache_size * 7)
    {
        size_t new_size = atlas->cache_size * 2;

        pf_cache_entry_t* new_cache = (pf_cache_entry_t*)PICO_FONT_CALLOC(new_size,
            sizeof(pf_cache_entry_t));

        if (!new_cache)
            return -1;

        // rehash
        for (size_t i = 0; i < atlas->cache_size; i++)
        {
            if (atlas->cache[i].key != 0)
            {
                pf_cache_insert_raw(new_cache, new_size,
                                    atlas->cache[i].key,
                                    atlas->cache[i].glyph_index);
            }
        }

        PICO_FONT_FREE(atlas->cache);

        atlas->cache      = new_cache;
        atlas->cache_size = new_size;
    }

    pf_cache_insert_raw(atlas->cache, atlas->cache_size, key, glyph_index);

    return 0;
}

// Add a new page to the atlas. Returns the page index or PICO_FONT_ERROR on failure.
static size_t pf_atlas_add_page(pf_atlas_t* atlas)
{
    if (atlas->page_count >= atlas->page_capacity)
    {
        size_t new_capacity = atlas->page_capacity ? atlas->page_capacity * 2 : 4;

        pf_atlas_page_t* new_array = (pf_atlas_page_t*)PICO_FONT_REALLOC(atlas->pages,
            new_capacity * sizeof(pf_atlas_page_t));

        if (!new_array)
            return PICO_FONT_ERROR;

        atlas->pages         = new_array;
        atlas->page_capacity = new_capacity;
    }

    size_t index = atlas->page_count;

    pf_atlas_page_t* page = &atlas->pages[index];
    PICO_FONT_MEMSET(page, 0, sizeof(*page));
    page->width  = atlas->page_width;
    page->height = 0;

    if (pf_page_grow(page, PICO_FONT_INIT_PAGE_HEIGHT,
                     atlas->max_page_height) != 0)
    {
        return PICO_FONT_ERROR;
    }

    atlas->page_count++;

    return index;
}

// Try to allocate a rectangle on a specific page. Returns 0 on success.
static int pf_page_alloc(pf_atlas_page_t* page, int w, int h,
                         int* out_x, int* out_y)
{
    int pad = PICO_FONT_GLYPH_PADDING;
    int pw  = w + pad;
    int ph  = h + pad;

    // Try current shelf.
    if (page->shelf.cursor_x + pw <= page->width &&
        page->shelf.cursor_y + ph <= page->height)
    {
        if (ph > page->shelf.shelf_height)
            page->shelf.shelf_height = ph;

        *out_x = page->shelf.cursor_x;
        *out_y = page->shelf.cursor_y;
        page->shelf.cursor_x += pw;

        return 0;
    }

    // Start a new shelf.
    page->shelf.cursor_x = 0;
    page->shelf.cursor_y += page->shelf.shelf_height;
    page->shelf.shelf_height = 0;

    if (page->shelf.cursor_x + pw <= page->width &&
        page->shelf.cursor_y + ph <= page->height)
    {
        page->shelf.shelf_height = ph;

        *out_x = page->shelf.cursor_x;
        *out_y = page->shelf.cursor_y;
        page->shelf.cursor_x += pw;

        return 0;
    }

    return -1; // page is full
}

/*
 * Try to grow a page's pixel buffer vertically.  Doubles the current height
 * (starting from 1 when height == 0) until it reaches at least needed_height,
 * capped at max_height.  Returns 0 on success.
 */
static int pf_page_grow(pf_atlas_page_t* page, int needed_height, int max_height)
{
    PICO_FONT_ASSERT(page != NULL);
    PICO_FONT_ASSERT(max_height > 0);

    if (needed_height <= page->height)
        return 0;

    if (needed_height > max_height)
        return -1;

    int new_height = page->height > 0 ? page->height : 1;
    while (new_height < needed_height)
    {
        new_height *= 2;
    }

    if (new_height > max_height)
        new_height = max_height;

    unsigned char* new_pixels = (unsigned char*)PICO_FONT_REALLOC(page->pixels,
        (size_t)page->width * (size_t)new_height);

    if (!new_pixels)
        return -1;

    // Zero the newly added rows.
    PICO_FONT_MEMSET(new_pixels + (size_t)page->width * (size_t)page->height, 0,
        (size_t)page->width *
        (size_t)(new_height - page->height));

    page->pixels = new_pixels;
    page->height = new_height;
    page->dirty  = true;

    return 0;
}

// Recompute v0/v1 for every glyph on the given page using its current height.
static void pf_page_recompute_uvs(pf_atlas_t* atlas, size_t page_index)
{
    PICO_FONT_ASSERT(page_index < atlas->page_count);
    PICO_FONT_ASSERT(atlas->pages[page_index].height > 0);

    float inv_h = 1.0f / (float)atlas->pages[page_index].height;

    for (size_t i = 0; i < atlas->glyph_count; i++)
    {
        pf_glyph_t* g = &atlas->glyphs[i];

        if (g->page != page_index)
            continue;

        g->v0 = (float)g->page_y * inv_h;
        g->v1 = (float)(g->page_y + g->page_h) * inv_h;
    }
}

/*
 * Allocate a rectangle in the atlas.  Tries the current page, growing it
 * vertically if needed, and only adds a new page as a last resort.
 * Returns 0 on success.
 */
static int pf_atlas_alloc(pf_atlas_t* atlas, int w, int h,
                          int* out_x, int* out_y, size_t* out_page)
{
    int pad = PICO_FONT_GLYPH_PADDING;
    int ph  = h + pad;

    // Try the last (current) page.
    if (atlas->page_count > 0)
    {
        size_t page_index = atlas->page_count - 1;
        pf_atlas_page_t* page = &atlas->pages[page_index];

        if (pf_page_alloc(page, w, h, out_x, out_y) == 0)
        {
            *out_page = page_index;
            return 0;
        }

        // Shelf packing failed -- try growing the page height.
        int needed = page->shelf.cursor_y + page->shelf.shelf_height + ph;

        if (pf_page_grow(page, needed, atlas->max_page_height) == 0)
        {
            pf_page_recompute_uvs(atlas, page_index);

            if (pf_page_alloc(page, w, h, out_x, out_y) == 0)
            {
                *out_page = page_index;
                return 0;
            }
        }
    }

    // Current page is at max height -- add a new one.
    size_t page_index = pf_atlas_add_page(atlas);

    if (page_index == PICO_FONT_ERROR)
        return -1;

    // Grow the fresh page to fit the glyph.
    pf_atlas_page_t* page = &atlas->pages[page_index];
    if (pf_page_grow(page, ph, atlas->max_page_height) != 0)
    {
        return -1;
    }

    if (pf_page_alloc(page, w, h, out_x, out_y) == 0)
    {
        *out_page = page_index;
        return 0;
    }

    return -1; // glyph too large for a single page
}

// Append a glyph to the dynamic array. Returns index or PICO_FONT_ERROR.
static size_t pf_glyph_push(pf_atlas_t* atlas, const pf_glyph_t* glyph)
{
    if (atlas->glyph_count >= atlas->glyph_capacity)
    {
        size_t new_capacity = atlas->glyph_capacity ? atlas->glyph_capacity * 2 : 64;

        pf_glyph_t* new_array = (pf_glyph_t*)PICO_FONT_REALLOC(atlas->glyphs,
            new_capacity * sizeof(pf_glyph_t));

        if (!new_array)
            return PICO_FONT_ERROR;

        atlas->glyphs         = new_array;
        atlas->glyph_capacity = new_capacity;
    }

    size_t index = atlas->glyph_count++;
    atlas->glyphs[index] = *glyph;

    return index;
}

// Decode one UTF-8 codepoint. Advances *str. Returns 0xFFFD on error.
static uint32_t pf_utf8_decode(const char** str)
{
    const unsigned char* s = (const unsigned char*)*str;
    uint32_t cp;
    int n;

    if (s[0] < 0x80)      { cp = s[0]; n = 1; }
    else if (s[0] < 0xC0) { cp = 0xFFFD; n = 1; }
    else if (s[0] < 0xE0) { cp = s[0] & 0x1F; n = 2; }
    else if (s[0] < 0xF0) { cp = s[0] & 0x0F; n = 3; }
    else if (s[0] < 0xF8) { cp = s[0] & 0x07; n = 4; }
    else                  { cp = 0xFFFD; n = 1; }

    for (int i = 1; i < n; i++)
    {
        if ((s[i] & 0xC0) != 0x80)
        {
            *str = (const char*)(s + i);
            return 0xFFFD;
        }
        cp = (cp << 6) | (s[i] & 0x3F);
    }

    // Reject overlong encodings and surrogates.
    if ((n == 2 && cp < 0x80)          ||
        (n == 3 && cp < 0x800)         ||
        (n == 4 && cp < 0x10000)       ||
        (cp >= 0xD800 && cp <= 0xDFFF) ||
         cp > 0x10FFFF)
    {
        cp = 0xFFFD;
    }

    *str = (const char*)(s + n);
    return cp;
}

// Internal: iterate over a UTF-8 string, resolve glyphs, and optionally
// emit quads via callback.  Used by both pf_draw_text and pf_measure_text.
static void pf_walk_text(pf_face_t* face, const char* text,
                         float* x, float* y,
                         pf_draw_callback_fn cb, void* user)
{
    const char* s = text;
    uint32_t prev_cp = 0;

    while (*s)
    {
        uint32_t cp = pf_utf8_decode(&s);

        if (cp == 0)
            break;

        if (prev_cp)
            *x += pf_get_kerning(face, prev_cp, cp);

        const pf_glyph_t* g = pf_get_glyph(face, cp);

        if (!g)
        {
            prev_cp = cp;
            continue;
        }

        if (g->page_w > 0 && g->page_h > 0 && cb)
        {
            pf_quad_t q = { 0 };

            q.x0 = *x +   (float)g->offset_x;
            q.y0 = *y +   (float)g->offset_y + (float)face->ascent;
            q.x1 = q.x0 + (float)g->page_w;
            q.y1 = q.y0 + (float)g->page_h;
            q.u0 = g->u0;
            q.v0 = g->v0;
            q.u1 = g->u1;
            q.v1 = g->v1;
            q.page = g->page;

            if (!cb(&q, user))
                return;
        }

        *x += g->advance_x;
        prev_cp = cp;
    }
}

static bool pf_measure_cb(const pf_quad_t* quad, void* user)
{
    pf_measure_state_t* st = (pf_measure_state_t*)user;

    if (quad->x1 > st->max_x)
        st->max_x = quad->x1;

    if (quad->y1 > st->max_y)
        st->max_y = quad->y1;

    return true;
}

#endif // PICO_FONT_IMPLEMENTATION

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2026 James McLean

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

// EoF
