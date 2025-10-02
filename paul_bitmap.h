/* paul/paul_bitmap.h -- https://github.com/takeiteasy/paul

 Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

 /*!
 @header paul_bitmap.h
 @copyright George Watson GPLv3
 @updated 2025-09-29
 @brief Bitmap manipulation library for C/C++.
 @discussion
    Implementation is included when PAUL_BITMAP_IMPLEMENTATION or PAUL_IMPLEMENTATION is defined.
*/

#ifndef PAUL_BITMAP_HEADER
#define PAUL_BITMAP_HEADER
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef PAUL_COLOR_HEADER
#ifndef __has_include
#define __has_include(x) 0
#endif
#if __has_include("paul_color.h")
#include "paul_color.h"
#else
/*! @typedef color_t
    @brief Represents a color with red, green, blue, and alpha components.
    @field r Red component (0-255)
    @field g Green component (0-255)
    @field b Blue component (0-255)
    @field a Alpha component (0-255)
    @field rgba Packed RGBA value as 32-bit integer
*/
typedef union color_t {
    struct {
        uint8_t r, g, b, a;
    };
    uint32_t rgba;
} color_t;
#endif
#endif

/*! @typedef bitmap_t
    @brief Represents an RGBA color with 8 bits per channel.
*/
typedef color_t* bitmap_t;

/*!
 * @function bitmap_empty
 * @brief Creates a new empty image filled with a specified color.
 * @discussion Allocates memory for a new image of the specified width and height, initializing all pixels to the given color. The returned image must be freed with bitmap_destroy() when no longer needed.
 * @param w The width of the image in pixels.
 * @param h The height of the image in pixels.
 * @param color The color to fill the image with.
 * @return A pointer to the new image, or NULL on allocation failure.
*/
bitmap_t bitmap_empty(unsigned int w, unsigned int h, color_t color);

/*!
 * @function bitmap_destroy
 * @brief Frees the memory associated with an image.
 * @discussion Releases all memory allocated for the image. The image pointer becomes invalid after this call and should not be used.
 * @param img The image to destroy.
*/
void bitmap_destroy(bitmap_t img);

/*!
 * @function bitmap_width
 * @brief Gets the width of an image in pixels.
 * @discussion Returns the width dimension of the image.
 * @param img The image to query.
 * @return The width of the image in pixels.
*/
int bitmap_width(const bitmap_t img);

/*!
 * @function bitmap_height
 * @brief Gets the height of an image in pixels.
 * @discussion Returns the height dimension of the image.
 * @param img The image to query.
 * @return The height of the image in pixels.
*/
int bitmap_height(const bitmap_t img);

/*!
 * @function bitmap_size
 * @brief Gets both width and height of an image.
 * @discussion Retrieves the dimensions of the image, storing them in the provided pointers. This is more efficient than calling bitmap_width() and bitmap_height() separately.
 * @param img The image to query.
 * @param w Pointer to store the width (can be NULL if not needed).
 * @param h Pointer to store the height (can be NULL if not needed).
 * @return true if successful, false if the image is invalid.
*/
bool bitmap_size(const bitmap_t img, int *w, int *h);

/*!
 * @function bitmap_pset
 * @brief Sets the color of a pixel at the specified coordinates.
 * @discussion Changes the color of the pixel at position (x, y). Coordinates are zero-based, with (0,0) being the top-left corner. If coordinates are out of bounds, the operation fails.
 * @param img The image to modify.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The new color for the pixel.
 * @return true if successful, false if coordinates are out of bounds or image is invalid.
*/
bool bitmap_pset(bitmap_t img, int x, int y, color_t color);

/*!
 * @function bitmap_pget
 * @brief Gets the color of a pixel at the specified coordinates.
 * @discussion Retrieves the color of the pixel at position (x, y). Coordinates are zero-based, with (0,0) being the top-left corner. If coordinates are out of bounds, returns a default color.
 * @param img The image to query.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @return The color of the pixel, or a default color if coordinates are out of bounds.
*/
color_t bitmap_pget(const bitmap_t img, int x, int y);

/*!
 * @function bitmap_fill
 * @brief Fills the entire image with a solid color.
 * @discussion Sets all pixels in the image to the specified color, effectively clearing the image to a uniform color.
 * @param img The image to fill.
 * @param color The color to fill the image with.
 * @return true if successful, false if the image is invalid.
*/
bool bitmap_fill(bitmap_t img, color_t color);

/*!
 * @function bitmap_flood
 * @brief Performs a flood fill starting from the specified coordinates.
 * @discussion Fills a contiguous region of pixels with the same color as the starting pixel with a new color. Uses 4-way connectivity (up, down, left, right).
 * @param img The image to modify.
 * @param x The x-coordinate of the starting pixel.
 * @param y The y-coordinate of the starting pixel.
 * @param color The new color to fill the region with.
 * @return true if successful, false if coordinates are out of bounds or image is invalid.
*/
bool bitmap_flood(bitmap_t img, int x, int y, color_t color);

/*!
 * @function bitmap_paste
 * @brief Pastes one image onto another at the specified position.
 * @discussion Copies all pixels from the source image onto the destination image starting at position (x, y). The source image is pasted over the destination without any alpha blending.
 * @param dst The destination image to paste onto.
 * @param src The source image to paste from.
 * @param x The x-coordinate in the destination where to paste.
 * @param y The y-coordinate in the destination where to paste.
 * @return true if successful, false if images are invalid or operation would exceed bounds.
*/
bool bitmap_paste(bitmap_t dst, const bitmap_t src, int x, int y);

/*!
 * @function bitmap_clipped_paste
 * @brief Pastes a rectangular region from one image to another.
 * @discussion Copies a rectangular region (rx, ry, rw, rh) from the source image onto the destination image at position (x, y). Only the specified region of the source is copied.
 * @param dst The destination image to paste onto.
 * @param src The source image to paste from.
 * @param x The x-coordinate in the destination where to paste.
 * @param y The y-coordinate in the destination where to paste.
 * @param rx The x-coordinate of the region in the source image.
 * @param ry The y-coordinate of the region in the source image.
 * @param rw The width of the region in the source image.
 * @param rh The height of the region in the source image.
 * @return true if successful, false if images are invalid or operation would exceed bounds.
*/
bool bitmap_clipped_paste(bitmap_t dst, const bitmap_t src, int x, int y, int rx, int ry, int rw, int rh);

/*!
 * @function bitmap_resize
 * @brief Resizes an image in-place to new dimensions.
 * @discussion Changes the size of the image to the specified width and height. The original image data is lost and replaced with resized content. Uses nearest-neighbor interpolation.
 * @param src Pointer to the image to resize (will be modified).
 * @param nw The new width for the image.
 * @param nh The new height for the image.
 * @return true if successful, false if allocation fails or image is invalid.
*/
bool bitmap_resize(bitmap_t *src, int nw, int nh);

/*!
 * @function bitmap_rotate
 * @brief Rotates an image in-place by the specified angle.
 * @discussion Rotates the image around its center by the given angle in degrees. Positive angles rotate clockwise. The image dimensions may change to accommodate the rotated content.
 * @param src Pointer to the image to rotate (will be modified).
 * @param angle The rotation angle in degrees.
 * @return true if successful, false if allocation fails or image is invalid.
*/
bool bitmap_rotate(bitmap_t *src, float angle);

/*!
 * @function bitmap_clip
 * @brief Clips a rectangular region from an image in-place.
 * @discussion Extracts a rectangular region from the image, making it the new image content. The original image is replaced with just the clipped region.
 * @param src Pointer to the image to clip (will be modified).
 * @param rx The x-coordinate of the region to keep.
 * @param ry The y-coordinate of the region to keep.
 * @param rw The width of the region to keep.
 * @param rh The height of the region to keep.
 * @return true if successful, false if region is invalid or image is invalid.
*/
bool bitmap_clip(bitmap_t *src, int rx, int ry, int rw, int rh);

/*!
 * @function bitmap_dupe
 * @brief Creates a duplicate copy of an image.
 * @discussion Allocates a new image with the same dimensions and pixel data as the source image. The returned image is independent and must be freed with bitmap_destroy() when no longer needed.
 * @param src The source image to duplicate.
 * @return A new image that is a copy of the source, or NULL on allocation failure.
*/
bitmap_t bitmap_dupe(bitmap_t src);

/*!
 * @function bitmap_resized
 * @brief Creates a resized copy of an image.
 * @discussion Allocates a new image with the specified dimensions and copies the source image content resized to fit. Uses nearest-neighbor interpolation. The original image remains unchanged.
 * @param src The source image to resize.
 * @param nw The width of the new image.
 * @param nh The height of the new image.
 * @return A new resized image, or NULL on allocation failure.
*/
bitmap_t bitmap_resized(bitmap_t src, int nw, int nh);

/*!
 * @function bitmap_rotated
 * @brief Creates a rotated copy of an image.
 * @discussion Allocates a new image and copies the source image content rotated by the specified angle. The original image remains unchanged.
 * @param src The source image to rotate.
 * @param angle The rotation angle in degrees.
 * @return A new rotated image, or NULL on allocation failure.
*/
bitmap_t bitmap_rotated(bitmap_t src, float angle);

/*!
 * @function bitmap_clipped
 * @brief Creates a new image from a rectangular region of another image.
 * @discussion Allocates a new image containing only the specified rectangular region from the source image. The original image remains unchanged.
 * @param src The source image to clip from.
 * @param rx The x-coordinate of the region in the source image.
 * @param ry The y-coordinate of the region in the source image.
 * @param rw The width of the region.
 * @param rh The height of the region.
 * @return A new image containing the clipped region, or NULL on allocation failure.
*/
bitmap_t bitmap_clipped(bitmap_t src, int rx, int ry, int rw, int rh);

/*!
 * @function bitmap_dominant_color
 * @brief Finds the most frequently occurring color in the image.
 * @discussion Analyzes all pixels in the image and returns the color that appears most often. Useful for generating color palettes or determining the primary color of an image.
 * @param img The image to analyze.
 * @return The dominant color in the image.
*/
color_t bitmap_dominant_color(const bitmap_t img);

/*!
 * @function bitmap_histogram
 * @brief Generates a histogram of color frequencies in the image.
 * @discussion Creates an array representing the frequency distribution of colors in the image. The returned array must be freed by the caller when no longer needed.
 * @param img The image to analyze.
 * @return An array of integers representing color frequencies, or NULL on failure.
*/
int* bitmap_histogram(const bitmap_t img);

/*!
 * @function bitmap_palette
 * @brief Extracts a color palette from the image.
 * @discussion Analyzes the image and extracts the specified number of most representative colors. The returned array contains the palette colors and must be freed by the caller.
 * @param img The image to analyze.
 * @param count The number of colors to extract for the palette.
 * @return An array of colors representing the extracted palette, or NULL on failure.
*/
color_t* bitmap_palette(const bitmap_t img, int count);

/*!
 * @function bitmap_draw_line
 * @brief Draws a line between two points.
 * @discussion Draws a straight line from (x0, y0) to (x1, y1) using the specified color. Uses Bresenham's line algorithm for efficient rasterization.
 * @param img The image to draw on.
 * @param x0 The x-coordinate of the starting point.
 * @param y0 The y-coordinate of the starting point.
 * @param x1 The x-coordinate of the ending point.
 * @param y1 The y-coordinate of the ending point.
 * @param color The color to draw the line with.
 * @return true if successful, false if the image is invalid.
*/
bool bitmap_draw_line(bitmap_t img, int x0, int y0, int x1, int y1, color_t color);

/*!
 * @function bitmap_draw_circle
 * @brief Draws a circle or filled circle.
 * @discussion Draws a circle centered at (xc, yc) with radius r. If fill is non-zero, the circle is filled; otherwise only the outline is drawn.
 * @param img The image to draw on.
 * @param xc The x-coordinate of the circle center.
 * @param yc The y-coordinate of the circle center.
 * @param r The radius of the circle.
 * @param color The color to draw the circle with.
 * @param fill Non-zero to fill the circle, zero for outline only.
 * @return true if successful, false if the image is invalid.
*/
bool bitmap_draw_circle(bitmap_t img, int xc, int yc, int r, color_t color, int fill);

/*!
 * @function bitmap_draw_rectangle
 * @brief Draws a rectangle or filled rectangle.
 * @discussion Draws a rectangle with top-left corner at (x, y) and dimensions w by h. If fill is non-zero, the rectangle is filled; otherwise only the outline is drawn.
 * @param img The image to draw on.
 * @param x The x-coordinate of the top-left corner.
 * @param y The y-coordinate of the top-left corner.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @param color The color to draw the rectangle with.
 * @param fill Non-zero to fill the rectangle, zero for outline only.
 * @return true if successful, false if the image is invalid.
*/
bool bitmap_draw_rectangle(bitmap_t img, int x, int y, int w, int h, color_t color, int fill);

/*!
 * @function bitmap_draw_triangle
 * @brief Draws a triangle or filled triangle.
 * @discussion Draws a triangle defined by three vertices (x0,y0), (x1,y1), (x2,y2). If fill is non-zero, the triangle is filled; otherwise only the outline is drawn.
 * @param img The image to draw on.
 * @param x0 The x-coordinate of the first vertex.
 * @param y0 The y-coordinate of the first vertex.
 * @param x1 The x-coordinate of the second vertex.
 * @param y1 The y-coordinate of the second vertex.
 * @param x2 The x-coordinate of the third vertex.
 * @param y2 The y-coordinate of the third vertex.
 * @param color The color to draw the triangle with.
 * @param fill Non-zero to fill the triangle, zero for outline only.
 * @return true if successful, false if the image is invalid.
*/
bool bitmap_draw_triangle(bitmap_t img, int x0, int y0, int x1, int y1, int x2, int y2, color_t color, int fill);

// Data format enumeration for bitmap_load function
/*!
 * @typedef bitmap_format_t
 * @brief Enumeration of supported pixel data formats for image loading.
 * @discussion Defines the various pixel formats that can be used when loading image data from raw pixel arrays. Each format specifies the byte layout and channel count of the pixel data.
 * @field BITMAP_FORMAT_RGBA 4 bytes per pixel: R, G, B, A
 * @field BITMAP_FORMAT_RGB 3 bytes per pixel: R, G, B (alpha = 255)
 * @field BITMAP_FORMAT_BGRA 4 bytes per pixel: B, G, R, A
 * @field BITMAP_FORMAT_BGR 3 bytes per pixel: B, G, R (alpha = 255)
 * @field BITMAP_FORMAT_ARGB 4 bytes per pixel: A, R, G, B
 * @field BITMAP_FORMAT_ABGR 4 bytes per pixel: A, B, G, R
 * @field BITMAP_FORMAT_GRAY 1 byte per pixel: grayscale (alpha = 255)
 * @field BITMAP_FORMAT_GRAY_ALPHA 2 bytes per pixel: grayscale, alpha
 * @field BITMAP_FORMAT_RGB565 2 bytes per pixel: 5-bit R, 6-bit G, 5-bit B (alpha = 255)
 * @field BITMAP_FORMAT_RGB555 2 bytes per pixel: 5-bit R, 5-bit G, 5-bit B (alpha = 255)
 * @field BITMAP_FORMAT_ARGB1555 2 bytes per pixel: 1-bit A, 5-bit R, 5-bit G, 5-bit B
*/
typedef enum {
    BITMAP_FORMAT_RGBA,          // 4 bytes per pixel: R, G, B, A
    BITMAP_FORMAT_RGB,           // 3 bytes per pixel: R, G, B (alpha = 255)
    BITMAP_FORMAT_BGRA,          // 4 bytes per pixel: B, G, R, A
    BITMAP_FORMAT_BGR,           // 3 bytes per pixel: B, G, R (alpha = 255)
    BITMAP_FORMAT_ARGB,          // 4 bytes per pixel: A, R, G, B
    BITMAP_FORMAT_ABGR,          // 4 bytes per pixel: A, B, G, R
    BITMAP_FORMAT_GRAY,          // 1 byte per pixel: grayscale (alpha = 255)
    BITMAP_FORMAT_GRAY_ALPHA,    // 2 bytes per pixel: grayscale, alpha
    BITMAP_FORMAT_RGB565,        // 2 bytes per pixel: 5-bit R, 6-bit G, 5-bit B (alpha = 255)
    BITMAP_FORMAT_RGB555,        // 2 bytes per pixel: 5-bit R, 5-bit G, 5-bit B (alpha = 255)
    BITMAP_FORMAT_ARGB1555       // 2 bytes per pixel: 1-bit A, 5-bit R, 5-bit G, 5-bit B
} bitmap_format_t;

/*!
 * @function bitmap_load
 * @brief Creates an image from raw pixel data in the specified format.
 * @discussion Loads pixel data from a raw byte array and creates a new image. The data must be in the specified format and contain width * height * bytes_per_pixel bytes. The returned image must be freed with bitmap_destroy() when no longer needed.
 * @param data Pointer to the raw pixel data.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param format The format of the pixel data (see bitmap_format_t).
 * @return A new image created from the pixel data, or NULL on failure.
*/
bitmap_t bitmap_load(const void* data, int width, int height, bitmap_format_t format);

#ifdef __cplusplus
}
#endif
#endif // PAUL_BITMAP_HEADER

#if defined(PAUL_BITMAP_IMPLEMENTATION) || defined(PAUL_IMPLEMENTATION)
#ifdef _WIN32
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _RGBA(R, G, B, A) (((unsigned int)(R) << 24) | ((unsigned int)(B) << 16) | ((unsigned int)(G) << 8) | (A))
#define _RADIANS(N) (((double)(N)) * (M_PI / 180.))
#define _MIN(A, B) ((A) < (B) ? (A) : (B))
#define _MAX(A, B) ((A) > (B) ? (A) : (B))
#define _CLAMP(x, low, high) _MIN(_MAX(x, low), high)
#define _SWAP(A, B) ((A)^=(B)^=(A)^=(B))
#define _CLAMP_UINT8(V) ((uint8_t)_CLAMP((V), 0, 255))
#define _CLAMP_FLOAT(V) ((float)_CLAMP((V), 0.0f, 1.0f))
#define _CLAMP_FLOAT_RANGE(V, _MINV, _MAXV) (fminf(fmaxf(V, _MINV), _MAXV))
#define _FMIN3(A, B, C) (fminf(fminf((A), (B)), (C)))
#define _FMAX3(A, B, C) (fmaxf(fmaxf((A), (B)), (C)))
static _CONSTEXPR color_t _black = { 0.0f, 0.0f, 0.0f, 1.0f };

color_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (color_t){ r, g, b, a };
}

color_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return rgba(r, g, b, 255);
}

color_t rgbaf(float r, float g, float b, float a) {
    return rgbaf_to_rgba((color_rgbaf_t){ r, g, b, a });
}
color_t rgbf(float r, float g, float b) {
    return rgbaf(r, g, b, 1.f);
}

color_t hsva(float h, float s, float v, float a) {
    return hsva_to_rgba((color_hsva_t){ h, s, v, a });
}

color_t hsv(float h, float s, float v) {
    return hsva(h, s, v, 1.f);
}

color_t hsla(float h, float s, float l, float a) {
    return hsla_to_rgba((color_hsla_t){ h, s, l, a });
}

color_t hsl(float h, float s, float l) {
    return hsla(h, s, l, 1.f);
}

color_t lab(float l, float a, float b, float alpha) {
    return lab_to_rgba((color_lab_t){ l, a, b, alpha });
}

color_t xyz(float x, float y, float z, float a) {
    return xyz_to_rgba((color_xyz_t){ x, y, z, a });
}

color_t yuv(float y, float u, float v, float a) {
    return yuv_to_rgba((color_yuv_t){ y, u, v, a });
}

color_t cmyk(float c, float m, float y, float k, float a) {
    return cmyk_to_rgba((color_cmyk_t){ c, m, y, k, a });
}

color_t rgb_565(uint16_t rgb565, uint8_t a) {
    return rgb565_to_rgba((color_rgb_t565){ rgb565, a });
}

// Basic RGBA <-> RGBAf conversions
color_rgbaf_t rgba_to_rgbaf(color_t rgba) {
    color_rgbaf_t result;
    result.r = rgba.r / 255.0f;
    result.g = rgba.g / 255.0f;
    result.b = rgba.b / 255.0f;
    result.a = rgba.a / 255.0f;
    return result;
}

color_t rgbaf_to_rgba(color_rgbaf_t rgbaf) {
    color_t result;
    result.r = _CLAMP_UINT8(rgbaf.r);
    result.g = _CLAMP_UINT8(rgbaf.g);
    result.b = _CLAMP_UINT8(rgbaf.b);
    result.a = _CLAMP_UINT8(rgbaf.a);
    return result;
}

// HSV conversions
color_hsva_t rgba_to_hsva(color_t rgba) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(rgba);
    return rgbaf_to_hsva(rgbaf);
}

color_hsva_t rgbaf_to_hsva(color_rgbaf_t rgbaf) {
    color_hsva_t hsva;
    float max = _FMAX3(rgbaf.r, rgbaf.g, rgbaf.b);
    float min = _FMIN3(rgbaf.r, rgbaf.g, rgbaf.b);
    float delta = max - min;

    // Value
    hsva.v = max;

    // Saturation
    hsva.s = (max == 0) ? 0 : delta / max;

    // Hue
    if (delta == 0) {
        hsva.h = 0; // achromatic
    } else {
        if (max == rgbaf.r) {
            hsva.h = (rgbaf.g - rgbaf.b) / delta + (rgbaf.g < rgbaf.b ? 6 : 0);
        } else if (max == rgbaf.g) {
            hsva.h = (rgbaf.b - rgbaf.r) / delta + 2;
        } else {
            hsva.h = (rgbaf.r - rgbaf.g) / delta + 4;
        }
        hsva.h /= 6.0f;
    }

    hsva.a = rgbaf.a;
    return hsva;
}

color_t hsva_to_rgba(color_hsva_t hsva) {
    color_rgbaf_t rgbaf = hsva_to_rgbaf(hsva);
    return rgbaf_to_rgba(rgbaf);
}

color_rgbaf_t hsva_to_rgbaf(color_hsva_t hsva) {
    color_rgbaf_t rgbaf;

    float c = hsva.v * hsva.s;
    float x = c * (1 - fabsf(fmodf(hsva.h * 6.0f, 2.0f) - 1));
    float m = hsva.v - c;

    float r1, g1, b1;

    if (hsva.h < 1.0f/6.0f) {
        r1 = c; g1 = x; b1 = 0;
    } else if (hsva.h < 2.0f/6.0f) {
        r1 = x; g1 = c; b1 = 0;
    } else if (hsva.h < 3.0f / 6.0f) {
        r1 = 0; g1 = c; b1 = x;
    } else if (hsva.h < 4.0f/6.0f) {
        r1 = 0; g1 = x; b1 = c;
    } else if (hsva.h < 5.0f/6.0f) {
        r1 = x; g1 = 0; b1 = c;
    } else {
        r1 = c; g1 = 0; b1 = x;
    }

    rgbaf.r = r1 + m;
    rgbaf.g = g1 + m;
    rgbaf.b = b1 + m;
    rgbaf.a = hsva.a;
    return rgbaf;
}

// HSL conversions
color_hsla_t rgba_to_hsla(color_t rgba) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(rgba);
    return rgbaf_to_hsla(rgbaf);
}

color_hsla_t rgbaf_to_hsla(color_rgbaf_t rgbaf) {
    color_hsla_t hsla;
    float max = _FMAX3(rgbaf.r, rgbaf.g, rgbaf.b);
    float min = _FMIN3(rgbaf.r, rgbaf.g, rgbaf.b);
    float delta = max - min;

    // Lightness
    hsla.l = (max + min) / 2.0f;

    if (delta == 0)
        hsla.h = hsla.s = 0; // achromatic
    else {
        // Saturation
        hsla.s = hsla.l > 0.5f ? delta / (2.0f - max - min) : delta / (max + min);

        // Hue
        if (max == rgbaf.r)
            hsla.h = (rgbaf.g - rgbaf.b) / delta + (rgbaf.g < rgbaf.b ? 6 : 0);
        else if (max == rgbaf.g)
            hsla.h = (rgbaf.b - rgbaf.r) / delta + 2;
        else
            hsla.h = (rgbaf.r - rgbaf.g) / delta + 4;
        hsla.h /= 6.0f;
    }

    hsla.a = rgbaf.a;
    return hsla;
}

color_t hsla_to_rgba(color_hsla_t hsla) {
    color_rgbaf_t rgbaf = hsla_to_rgbaf(hsla);
    return rgbaf_to_rgba(rgbaf);
}

static inline float hue_to_rgb(float p, float q, float t) {
    if (t < 0)
        t += 1;
    if (t > 1)
        t -= 1;
    if (t < 1.0f/6.0f)
        return p + (q - p) * 6 * t;
    if (t < 1.0f/2.0f)
        return q;
    if (t < 2.0f/3.0f)
        return p + (q - p) * (2.0f/3.0f - t) * 6;
    return p;
}

color_rgbaf_t hsla_to_rgbaf(color_hsla_t hsla) {
    color_rgbaf_t rgbaf;

    if (hsla.s == 0)
        rgbaf.r = rgbaf.g = rgbaf.b = hsla.l; // achromatic
    else {
        float q = hsla.l < 0.5f ? hsla.l * (1 + hsla.s) : hsla.l + hsla.s - hsla.l * hsla.s;
        float p = 2 * hsla.l - q;

        rgbaf.r = hue_to_rgb(p, q, hsla.h + 1.0f/3.0f);
        rgbaf.g = hue_to_rgb(p, q, hsla.h);
        rgbaf.b = hue_to_rgb(p, q, hsla.h - 1.0f/3.0f);
    }

    rgbaf.a = hsla.a;
    return rgbaf;
}

// XYZ conversions (sRGB D65 white point)
color_xyz_t rgba_to_xyz(color_t rgba) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(rgba);

    // Apply gamma correction (sRGB to linear RGB)
    float r = (rgbaf.r > 0.04045f) ? powf((rgbaf.r + 0.055f) / 1.055f, 2.4f) : rgbaf.r / 12.92f;
    float g = (rgbaf.g > 0.04045f) ? powf((rgbaf.g + 0.055f) / 1.055f, 2.4f) : rgbaf.g / 12.92f;
    float b = (rgbaf.b > 0.04045f) ? powf((rgbaf.b + 0.055f) / 1.055f, 2.4f) : rgbaf.b / 12.92f;

    color_xyz_t xyz;

    // sRGB to XYZ matrix (D65)
    xyz.x = r * 0.4124564f + g * 0.3575761f + b * 0.1804375f;
    xyz.y = r * 0.2126729f + g * 0.7151522f + b * 0.0721750f;
    xyz.z = r * 0.0193339f + g * 0.1191920f + b * 0.9503041f;
    xyz.a = rgbaf.a;

    return xyz;
}

color_t xyz_to_rgba(color_xyz_t xyz) {
    // XYZ to sRGB matrix (D65)
    float r = xyz.x *  3.2404542f + xyz.y * -1.5371385f + xyz.z * -0.4985314f;
    float g = xyz.x * -0.9692660f + xyz.y *  1.8760108f + xyz.z *  0.0415560f;
    float b = xyz.x *  0.0556434f + xyz.y * -0.2040259f + xyz.z *  1.0572252f;

    // Apply gamma correction (linear RGB to sRGB)
    r = (r > 0.0031308f) ? 1.055f * powf(r, 1.0f/2.4f) - 0.055f : 12.92f * r;
    g = (g > 0.0031308f) ? 1.055f * powf(g, 1.0f/2.4f) - 0.055f : 12.92f * g;
    b = (b > 0.0031308f) ? 1.055f * powf(b, 1.0f/2.4f) - 0.055f : 12.92f * b;

    color_rgbaf_t rgbaf = {
        _CLAMP_FLOAT(r),
        _CLAMP_FLOAT(g),
        _CLAMP_FLOAT(b),
        xyz.a
    };
    return rgbaf_to_rgba(rgbaf);
}

// Lab conversions
color_lab_t xyz_to_lab(color_xyz_t xyz) {
    // D65 white point
    const float xn = 0.95047f;
    const float yn = 1.00000f;
    const float zn = 1.08883f;

    float x = xyz.x / xn;
    float y = xyz.y / yn;
    float z = xyz.z / zn;

    // Apply Lab transformation
    x = (x > 0.008856f) ? cbrtf(x) : (7.787f * x + 16.0f/116.0f);
    y = (y > 0.008856f) ? cbrtf(y) : (7.787f * y + 16.0f/116.0f);
    z = (z > 0.008856f) ? cbrtf(z) : (7.787f * z + 16.0f/116.0f);

    color_lab_t lab;
    lab.l = 116.0f * y - 16.0f;
    lab.a = 500.0f * (x - y);
    lab.b = 200.0f * (y - z);
    lab.alpha = xyz.a;

    return lab;
}

color_xyz_t lab_to_xyz(color_lab_t lab) {
    // D65 white point
    const float xn = 0.95047f;
    const float yn = 1.00000f;
    const float zn = 1.08883f;

    float fy = (lab.l + 16.0f) / 116.0f;
    float fx = lab.a / 500.0f + fy;
    float fz = fy - lab.b / 200.0f;

    float x = (fx > 0.206897f) ? fx*fx*fx : (fx - 16.0f/116.0f) / 7.787f;
    float y = (fy > 0.206897f) ? fy*fy*fy : (fy - 16.0f/116.0f) / 7.787f;
    float z = (fz > 0.206897f) ? fz*fz*fz : (fz - 16.0f/116.0f) / 7.787f;

    color_xyz_t xyz;
    xyz.x = x * xn;
    xyz.y = y * yn;
    xyz.z = z * zn;
    xyz.a = lab.alpha;

    return xyz;
}

color_lab_t rgba_to_lab(color_t rgba) {
    color_xyz_t xyz = rgba_to_xyz(rgba);
    return xyz_to_lab(xyz);
}

color_t lab_to_rgba(color_lab_t lab) {
    color_xyz_t xyz = lab_to_xyz(lab);
    return xyz_to_rgba(xyz);
}

// YUV conversions (BT.601)
color_yuv_t rgba_to_yuv(color_t rgba) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(rgba);

    color_yuv_t yuv;
    yuv.y = 0.299f * rgbaf.r + 0.587f * rgbaf.g + 0.114f * rgbaf.b;
    yuv.u = -0.14713f * rgbaf.r - 0.28886f * rgbaf.g + 0.436f * rgbaf.b;
    yuv.v = 0.615f * rgbaf.r - 0.51499f * rgbaf.g - 0.10001f * rgbaf.b;
    yuv.a = rgbaf.a;

    return yuv;
}

color_t yuv_to_rgba(color_yuv_t yuv) {
    color_rgbaf_t rgbaf;

    rgbaf.r = yuv.y + 1.13983f * yuv.v;
    rgbaf.g = yuv.y - 0.39465f * yuv.u - 0.58060f * yuv.v;
    rgbaf.b = yuv.y + 2.03211f * yuv.u;
    rgbaf.a = yuv.a;

    // Clamp values
    rgbaf.r = _CLAMP_FLOAT(rgbaf.r);
    rgbaf.g = _CLAMP_FLOAT(rgbaf.g);
    rgbaf.b = _CLAMP_FLOAT(rgbaf.b);

    return rgbaf_to_rgba(rgbaf);
}

// CMYK conversions
color_cmyk_t rgba_to_cmyk(color_t rgba) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(rgba);

    color_cmyk_t cmyk;
    // Find K (black)
    cmyk.k = 1.0f - _FMAX3(rgbaf.r, rgbaf.g, rgbaf.b);

    if (cmyk.k == 1.0f) // Pure black
        cmyk.c = cmyk.m = cmyk.y = 0.0f;
    else {
        float inv_k = 1.0f - cmyk.k;
        cmyk.c = (1.0f - rgbaf.r - cmyk.k) / inv_k;
        cmyk.m = (1.0f - rgbaf.g - cmyk.k) / inv_k;
        cmyk.y = (1.0f - rgbaf.b - cmyk.k) / inv_k;
    }

    cmyk.a = rgbaf.a;
    return cmyk;
}

color_t cmyk_to_rgba(color_cmyk_t cmyk) {
    color_rgbaf_t rgbaf;

    float inv_k = 1.0f - cmyk.k;
    rgbaf.r = (1.0f - cmyk.c) * inv_k;
    rgbaf.g = (1.0f - cmyk.m) * inv_k;
    rgbaf.b = (1.0f - cmyk.y) * inv_k;
    rgbaf.a = cmyk.a;

    return rgbaf_to_rgba(rgbaf);
}

// RGB565 conversions
color_rgb_t565 rgba_to_rgb565(color_t rgba) {
    color_rgb_t565 rgb565;

    // Convert to 5-6-5 format
    uint16_t r = (rgba.r >> 3) & 0x1F;
    uint16_t g = (rgba.g >> 2) & 0x3F;
    uint16_t b = (rgba.b >> 3) & 0x1F;

    rgb565.rgb565 = (r << 11) | (g << 5) | b;
    rgb565.a = rgba.a;

    return rgb565;
}

color_t rgb565_to_rgba(color_rgb_t565 rgb565) {
    color_t rgba;

    // Extract components
    uint16_t r = (rgb565.rgb565 >> 11) & 0x1F;
    uint16_t g = (rgb565.rgb565 >> 5) & 0x3F;
    uint16_t b = rgb565.rgb565 & 0x1F;

    // Scale back to 8-bit
    rgba.r = (r << 3) | (r >> 2);
    rgba.g = (g << 2) | (g >> 4);
    rgba.b = (b << 3) | (b >> 2);
    rgba.a = rgb565.a;

    return rgba;
}

float color_distance_lab(color_lab_t a, color_lab_t b) {
    float dl = a.l - b.l;
    float da = a.a - b.a;
    float db = a.b - b.b;
    return sqrtf(dl*dl + da*da + db*db);
}

float color_distance(color_t a, color_t b) {
    int dr = a.r - b.r;
    int dg = a.g - b.g;
    int db = a.b - b.b;
    return sqrtf(dr*dr + dg*dg + db*db);
}

color_t color_lerp(color_t a, color_t b, float t) {
    t = _CLAMP_FLOAT(t);
    color_t result;
    result.r = (uint8_t)(a.r + t * (b.r - a.r));
    result.g = (uint8_t)(a.g + t * (b.g - a.g));
    result.b = (uint8_t)(a.b + t * (b.b - a.b));
    result.a = (uint8_t)(a.a + t * (b.a - a.a));
    return result;
}

// Perceptual luminance using sRGB coefficients
float color_luminance(color_t color) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    return 0.299f * rgbaf.r + 0.587f * rgbaf.g + 0.114f * rgbaf.b;
}

// Relative luminance for WCAG calculations (uses gamma-corrected values)
float color_relative_luminance(color_t color) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);

    // Apply gamma correction (sRGB to linear RGB)
    float r = (rgbaf.r > 0.04045f) ? powf((rgbaf.r + 0.055f) / 1.055f, 2.4f) : rgbaf.r / 12.92f;
    float g = (rgbaf.g > 0.04045f) ? powf((rgbaf.g + 0.055f) / 1.055f, 2.4f) : rgbaf.g / 12.92f;
    float b = (rgbaf.b > 0.04045f) ? powf((rgbaf.b + 0.055f) / 1.055f, 2.4f) : rgbaf.b / 12.92f;

    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Simple average brightness
float color_brightness(color_t color) {
    return (color.r + color.g + color.b) / (3.0f * 255.0f);
}

// Check if color is considered dark (using perceptual luminance)
int color_is_dark(color_t color) {
    return color_luminance(color) < 0.5f;
}

// Get saturation level (0-1)
float color_saturation(color_t color) {
    color_hsva_t hsva = rgba_to_hsva(color);
    return hsva.s;
}

// Get hue in degrees (0-360)
float color_hue(color_t color) {
    color_hsva_t hsva = rgba_to_hsva(color);
    return hsva.h * 360.0f;
}

// Convert to grayscale using perceptual luminance
color_t color_grayscale(color_t color) {
    uint8_t gray = (uint8_t)(color_luminance(color) * 255.0f);
    color_t result = {gray, gray, gray, color.a};
    return result;
}

// Invert RGB channels (keeping alpha)
color_t color_invert(color_t color) {
    color_t result = {
        255 - color.r,
        255 - color.g,
        255 - color.b,
        color.a
    };
    return result;
}

// Get complementary color (opposite hue)
color_t color_complement(color_t color) {
    color_hsva_t hsva = rgba_to_hsva(color);
    hsva.h = fmodf(hsva.h + 0.5f, 1.0f);  // Add 180 degrees (0.5 in normalized hue)
    return hsva_to_rgba(hsva);
}

// Additional analysis functions

// Get the dominant color channel
int color_dominant_channel(color_t color) {
    if (color.r >= color.g && color.r >= color.b) return 0; // Red
    if (color.g >= color.b) return 1; // Green
    return 2; // Blue
}

// Check if color is grayscale (all channels equal within tolerance)
int color_is_grayscale(color_t color) {
    int max_diff = abs(color.r - color.g);
    max_diff = fmax(max_diff, abs(color.r - color.b));
    max_diff = fmax(max_diff, abs(color.g - color.b));
    return max_diff <= 2; // Allow small tolerance for rounding errors
}

// Get color temperature estimate (very rough approximation)
float color_temperature_estimate(color_t color) {
    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);

    // Rough approximation based on blue/red ratio
    if (rgbaf.r == 0.0f) return 6500.0f; // Default daylight

    float ratio = rgbaf.b / rgbaf.r;
    if (ratio > 1.0f) {
        // More blue than red - cooler
        return 6500.0f + (ratio - 1.0f) * 3500.0f; // Up to 10000K
    } else {
        // More red than blue - warmer
        return 6500.0f - (1.0f - ratio) * 3500.0f; // Down to 3000K
    }
}

// Check if color is "warm" (reddish/yellowish)
int color_is_warm(color_t color) {
    return color.r > color.b && (color.r + color.g) > color.b * 1.5f;
}

// Check if color is "cool" (blueish/greenish)
int color_is_cool(color_t color) {
    return color.b > color.r && (color.b + color.g) > color.r * 1.5f;
}

// Get color purity (distance from gray)
float color_purity(color_t color) {
    uint8_t gray = (uint8_t)(color_luminance(color) * 255.0f);
    int dr = abs(color.r - gray);
    int dg = abs(color.g - gray);
    int db = abs(color.b - gray);
    return sqrtf(dr*dr + dg*dg + db*db) / (255.0f * sqrtf(3.0f));
}

// Get color energy (sum of all channels)
float color_energy(color_t color) {
    return (color.r + color.g + color.b) / (3.0f * 255.0f);
}

// Adjust brightness (-1.0 to 1.0, where 0 = no change)
color_t color_adjust_brightness(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    rgbaf.r = _CLAMP_FLOAT(rgbaf.r + amount);
    rgbaf.g = _CLAMP_FLOAT(rgbaf.g + amount);
    rgbaf.b = _CLAMP_FLOAT(rgbaf.b + amount);

    return rgbaf_to_rgba(rgbaf);
}

// Adjust contrast (-1.0 to 1.0, where 0 = no change)
color_t color_adjust_contrast(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);
    float factor = (1.0f + amount) / (1.0f - amount);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    rgbaf.r = _CLAMP_FLOAT((rgbaf.r - 0.5f) * factor + 0.5f);
    rgbaf.g = _CLAMP_FLOAT((rgbaf.g - 0.5f) * factor + 0.5f);
    rgbaf.b = _CLAMP_FLOAT((rgbaf.b - 0.5f) * factor + 0.5f);

    return rgbaf_to_rgba(rgbaf);
}

// Apply gamma correction
color_t color_adjust_gamma(color_t color, float gamma) {
    if (gamma <= 0.0f) gamma = 0.01f; // Prevent division by zero

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    rgbaf.r = powf(rgbaf.r, 1.0f / gamma);
    rgbaf.g = powf(rgbaf.g, 1.0f / gamma);
    rgbaf.b = powf(rgbaf.b, 1.0f / gamma);

    return rgbaf_to_rgba(rgbaf);
}

// Increase saturation (0.0 to 2.0+, where 1.0 = no change)
color_t color_saturate(color_t color, float amount) {
    if (amount < 0.0f) amount = 0.0f;

    color_hsva_t hsva = rgba_to_hsva(color);
    hsva.s = _CLAMP_FLOAT(hsva.s * amount);

    return hsva_to_rgba(hsva);
}

// Decrease saturation (0.0 to 1.0, where 0.0 = grayscale, 1.0 = no change)
color_t color_desaturate(color_t color, float amount) {
    amount = _CLAMP_FLOAT(amount);

    color_hsva_t hsva = rgba_to_hsva(color);
    hsva.s *= amount;

    return hsva_to_rgba(hsva);
}

// Shift hue by degrees (-360 to 360)
color_t color_hue_shift(color_t color, float degrees) {
    float shift = fmodf(degrees / 360.0f, 1.0f);
    if (shift < 0.0f) shift += 1.0f;

    color_hsva_t hsva = rgba_to_hsva(color);
    hsva.h = fmodf(hsva.h + shift, 1.0f);

    return hsva_to_rgba(hsva);
}

// Adjust color temperature (1000K to 40000K, 6500K = daylight)
color_t color_adjust_temperature(color_t color, float kelvin) {
    kelvin = _CLAMP_FLOAT_RANGE(kelvin, 1000.0f, 40000.0f);

    // Temperature to RGB conversion (rough approximation)
    float temp = kelvin / 100.0f;
    float r, g, b;

    if (temp <= 66.0f) {
        r = 255.0f;
        g = temp;
        g = 99.4708025861f * logf(g) - 161.1195681661f;
    } else {
        r = temp - 60.0f;
        r = 329.698727446f * powf(r, -0.1332047592f);
        g = temp - 60.0f;
        g = 288.1221695283f * powf(g, -0.0755148492f);
    }

    if (temp >= 66.0f) {
        b = 255.0f;
    } else if (temp <= 19.0f) {
        b = 0.0f;
    } else {
        b = temp - 10.0f;
        b = 138.5177312231f * logf(b) - 305.0447927307f;
    }

    // Normalize and blend with original color
    r = _CLAMP_FLOAT_RANGE(r, 0.0f, 255.0f) / 255.0f;
    g = _CLAMP_FLOAT_RANGE(g, 0.0f, 255.0f) / 255.0f;
    b = _CLAMP_FLOAT_RANGE(b, 0.0f, 255.0f) / 255.0f;

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    rgbaf.r *= r;
    rgbaf.g *= g;
    rgbaf.b *= b;

    return rgbaf_to_rgba(rgbaf);
}

// Adjust tint (magenta-green balance, -1.0 to 1.0)
color_t color_tint(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);

    if (amount > 0.0f) {
        // Add magenta (increase red and blue, decrease green)
        rgbaf.r += amount * 0.1f;
        rgbaf.b += amount * 0.1f;
        rgbaf.g -= amount * 0.05f;
    } else {
        // Add green (increase green, decrease red and blue)
        rgbaf.g += (-amount) * 0.1f;
        rgbaf.r -= (-amount) * 0.05f;
        rgbaf.b -= (-amount) * 0.05f;
    }

    rgbaf.r = _CLAMP_FLOAT(rgbaf.r);
    rgbaf.g = _CLAMP_FLOAT(rgbaf.g);
    rgbaf.b = _CLAMP_FLOAT(rgbaf.b);

    return rgbaf_to_rgba(rgbaf);
}

// Adjust exposure in stops (-3.0 to 3.0, where 0 = no change)
color_t color_adjust_exposure(color_t color, float stops) {
    stops = _CLAMP_FLOAT_RANGE(stops, -3.0f, 3.0f);
    float factor = powf(2.0f, stops);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    rgbaf.r = _CLAMP_FLOAT(rgbaf.r * factor);
    rgbaf.g = _CLAMP_FLOAT(rgbaf.g * factor);
    rgbaf.b = _CLAMP_FLOAT(rgbaf.b * factor);

    return rgbaf_to_rgba(rgbaf);
}

// Adjust highlights (-1.0 to 1.0, affects bright areas)
color_t color_adjust_highlights(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    float lum = color_luminance(color);

    // Only affect pixels above 50% luminance
    if (lum > 0.5f) {
        float weight = (lum - 0.5f) * 2.0f; // 0 to 1
        rgbaf.r = _CLAMP_FLOAT(rgbaf.r + amount * weight * 0.2f);
        rgbaf.g = _CLAMP_FLOAT(rgbaf.g + amount * weight * 0.2f);
        rgbaf.b = _CLAMP_FLOAT(rgbaf.b + amount * weight * 0.2f);
    }

    return rgbaf_to_rgba(rgbaf);
}

// Adjust shadows (-1.0 to 1.0, affects dark areas)
color_t color_adjust_shadows(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    float lum = color_luminance(color);

    // Only affect pixels below 50% luminance
    if (lum < 0.5f) {
        float weight = (0.5f - lum) * 2.0f; // 0 to 1
        rgbaf.r = _CLAMP_FLOAT(rgbaf.r + amount * weight * 0.2f);
        rgbaf.g = _CLAMP_FLOAT(rgbaf.g + amount * weight * 0.2f);
        rgbaf.b = _CLAMP_FLOAT(rgbaf.b + amount * weight * 0.2f);
    }

    return rgbaf_to_rgba(rgbaf);
}

// Adjust whites (-1.0 to 1.0, affects very bright areas)
color_t color_adjust_whites(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    float lum = color_luminance(color);

    // Only affect pixels above 80% luminance
    if (lum > 0.8f) {
        float weight = (lum - 0.8f) * 5.0f; // 0 to 1
        rgbaf.r = _CLAMP_FLOAT(rgbaf.r + amount * weight * 0.3f);
        rgbaf.g = _CLAMP_FLOAT(rgbaf.g + amount * weight * 0.3f);
        rgbaf.b = _CLAMP_FLOAT(rgbaf.b + amount * weight * 0.3f);
    }

    return rgbaf_to_rgba(rgbaf);
}

// Adjust blacks (-1.0 to 1.0, affects very dark areas)
color_t color_adjust_blacks(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    float lum = color_luminance(color);

    // Only affect pixels below 20% luminance
    if (lum < 0.2f) {
        float weight = (0.2f - lum) * 5.0f; // 0 to 1
        rgbaf.r = _CLAMP_FLOAT(rgbaf.r + amount * weight * 0.3f);
        rgbaf.g = _CLAMP_FLOAT(rgbaf.g + amount * weight * 0.3f);
        rgbaf.b = _CLAMP_FLOAT(rgbaf.b + amount * weight * 0.3f);
    }

    return rgbaf_to_rgba(rgbaf);
}

// Adjust clarity/structure (-1.0 to 1.0, enhances local contrast)
color_t color_adjust_clarity(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    // Simplified clarity - boost contrast in mid-tones
    color_rgbaf_t rgbaf = rgba_to_rgbaf(color);
    float lum = color_luminance(color);

    // Maximum effect at 50% luminance
    float weight = 1.0f - fabsf(lum - 0.5f) * 2.0f;
    weight = _CLAMP_FLOAT(weight);

    float contrast_factor = 1.0f + amount * weight * 0.5f;
    rgbaf.r = _CLAMP_FLOAT((rgbaf.r - 0.5f) * contrast_factor + 0.5f);
    rgbaf.g = _CLAMP_FLOAT((rgbaf.g - 0.5f) * contrast_factor + 0.5f);
    rgbaf.b = _CLAMP_FLOAT((rgbaf.b - 0.5f) * contrast_factor + 0.5f);

    return rgbaf_to_rgba(rgbaf);
}

// Adjust vibrance (-1.0 to 1.0, smart saturation that protects skin tones)
color_t color_adjust_vibrance(color_t color, float amount) {
    amount = _CLAMP_FLOAT_RANGE(amount, -1.0f, 1.0f);

    color_hsva_t hsva = rgba_to_hsva(color);

    // Reduce effect on skin tones (hue around 0.05-0.15, or 18-54 degrees)
    float skin_protection = 1.0f;
    if (hsva.h >= 0.05f && hsva.h <= 0.15f) {
        skin_protection = 0.3f; // Reduce vibrance effect on skin tones
    }

    // Reduce effect on already saturated colors
    float saturation_protection = 1.0f - hsva.s * 0.5f;

    float final_amount = amount * skin_protection * saturation_protection;
    hsva.s = _CLAMP_FLOAT(hsva.s + final_amount * 0.5f);

    return hsva_to_rgba(hsva);
}

// Helper function to blend alpha
static inline uint8_t blend_alpha(uint8_t a1, uint8_t a2) {
    return 255 - (((255 - a1) * (255 - a2)) / 255);
}

// Multiply: a * b / 255
color_t color_multiply(color_t a, color_t b) {
    color_t result;
    result.r = (a.r * b.r) / 255;
    result.g = (a.g * b.g) / 255;
    result.b = (a.b * b.b) / 255;
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Screen: 255 - ((255-a) * (255-b) / 255)
color_t color_screen(color_t a, color_t b) {
    color_t result;
    result.r = 255 - (((255 - a.r) * (255 - b.r)) / 255);
    result.g = 255 - (((255 - a.g) * (255 - b.g)) / 255);
    result.b = 255 - (((255 - a.b) * (255 - b.b)) / 255);
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Overlay: combines multiply and screen
color_t color_overlay(color_t a, color_t b) {
    color_t result;

    // For each channel: if base < 128, multiply*2, else screen*2-255
    result.r = (a.r < 128) ? (2 * a.r * b.r) / 255 : 255 - (2 * (255 - a.r) * (255 - b.r)) / 255;
    result.g = (a.g < 128) ? (2 * a.g * b.g) / 255 : 255 - (2 * (255 - a.g) * (255 - b.g)) / 255;
    result.b = (a.b < 128) ? (2 * a.b * b.b) / 255 : 255 - (2 * (255 - a.b) * (255 - b.b)) / 255;
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Soft Light: complex blend mode
color_t color_soft_light(color_t a, color_t b) {
    color_t result;

    // Simplified soft light formula
    for (int i = 0; i < 3; i++) {
        uint8_t base = ((uint8_t*)&a)[i];
        uint8_t blend = ((uint8_t*)&b)[i];

        if (blend < 128) {
            ((uint8_t*)&result)[i] = (2 * base * blend) / 255 + (base * base * (255 - 2 * blend)) / (255 * 255);
        } else {
            int sqrt_base = (int)(255 * sqrt(base / 255.0));
            ((uint8_t*)&result)[i] = (2 * base * (255 - blend)) / 255 + sqrt_base * (2 * blend - 255) / 255;
        }
    }
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Hard Light: reverse of overlay
color_t color_hard_light(color_t a, color_t b) {
    color_t result;

    // For each channel: if blend < 128, multiply*2, else screen*2-255
    result.r = (b.r < 128) ? (2 * a.r * b.r) / 255 : 255 - (2 * (255 - a.r) * (255 - b.r)) / 255;
    result.g = (b.g < 128) ? (2 * a.g * b.g) / 255 : 255 - (2 * (255 - a.g) * (255 - b.g)) / 255;
    result.b = (b.b < 128) ? (2 * a.b * b.b) / 255 : 255 - (2 * (255 - a.b) * (255 - b.b)) / 255;
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Color Dodge: base / (255 - blend)
color_t color_color_dodge(color_t a, color_t b) {
    color_t result;

    result.r = (b.r == 255) ? 255 : (a.r * 255) / (255 - b.r);
    result.g = (b.g == 255) ? 255 : (a.g * 255) / (255 - b.g);
    result.b = (b.b == 255) ? 255 : (a.b * 255) / (255 - b.b);

    // Clamp to 255
    result.r = (result.r > 255) ? 255 : result.r;
    result.g = (result.g > 255) ? 255 : result.g;
    result.b = (result.b > 255) ? 255 : result.b;

    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Color Burn: 255 - (255 - base) / blend
color_t color_color_burn(color_t a, color_t b) {
    color_t result;

    result.r = (b.r == 0) ? 0 : 255 - ((255 - a.r) * 255) / b.r;
    result.g = (b.g == 0) ? 0 : 255 - ((255 - a.g) * 255) / b.g;
    result.b = (b.b == 0) ? 0 : 255 - ((255 - a.b) * 255) / b.b;

    // Clamp to valid range
    result.r = (result.r > 255) ? 255 : (result.r < 0) ? 0 : result.r;
    result.g = (result.g > 255) ? 255 : (result.g < 0) ? 0 : result.g;
    result.b = (result.b > 255) ? 255 : (result.b < 0) ? 0 : result.b;

    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Darken: min(a, b)
color_t color_darken(color_t a, color_t b) {
    color_t result;
    result.r = (a.r < b.r) ? a.r : b.r;
    result.g = (a.g < b.g) ? a.g : b.g;
    result.b = (a.b < b.b) ? a.b : b.b;
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Lighten: max(a, b)
color_t color_lighten(color_t a, color_t b) {
    color_t result;
    result.r = (a.r > b.r) ? a.r : b.r;
    result.g = (a.g > b.g) ? a.g : b.g;
    result.b = (a.b > b.b) ? a.b : b.b;
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Difference: |a - b|
color_t color_difference(color_t a, color_t b) {
    color_t result;
    result.r = (a.r > b.r) ? a.r - b.r : b.r - a.r;
    result.g = (a.g > b.g) ? a.g - b.g : b.g - a.g;
    result.b = (a.b > b.b) ? a.b - b.b : b.b - a.b;
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Exclusion: a + b - 2*a*b/255
color_t color_exclusion(color_t a, color_t b) {
    color_t result;
    result.r = a.r + b.r - (2 * a.r * b.r) / 255;
    result.g = a.g + b.g - (2 * a.g * b.g) / 255;
    result.b = a.b + b.b - (2 * a.b * b.b) / 255;
    result.a = blend_alpha(a.a, b.a);
    return result;
}

// Alpha blending: standard Porter-Duff "over" operation
color_t color_alpha_blend(color_t fg, color_t bg) {
    color_t result;

    // Convert alpha to 0-1 range for calculations
    float fg_alpha = fg.a / 255.0f;
    float bg_alpha = bg.a / 255.0f;

    // Calculate output alpha: fg_alpha + bg_alpha * (1 - fg_alpha)
    float out_alpha = fg_alpha + bg_alpha * (1.0f - fg_alpha);

    if (out_alpha == 0) {
        // Completely transparent
        result.r = result.g = result.b = result.a = 0;
        return result;
    }

    // Alpha blend RGB channels: (fg * fg_alpha + bg * bg_alpha * (1 - fg_alpha)) / out_alpha
    result.r = (uint8_t)((fg.r * fg_alpha + bg.r * bg_alpha * (1.0f - fg_alpha)) / out_alpha);
    result.g = (uint8_t)((fg.g * fg_alpha + bg.g * bg_alpha * (1.0f - fg_alpha)) / out_alpha);
    result.b = (uint8_t)((fg.b * fg_alpha + bg.b * bg_alpha * (1.0f - fg_alpha)) / out_alpha);
    result.a = (uint8_t)(out_alpha * 255);

    return result;
}

// Premultiply alpha: multiply RGB channels by alpha
color_t color_premultiply_alpha(color_t color) {
    color_t result;

    float alpha = color.a / 255.0f;
    result.r = (uint8_t)(color.r * alpha);
    result.g = (uint8_t)(color.g * alpha);
    result.b = (uint8_t)(color.b * alpha);
    result.a = color.a;

    return result;
}

// Unpremultiply alpha: divide RGB channels by alpha
color_t color_unpremultiply_alpha(color_t color) {
    color_t result;

    if (color.a == 0) {
        // Completely transparent - RGB values are undefined
        result.r = result.g = result.b = result.a = 0;
        return result;
    }

    float alpha = color.a / 255.0f;
    result.r = (uint8_t)(color.r / alpha);
    result.g = (uint8_t)(color.g / alpha);
    result.b = (uint8_t)(color.b / alpha);
    result.a = color.a;

    // Clamp to prevent overflow
    result.r = (result.r > 255) ? 255 : result.r;
    result.g = (result.g > 255) ? 255 : result.g;
    result.b = (result.b > 255) ? 255 : result.b;

    return result;
}

// Normalize hue to 0-360 range
static float normalize_hue(float hue) {
    while (hue < 0) hue += 360;
    while (hue >= 360) hue -= 360;
    return hue;
}

// Analogous colors: adjacent hues (±30° typically)
void color_analogous(color_t base, color_t* colors, int count) {
    if (count <= 0) return;

    color_hsva_t base_hsv = rgba_to_hsva(base);
    float step = 60.0f / (count + 1); // Spread across ±30°

    for (int i = 0; i < count; i++) {
        color_hsva_t variant = base_hsv;
        variant.h = normalize_hue(base_hsv.h + (i + 1 - count/2.0f) * step);
        colors[i] = hsva_to_rgba(variant);
    }
}

// Triadic colors: 120° apart
void color_triadic(color_t base, color_t* color1, color_t* color2) {
    if (!color1 && !color2)
        return;

    color_hsva_t base_hsv = rgba_to_hsva(base);

    // First triadic color: +120°
    color_hsva_t triadic1 = base_hsv;
    triadic1.h = normalize_hue(base_hsv.h + 120);
    if (color1)
        *color1 = hsva_to_rgba(triadic1);

    // Second triadic color: +240°
    color_hsva_t triadic2 = base_hsv;
    triadic2.h = normalize_hue(base_hsv.h + 240);
    if (color2)
        *color2 = hsva_to_rgba(triadic2);
}

// Tetradic colors: rectangle scheme (90° apart)
void color_tetradic(color_t base, color_t* color1, color_t* color2, color_t* color3) {
    if (!color1 && !color2 && !color3)
        return;

    color_hsva_t base_hsv = rgba_to_hsva(base);
    // Four colors: +90°, +180°, +270°
    color_hsva_t tetradic = base_hsv;
    tetradic.h = normalize_hue(base_hsv.h + 90);
    if (color1)
        *color1 = hsva_to_rgba(tetradic);
    tetradic.h = normalize_hue(base_hsv.h + 180);
    if (color2)
        *color2 = hsva_to_rgba(tetradic);
    tetradic.h = normalize_hue(base_hsv.h + 270);
    if (color3)
        *color3 = hsva_to_rgba(tetradic);
}

// Split complementary: base complement ±30°
void color_split_complementary(color_t base, color_t* color1, color_t* color2) {
    if (!color1 && !color2)
        return;

    color_hsva_t base_hsv = rgba_to_hsva(base);
    float complement_hue = normalize_hue(base_hsv.h + 180);

    // First split complement: complement - 30°
    color_hsva_t split1 = base_hsv;
    split1.h = normalize_hue(complement_hue - 30);
    if (color1)
        *color1 = hsva_to_rgba(split1);

    // Second split complement: complement + 30°
    color_hsva_t split2 = base_hsv;
    split2.h = normalize_hue(complement_hue + 30);
    if (color2)
        *color2 = hsva_to_rgba(split2);
}

// Monochromatic colors: same hue, different saturation/value
void color_monochromatic(color_t base, color_t* colors, int count) {
    if (count <= 0) return;

    color_hsva_t base_hsv = rgba_to_hsva(base);

    for (int i = 0; i < count; i++) {
        color_hsva_t variant = base_hsv;
        float factor = (i + 1.0f) / (count + 1.0f);

        // Vary saturation and value
        variant.s = base_hsv.s * (0.3f + 0.7f * factor);
        variant.v = base_hsv.v * (0.4f + 0.6f * factor);

        // Clamp values
        variant.s = (variant.s > 1.0f) ? 1.0f : variant.s;
        variant.v = (variant.v > 1.0f) ? 1.0f : variant.v;

        colors[i] = hsva_to_rgba(variant);
    }
}

// Generate gradient between two colors
void color_gradient(color_t start, color_t end, color_t* colors, int count) {
    if (count <= 0) return;

    for (int i = 0; i < count; i++) {
        float t = (count == 1) ? 0.5f : (float)i / (count - 1);

        // Linear interpolation in RGB space
        colors[i].r = (uint8_t)(start.r + t * (end.r - start.r));
        colors[i].g = (uint8_t)(start.g + t * (end.g - start.g));
        colors[i].b = (uint8_t)(start.b + t * (end.b - start.b));
        colors[i].a = (uint8_t)(start.a + t * (end.a - start.a));
    }
}

// Gamma correction for sRGB
static float gamma_correct(float value) {
    if (value <= 0.04045f) {
        return value / 12.92f;
    } else {
        return powf((value + 0.055f) / 1.055f, 2.4f);
    }
}

// Inverse gamma correction for sRGB
static float inverse_gamma_correct(float value) {
    if (value <= 0.0031308f) {
        return 12.92f * value;
    } else {
        return 1.055f * powf(value, 1.0f / 2.4f) - 0.055f;
    }
}

// Convert RGBA to XYZ with custom white point
color_xyz_t rgba_to_xyz_custom(color_t rgba, float wx, float wy, float wz) {
    color_xyz_t xyz;

    // Convert to 0-1 range and apply gamma correction
    float r = gamma_correct(rgba.r / 255.0f);
    float g = gamma_correct(rgba.g / 255.0f);
    float b = gamma_correct(rgba.b / 255.0f);

    // sRGB to XYZ matrix (D65 illuminant)
    // Then scale by custom white point
    float x = (0.4124564f * r + 0.3575761f * g + 0.1804375f * b) * wx;
    float y = (0.2126729f * r + 0.7151522f * g + 0.0721750f * b) * wy;
    float z = (0.0193339f * r + 0.1191920f * g + 0.9503041f * b) * wz;

    xyz.x = x;
    xyz.y = y;
    xyz.z = z;
    xyz.a = rgba.a / 255.0f;

    return xyz;
}

// XYZ to LAB conversion helper
static float xyz_to_lab_f(float t) {
    const float delta = 6.0f / 29.0f;
    const float delta_cubed = delta * delta * delta;

    if (t > delta_cubed) {
        return powf(t, 1.0f / 3.0f);
    } else {
        return t / (3.0f * delta * delta) + 4.0f / 29.0f;
    }
}

// Convert RGBA to LAB with custom white point
color_lab_t rgba_to_lab_custom(color_t rgba, float wx, float wy, float wz) {
    color_lab_t lab;

    // First convert to XYZ
    color_xyz_t xyz = rgba_to_xyz_custom(rgba, wx, wy, wz);

    // Normalize by white point
    float xn = xyz.x / wx;
    float yn = xyz.y / wy;
    float zn = xyz.z / wz;

    // Convert to LAB
    float fx = xyz_to_lab_f(xn);
    float fy = xyz_to_lab_f(yn);
    float fz = xyz_to_lab_f(zn);

    lab.l = 116.0f * fy - 16.0f;
    lab.a = 500.0f * (fx - fy);
    lab.b = 200.0f * (fy - fz);
    lab.alpha = rgba.a / 255.0f;

    return lab;
}

// Delta E CIE76 (simple Euclidean distance in LAB)
float color_delta_e_76(color_lab_t a, color_lab_t b) {
    float dl = a.l - b.l;
    float da = a.a - b.a;
    float db = a.b - b.b;

    return sqrtf(dl*dl + da*da + db*db);
}

// Delta E CIE94 (improved formula with weighting)
float color_delta_e_94(color_lab_t a, color_lab_t b) {
    float dl = a.l - b.l;
    float da = a.a - b.a;
    float db = a.b - b.b;

    float c1 = sqrtf(a.a * a.a + a.b * a.b);
    float c2 = sqrtf(b.a * b.a + b.b * b.b);
    float dc = c1 - c2;

    float dh_squared = da*da + db*db - dc*dc;
    float dh = (dh_squared < 0) ? 0 : sqrtf(dh_squared);

    // Weighting factors for CIE94
    float kl = 1.0f;
    float kc = 1.0f;
    float kh = 1.0f;

    float sl = 1.0f;
    float sc = 1.0f + 0.045f * c1;
    float sh = 1.0f + 0.015f * c1;

    float dl_term = dl / (kl * sl);
    float dc_term = dc / (kc * sc);
    float dh_term = dh / (kh * sh);

    return sqrtf(dl_term*dl_term + dc_term*dc_term + dh_term*dh_term);
}

// Delta E CIEDE2000 (most perceptually accurate)
float color_delta_e_2000(color_lab_t a, color_lab_t b) {
    float l1 = a.l, a1 = a.a, b1 = a.b;
    float l2 = b.l, a2 = b.a, b2 = b.b;

    float lbar = (l1 + l2) / 2.0f;
    float c1 = sqrtf(a1*a1 + b1*b1);
    float c2 = sqrtf(a2*a2 + b2*b2);
    float cbar = (c1 + c2) / 2.0f;

    float cbar7 = powf(cbar, 7.0f);
    float g = 0.5f * (1.0f - sqrtf(cbar7 / (cbar7 + powf(25.0f, 7.0f))));

    float ap1 = (1.0f + g) * a1;
    float ap2 = (1.0f + g) * a2;
    float cp1 = sqrtf(ap1*ap1 + b1*b1);
    float cp2 = sqrtf(ap2*ap2 + b2*b2);

    float hp1 = (b1 == 0 && ap1 == 0) ? 0 : atan2f(b1, ap1) * 180.0f / M_PI;
    float hp2 = (b2 == 0 && ap2 == 0) ? 0 : atan2f(b2, ap2) * 180.0f / M_PI;

    if (hp1 < 0) hp1 += 360.0f;
    if (hp2 < 0) hp2 += 360.0f;

    float dl = l2 - l1;
    float dc = cp2 - cp1;
    float dhp = 0;

    if (cp1 * cp2 != 0) {
        dhp = hp2 - hp1;
        if (dhp > 180.0f) dhp -= 360.0f;
        if (dhp < -180.0f) dhp += 360.0f;
    }

    float dh = 2.0f * sqrtf(cp1 * cp2) * sinf(dhp * M_PI / 360.0f);

    float cpbar = (cp1 + cp2) / 2.0f;
    float hpbar = 0;

    if (cp1 * cp2 != 0) {
        hpbar = (hp1 + hp2) / 2.0f;
        if (fabsf(hp1 - hp2) > 180.0f) {
            if (hpbar < 180.0f) hpbar += 180.0f;
            else hpbar -= 180.0f;
        }
    }

    float t = 1.0f - 0.17f * cosf((hpbar - 30.0f) * M_PI / 180.0f) +
              0.24f * cosf(2.0f * hpbar * M_PI / 180.0f) +
              0.32f * cosf((3.0f * hpbar + 6.0f) * M_PI / 180.0f) -
              0.20f * cosf((4.0f * hpbar - 63.0f) * M_PI / 180.0f);

    float dt = 30.0f * expf(-powf((hpbar - 275.0f) / 25.0f, 2.0f));
    float cpbar7 = powf(cpbar, 7.0f);
    float rc = 2.0f * sqrtf(cpbar7 / (cpbar7 + powf(25.0f, 7.0f)));

    float sl = 1.0f + (0.015f * powf(lbar - 50.0f, 2.0f)) / sqrtf(20.0f + powf(lbar - 50.0f, 2.0f));
    float sc = 1.0f + 0.045f * cpbar;
    float sh = 1.0f + 0.015f * cpbar * t;
    float rt = -sinf(2.0f * dt * M_PI / 180.0f) * rc;

    float kl = 1.0f, kc = 1.0f, kh = 1.0f;

    float dl_term = dl / (kl * sl);
    float dc_term = dc / (kc * sc);
    float dh_term = dh / (kh * sh);

    return sqrtf(dl_term*dl_term + dc_term*dc_term + dh_term*dh_term + rt * dc_term * dh_term);
}

// Levels adjustment: black point, white point, gamma
color_t color_levels(color_t color, float black_point, float white_point, float gamma) {
    color_t result;

    // Normalize inputs to 0-1 range
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    // Apply levels adjustment to each channel
    r = _CLAMP_FLOAT((r - black_point) / (white_point - black_point));
    g = _CLAMP_FLOAT((g - black_point) / (white_point - black_point));
    b = _CLAMP_FLOAT((b - black_point) / (white_point - black_point));

    // Apply gamma correction
    r = powf(r, 1.0f / gamma);
    g = powf(g, 1.0f / gamma);
    b = powf(b, 1.0f / gamma);

    result.r = _CLAMP_UINT8(r * 255.0f);
    result.g = _CLAMP_UINT8(g * 255.0f);
    result.b = _CLAMP_UINT8(b * 255.0f);
    result.a = color.a;

    return result;
}

// Curves adjustment using lookup tables
color_t color_curves(color_t color, float* curve_r, float* curve_g, float* curve_b) {
    color_t result;

    // Apply curves (assuming curves are 256-element arrays)
    result.r = _CLAMP_UINT8(curve_r[color.r] * 255.0f);
    result.g = _CLAMP_UINT8(curve_g[color.g] * 255.0f);
    result.b = _CLAMP_UINT8(curve_b[color.b] * 255.0f);
    result.a = color.a;

    return result;
}

// Shadow/Highlight adjustment
color_t color_shadow_highlight(color_t color, float shadow, float highlight) {
    color_t result;

    // Convert to 0-1 range
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    // Calculate luminance
    float lum = 0.299f * r + 0.587f * g + 0.114f * b;

    // Apply shadow/highlight adjustments
    float shadow_factor = 1.0f + shadow * (1.0f - lum);
    float highlight_factor = 1.0f + highlight * lum;

    r *= shadow_factor * highlight_factor;
    g *= shadow_factor * highlight_factor;
    b *= shadow_factor * highlight_factor;

    result.r = _CLAMP_UINT8(r * 255.0f);
    result.g = _CLAMP_UINT8(g * 255.0f);
    result.b = _CLAMP_UINT8(b * 255.0f);
    result.a = color.a;

    return result;
}

// Color balance adjustment
color_t color_color_balance(color_t color, float cyan_red, float magenta_green, float yellow_blue) {
    color_t result;

    // Convert to 0-1 range
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    // Apply color balance adjustments
    r += cyan_red;
    g += magenta_green;
    b += yellow_blue;

    result.r = _CLAMP_UINT8(r * 255.0f);
    result.g = _CLAMP_UINT8(g * 255.0f);
    result.b = _CLAMP_UINT8(b * 255.0f);
    result.a = color.a;

    return result;
}

// Vibrance adjustment (smart saturation)
color_t color_vibrance(color_t color, float amount) {
    color_t result;

    // Convert to HSV for saturation manipulation
    color_hsva_t hsv = rgba_to_hsva(color);

    // Calculate existing saturation level
    float sat_factor = 1.0f - hsv.s;

    // Apply vibrance (more effect on less saturated colors)
    hsv.s += amount * sat_factor;
    hsv.s = _CLAMP_FLOAT(hsv.s);

    result = hsva_to_rgba(hsv);
    result.a = color.a;

    return result;
}

// Selective color adjustment
color_t color_selective_color(color_t color, int channel, float cyan, float magenta, float yellow, float black) {
    color_t result = color;

    // Convert to CMY for selective color work
    float c = 1.0f - color.r / 255.0f;
    float m = 1.0f - color.g / 255.0f;
    float y = 1.0f - color.b / 255.0f;

    // Determine which color channel to adjust based on dominant color
    float max_cmy = (c > m) ? ((c > y) ? c : y) : ((m > y) ? m : y);

    // Apply selective adjustments based on channel
    switch (channel) {
        case 0: // Reds (low cyan)
            if (c < 0.5f) {
                c += cyan * (1.0f - c);
                m += magenta * (1.0f - m);
                y += yellow * (1.0f - y);
            }
            break;
        case 1: // Yellows (high yellow)
            if (y > 0.5f) {
                c += cyan * (1.0f - c);
                m += magenta * (1.0f - m);
                y += yellow * (1.0f - y);
            }
            break;
        case 2: // Greens (high yellow, low magenta)
            if (y > 0.3f && m < 0.5f) {
                c += cyan * (1.0f - c);
                m += magenta * (1.0f - m);
                y += yellow * (1.0f - y);
            }
            break;
        case 3: // Cyans (high cyan)
            if (c > 0.5f) {
                c += cyan * (1.0f - c);
                m += magenta * (1.0f - m);
                y += yellow * (1.0f - y);
            }
            break;
        case 4: // Blues (high cyan, high magenta)
            if (c > 0.3f && m > 0.3f) {
                c += cyan * (1.0f - c);
                m += magenta * (1.0f - m);
                y += yellow * (1.0f - y);
            }
            break;
        case 5: // Magentas (high magenta)
            if (m > 0.5f) {
                c += cyan * (1.0f - c);
                m += magenta * (1.0f - m);
                y += yellow * (1.0f - y);
            }
            break;
    }

    // Apply black adjustment
    float k = black;
    c = _CLAMP_FLOAT(c + k);
    m = _CLAMP_FLOAT(m + k);
    y = _CLAMP_FLOAT(y + k);

    // Convert back to RGB
    result.r = _CLAMP_UINT8((1.0f - c) * 255.0f);
    result.g = _CLAMP_UINT8((1.0f - m) * 255.0f);
    result.b = _CLAMP_UINT8((1.0f - y) * 255.0f);

    return result;
}

// Quantize color to fewer bits per channel
color_t color_quantize(color_t color, int bits_per_channel) {
    color_t result;

    int levels = (1 << bits_per_channel) - 1;
    float scale = 255.0f / levels;

    result.r = (uint8_t)((color.r / (int)scale) * scale);
    result.g = (uint8_t)((color.g / (int)scale) * scale);
    result.b = (uint8_t)((color.b / (int)scale) * scale);
    result.a = color.a;

    return result;
}

// Posterize to specific number of levels
color_t color_posterize(color_t color, int levels) {
    color_t result;

    float scale = 255.0f / (levels - 1);

    result.r = (uint8_t)((int)(color.r / scale) * scale);
    result.g = (uint8_t)((int)(color.g / scale) * scale);
    result.b = (uint8_t)((int)(color.b / scale) * scale);
    result.a = color.a;

    return result;
}

// Find closest color in palette
static int find_closest_color(color_t color, color_t* palette, int palette_size) {
    int closest = 0;
    float min_distance = color_distance(color, palette[0]);

    for (int i = 1; i < palette_size; i++) {
        float distance = color_distance(color, palette[i]);
        if (distance < min_distance) {
            min_distance = distance;
            closest = i;
        }
    }

    return closest;
}

// Floyd-Steinberg dithering
color_t color_dither_floyd_steinberg(color_t color, color_t* palette, int palette_size, int x, int y) {
    // Find closest palette color
    int closest_idx = find_closest_color(color, palette, palette_size);
    color_t closest = palette[closest_idx];

    // Calculate error (this would normally be distributed to neighboring pixels)
    // For a single pixel function, we just return the closest color
    return closest;
}

// Ordered dithering using Bayer matrix
color_t color_dither_ordered(color_t color, color_t* palette, int palette_size, int x, int y) {
    // 4x4 Bayer matrix
    static const int bayer_matrix[4][4] = {
        { 0,  8,  2, 10},
        {12,  4, 14,  6},
        { 3, 11,  1,  9},
        {15,  7, 13,  5}
    };

    // Get threshold from Bayer matrix
    int threshold = bayer_matrix[y % 4][x % 4];
    float dither_value = (threshold / 15.0f - 0.5f) * 32.0f; // Scale dither amount

    // Apply dither to color
    color_t dithered;
    dithered.r = _CLAMP_UINT8(color.r + dither_value);
    dithered.g = _CLAMP_UINT8(color.g + dither_value);
    dithered.b = _CLAMP_UINT8(color.b + dither_value);
    dithered.a = color.a;

    // Find closest palette color
    int closest_idx = find_closest_color(dithered, palette, palette_size);
    return palette[closest_idx];
}

// Protanopia (red-blind) - affects L-cones
// Uses Brettel, Viénot and Mollon transformation matrix
color_t color_protanopia(color_t color) {
    // Transformation matrix for protanopia
    // Row 1: [0.567, 0.433, 0.000]
    // Row 2: [0.558, 0.442, 0.000]
    // Row 3: [0.000, 0.242, 0.758]

    int r = (567 * color.r + 433 * color.g) / 1000;
    int g = (558 * color.r + 442 * color.g) / 1000;
    int b = (242 * color.g + 758 * color.b) / 1000;

    color_t result;
    result.r = _CLAMP_UINT8(r);
    result.g = _CLAMP_UINT8(g);
    result.b = _CLAMP_UINT8(b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Deuteranopia (green-blind) - affects M-cones
// Uses Brettel, Viénot and Mollon transformation matrix
color_t color_deuteranopia(color_t color) {
    // Transformation matrix for deuteranopia
    // Row 1: [0.625, 0.375, 0.000]
    // Row 2: [0.700, 0.300, 0.000]
    // Row 3: [0.000, 0.300, 0.700]

    int r = (625 * color.r + 375 * color.g) / 1000;
    int g = (700 * color.r + 300 * color.g) / 1000;
    int b = (300 * color.g + 700 * color.b) / 1000;

    color_t result;
    result.r = _CLAMP_UINT8(r);
    result.g = _CLAMP_UINT8(g);
    result.b = _CLAMP_UINT8(b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Tritanopia (blue-blind) - affects S-cones
// Uses Brettel, Viénot and Mollon transformation matrix
color_t color_tritanopia(color_t color) {
    // Transformation matrix for tritanopia
    // Row 1: [0.950, 0.050, 0.000]
    // Row 2: [0.000, 0.433, 0.567]
    // Row 3: [0.000, 0.475, 0.525]

    int r = (950 * color.r + 50 * color.g) / 1000;
    int g = (433 * color.g + 567 * color.b) / 1000;
    int b = (475 * color.g + 525 * color.b) / 1000;

    color_t result;
    result.r = _CLAMP_UINT8(r);
    result.g = _CLAMP_UINT8(g);
    result.b = _CLAMP_UINT8(b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Achromatopsia (complete color blindness) - monochromacy
// Converts to grayscale using luminance formula
color_t color_achromatopsia(color_t color) {
    // Standard luminance formula: 0.299*R + 0.587*G + 0.114*B
    // Using integer arithmetic: (299*R + 587*G + 114*B) / 1000

    int luminance = (299 * color.r + 587 * color.g + 114 * color.b) / 1000;
    uint8_t gray = _CLAMP_UINT8(luminance);

    color_t result;
    result.r = gray;
    result.g = gray;
    result.b = gray;
    result.a = color.a;  // Preserve alpha

    return result;
}

// Helper function to calculate relative luminance according to WCAG
// Uses the sRGB color space formula
static float relative_luminance(color_t color) {
    // Convert RGB values to 0-1 range
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    // Apply gamma correction (sRGB to linear RGB)
    r = (r <= 0.03928f) ? r / 12.92f : powf((r + 0.055f) / 1.055f, 2.4f);
    g = (g <= 0.03928f) ? g / 12.92f : powf((g + 0.055f) / 1.055f, 2.4f);
    b = (b <= 0.03928f) ? b / 12.92f : powf((b + 0.055f) / 1.055f, 2.4f);

    // Calculate relative luminance using ITU-R BT.709 coefficients
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Calculate WCAG contrast ratio between two colors
float color_contrast_ratio(color_t a, color_t b) {
    float lum_a = relative_luminance(a);
    float lum_b = relative_luminance(b);

    // Ensure lighter color is in numerator
    float lighter = (lum_a > lum_b) ? lum_a : lum_b;
    float darker = (lum_a > lum_b) ? lum_b : lum_a;

    // WCAG contrast ratio formula: (L1 + 0.05) / (L2 + 0.05)
    return (lighter + 0.05f) / (darker + 0.05f);
}

// Check if color combination meets WCAG AA compliance
// AA requires contrast ratio of at least 4.5:1 for normal text, 3:1 for large text
int color_wcag_aa_compliant(color_t fg, color_t bg) {
    float ratio = color_contrast_ratio(fg, bg);
    return ratio >= 4.5f;
}

// Check if color combination meets WCAG AAA compliance
// AAA requires contrast ratio of at least 7:1 for normal text, 4.5:1 for large text
int color_wcag_aaa_compliant(color_t fg, color_t bg) {
    float ratio = color_contrast_ratio(fg, bg);
    return ratio >= 7.0f;
}

// Find the closest color in a palette to a target color
color_t color_match_closest(color_t target, color_t* palette, int palette_size) {
    if (palette_size <= 0 || palette == NULL) {
        return target;  // Return target if no palette provided
    }

    color_t closest = palette[0];
    float min_distance = color_distance(target, palette[0]);

    for (int i = 1; i < palette_size; i++) {
        float distance = color_distance(target, palette[i]);
        if (distance < min_distance) {
            min_distance = distance;
            closest = palette[i];
        }
    }

    return closest;
}

// Calculate color similarity as a normalized value (0.0 = identical, 1.0 = maximum difference)
float color_similarity(color_t a, color_t b) {
    // Use Euclidean distance in RGB space, normalized to 0-1 range
    float dr = (a.r - b.r) / 255.0f;
    float dg = (a.g - b.g) / 255.0f;
    float db = (a.b - b.b) / 255.0f;

    // Calculate distance and normalize by maximum possible distance (sqrt(3))
    float distance = sqrtf(dr * dr + dg * dg + db * db);
    return distance / sqrtf(3.0f);
}

// Check if two colors are similar within a given threshold
// threshold: 0.0 (identical) to 1.0 (maximum difference)
int color_is_similar(color_t a, color_t b, float threshold) {
    return color_similarity(a, b) <= threshold;
}

#define _C2F(V) ((float)(V) / 255.0f)
#define _F2C(V) ((uint8_t)((V) * 255.0f + 0.5f))

// Lift-Gamma-Gain color grading (common in film/video post-production)
// lift: affects shadows/blacks, gamma: affects midtones, gain: affects highlights/whites
color_t color_lift_gamma_gain(color_t color, color_rgbaf_t lift, color_rgbaf_t gamma, color_rgbaf_t gain) {
    // Convert to 0-1 range
    float r = _C2F(color.r);
    float g = _C2F(color.g);
    float b = _C2F(color.b);

    // Apply lift (additive in shadows)
    r = r + lift.r * (1.0f - r);
    g = g + lift.g * (1.0f - g);
    b = b + lift.b * (1.0f - b);

    // Apply gamma (power function for midtones)
    if (gamma.r != 0.0f) r = powf(r, 1.0f / gamma.r);
    if (gamma.g != 0.0f) g = powf(g, 1.0f / gamma.g);
    if (gamma.b != 0.0f) b = powf(b, 1.0f / gamma.b);

    // Apply gain (multiplicative in highlights)
    r *= gain.r;
    g *= gain.g;
    b *= gain.b;

    color_t result;
    result.r = _F2C(r);
    result.g = _F2C(g);
    result.b = _F2C(b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Shadow-Midtone-Highlight color grading (alternative approach)
// Uses luminance-based weighting to apply corrections
color_t color_shadow_midtone_highlight(color_t color, color_rgbaf_t shadow, color_rgbaf_t midtone, color_rgbaf_t highlight) {
    // Convert to 0-1 range
    float r = _C2F(color.r);
    float g = _C2F(color.g);
    float b = _C2F(color.b);

    // Calculate luminance for weighting
    float luminance = 0.299f * r + 0.587f * g + 0.114f * b;

    // Create weighting functions for shadow, midtone, and highlight
    // Shadow weight: stronger in dark areas
    float shadow_weight = 1.0f - powf(luminance, 2.0f);

    // Midtone weight: bell curve centered at 0.5
    float midtone_weight = 4.0f * luminance * (1.0f - luminance);

    // Highlight weight: stronger in bright areas
    float highlight_weight = powf(luminance, 2.0f);

    // Apply weighted color corrections
    r += shadow.r * shadow_weight + midtone.r * midtone_weight + highlight.r * highlight_weight;
    g += shadow.g * shadow_weight + midtone.g * midtone_weight + highlight.g * highlight_weight;
    b += shadow.b * shadow_weight + midtone.b * midtone_weight + highlight.b * highlight_weight;

    color_t result;
    result.r = _F2C(r);
    result.g = _F2C(g);
    result.b = _F2C(b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Helper function to apply 3x3 color space transformation matrix
static color_t apply_color_matrix(color_t color, const float matrix[3][3]) {
    // Convert to 0-1 range
    float r = _C2F(color.r);
    float g = _C2F(color.g);
    float b = _C2F(color.b);

    // Apply gamma correction (sRGB to linear) before color space conversion
    r = (r <= 0.04045f) ? r / 12.92f : powf((r + 0.055f) / 1.055f, 2.4f);
    g = (g <= 0.04045f) ? g / 12.92f : powf((g + 0.055f) / 1.055f, 2.4f);
    b = (b <= 0.04045f) ? b / 12.92f : powf((b + 0.055f) / 1.055f, 2.4f);

    // Apply matrix transformation
    float new_r = matrix[0][0] * r + matrix[0][1] * g + matrix[0][2] * b;
    float new_g = matrix[1][0] * r + matrix[1][1] * g + matrix[1][2] * b;
    float new_b = matrix[2][0] * r + matrix[2][1] * g + matrix[2][2] * b;

    // Apply inverse gamma correction (linear to sRGB)
    new_r = (new_r <= 0.0031308f) ? new_r * 12.92f : 1.055f * powf(new_r, 1.0f / 2.4f) - 0.055f;
    new_g = (new_g <= 0.0031308f) ? new_g * 12.92f : 1.055f * powf(new_g, 1.0f / 2.4f) - 0.055f;
    new_b = (new_b <= 0.0031308f) ? new_b * 12.92f : 1.055f * powf(new_b, 1.0f / 2.4f) - 0.055f;

    color_t result;
    result.r = _F2C(new_r);
    result.g = _F2C(new_g);
    result.b = _F2C(new_b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Convert Rec.709 (standard HD) to Rec.2020 (ultra HD/HDR)
// Used in broadcast and streaming for HDR content
color_t color_rec709_to_rec2020(color_t color) {
    // Transformation matrix from Rec.709 to Rec.2020
    // Based on ITU-R BT.2087 standard
    static const float matrix[3][3] = {
        {0.627404f, 0.329283f, 0.043313f},
        {0.069097f, 0.919540f, 0.011362f},
        {0.016391f, 0.088013f, 0.895595f}
    };

    return apply_color_matrix(color, matrix);
}

// Convert ProPhoto RGB to sRGB
// ProPhoto RGB has a much wider gamut, commonly used in professional photography
color_t color_prophoto_to_srgb(color_t color) {
    // Convert to 0-1 range
    float r = _C2F(color.r);
    float g = _C2F(color.g);
    float b = _C2F(color.b);

    // Apply ProPhoto RGB gamma correction (gamma 1.8) to linear
    r = powf(r, 1.8f);
    g = powf(g, 1.8f);
    b = powf(b, 1.8f);

    // ProPhoto RGB to XYZ transformation matrix
    float x = 0.7976749f * r + 0.1351917f * g + 0.0313534f * b;
    float y = 0.2880402f * r + 0.7118741f * g + 0.0000857f * b;
    float z = 0.0000000f * r + 0.0000000f * g + 0.8252100f * b;

    // XYZ to sRGB transformation matrix
    float new_r = 3.2404542f * x - 1.5371385f * y - 0.4985314f * z;
    float new_g = -0.9692660f * x + 1.8760108f * y + 0.0415560f * z;
    float new_b = 0.0556434f * x - 0.2040259f * y + 1.0572252f * z;

    // Apply sRGB gamma correction (linear to sRGB)
    new_r = (new_r <= 0.0031308f) ? new_r * 12.92f : 1.055f * powf(new_r, 1.0f / 2.4f) - 0.055f;
    new_g = (new_g <= 0.0031308f) ? new_g * 12.92f : 1.055f * powf(new_g, 1.0f / 2.4f) - 0.055f;
    new_b = (new_b <= 0.0031308f) ? new_b * 12.92f : 1.055f * powf(new_b, 1.0f / 2.4f) - 0.055f;

    color_t result;
    result.r = _F2C(new_r);
    result.g = _F2C(new_g);
    result.b = _F2C(new_b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Convert Adobe RGB to sRGB
// Adobe RGB has wider gamut than sRGB, commonly used in print and professional workflows
color_t color_adobe_rgb_to_srgb(color_t color) {
    // Convert to 0-1 range
    float r = _C2F(color.r);
    float g = _C2F(color.g);
    float b = _C2F(color.b);

    // Apply Adobe RGB gamma correction (gamma 2.2) to linear
    r = powf(r, 2.2f);
    g = powf(g, 2.2f);
    b = powf(b, 2.2f);

    // Adobe RGB to XYZ transformation matrix
    float x = 0.5767309f * r + 0.1855540f * g + 0.1881852f * b;
    float y = 0.2973769f * r + 0.6273491f * g + 0.0752741f * b;
    float z = 0.0270343f * r + 0.0706872f * g + 0.9911085f * b;

    // XYZ to sRGB transformation matrix
    float new_r = 3.2404542f * x - 1.5371385f * y - 0.4985314f * z;
    float new_g = -0.9692660f * x + 1.8760108f * y + 0.0415560f * z;
    float new_b = 0.0556434f * x - 0.2040259f * y + 1.0572252f * z;

    // Apply sRGB gamma correction (linear to sRGB)
    new_r = (new_r <= 0.0031308f) ? new_r * 12.92f : 1.055f * powf(new_r, 1.0f / 2.4f) - 0.055f;
    new_g = (new_g <= 0.0031308f) ? new_g * 12.92f : 1.055f * powf(new_g, 1.0f / 2.4f) - 0.055f;
    new_b = (new_b <= 0.0031308f) ? new_b * 12.92f : 1.055f * powf(new_b, 1.0f / 2.4f) - 0.055f;

    color_t result;
    result.r = _F2C(new_r);
    result.g = _F2C(new_g);
    result.b = _F2C(new_b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Fast grayscale conversion using integer arithmetic only
// Uses approximate luminance weights for speed
color_t color_fast_grayscale(color_t color) {
    // Fast integer approximation: (R + G + B) / 3
    // Or slightly better: (R*77 + G*151 + B*28) >> 8 (approximates 0.299, 0.587, 0.114)
    uint8_t gray = (uint8_t)((77 * color.r + 151 * color.g + 28 * color.b) >> 8);

    color_t result;
    result.r = gray;
    result.g = gray;
    result.b = gray;
    result.a = color.a;  // Preserve alpha

    return result;
}

// Fast sepia tone effect using integer arithmetic
// Creates warm brown tones typical of old photographs
color_t color_fast_sepia(color_t color) {
    // Calculate sepia values using integer arithmetic
    // Standard sepia transformation with bit shifting for speed
    int r = (color.r * 393 + color.g * 769 + color.b * 189) >> 10;  // / 1024
    int g = (color.r * 349 + color.g * 686 + color.b * 168) >> 10;  // / 1024
    int b = (color.r * 272 + color.g * 534 + color.b * 131) >> 10;  // / 1024

    color_t result;
    result.r = _CLAMP_UINT8(r);
    result.g = _CLAMP_UINT8(g);
    result.b = _CLAMP_UINT8(b);
    result.a = color.a;  // Preserve alpha

    return result;
}

// Fast color inversion using bitwise operations
// Simply inverts each color component
color_t color_fast_invert(color_t color) {
    color_t result;
    result.r = 255 - color.r;  // Equivalent to: color.r ^ 0xFF
    result.g = 255 - color.g;  // Equivalent to: color.g ^ 0xFF
    result.b = 255 - color.b;  // Equivalent to: color.b ^ 0xFF
    result.a = color.a;        // Preserve alpha

    return result;
}

static color_t* bitmap_make(unsigned int w, unsigned int h) {
    static_assert(sizeof(color_t) == sizeof(uint32_t), "color_t must be 4 bytes");
    uint32_t *buffer = (uint32_t*)malloc((w * h + 2) * sizeof(uint32_t));
    if (!buffer)
        return NULL;
    buffer[0] = w;
    buffer[1] = h;
    return (color_t*)(buffer + 2);
}

bitmap_t bitmap_empty(unsigned int w, unsigned int h, color_t color) {
    if (w <= 0 || h <= 0)
        return NULL;
    bitmap_t result = bitmap_make(w, h);
    if (!result)
        return NULL;
    bitmap_fill(result, _black);
    return result;
}

static uint32_t* _raw(bitmap_t img) {
    return img ? (uint32_t*)(img) - 2 : NULL;
}

void bitmap_destroy(bitmap_t img) {
    uint32_t *raw = _raw(img);
    if (raw)
        free(raw);
}

int bitmap_width(const bitmap_t img) {
    uint32_t *raw = _raw(img);
    return raw ? raw[0] : 0;
}

int bitmap_height(const bitmap_t img) {
    uint32_t *raw = _raw(img);
    return raw ? raw[1] : 0;
}

bool bitmap_size(const bitmap_t img, int *w, int *h) {
    if (!img || (!w && !h))
        return false;
    uint32_t *raw = _raw(img);
    int _w = raw ? raw[0] : 0;
    int _h = raw ? raw[1] : 0;
    if (w)
        *w = _w;
    if (h)
        *h = _h;
    return true;
}

bool bitmap_pset(bitmap_t img, int x, int y, color_t color) {
    if (!img || x < 0 || y < 0)
        return false;
    int w, h;
    bitmap_size(img, &w, &h);
    if (x >= w || y >= h || w <= 0 || h <= 0)
        return false;
    img[y * w + x] = color;
    return true;
}

color_t bitmap_pget(const bitmap_t img, int x, int y) {
    if (!img || x < 0 || y < 0)
        return _black;
    int w, h;
    bitmap_size(img, &w, &h);
    if (x >= w || y >= h || w <= 0 || h <= 0)
        return _black;
    return img[y * w + x];
}

bool bitmap_fill(bitmap_t img, color_t color) {
    if (!img)
        return false;
    int w, h;
    bitmap_size(img, &w, &h);
    for (int i = 0; i < w * h; ++i)
        img[i] = color;
    return true;
}

bool color_cmp(color_t a, color_t b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static inline void _flood(bitmap_t img, int x, int y, color_t _new, color_t _old) {
    if (color_cmp(_new, _old) || !color_cmp(bitmap_pget(img, x, y), _old))
        return;

    int x1 = x;
    int w = bitmap_width(img);
    while (x1 < w && color_cmp(bitmap_pget(img, x1, y), _old)) {
        bitmap_pset(img, x1, y, _new);
        x1++;
    }

    x1 = x - 1;
    while (x1 >= 0 && color_cmp(bitmap_pget(img, x1, y), _old)) {
        bitmap_pset(img, x1, y, _new);
        x1--;
    }

    x1 = x;
    while (x1 < w && color_cmp(bitmap_pget(img, x1, y), _old)) {
        if(y > 0 && color_cmp(bitmap_pget(img, x1, y - 1), _old))
            _flood(img, x1, y - 1, _new, _old);
        x1++;
    }

    x1 = x - 1;
    while(x1 >= 0 && color_cmp(bitmap_pget(img, x1, y), _old)) {
        if(y > 0 && color_cmp(bitmap_pget(img, x1, y - 1), _old))
            _flood(img, x1, y - 1, _new, _old);
        x1--;
    }

    x1 = x;
    int h = bitmap_height(img);
    while(x1 < w && color_cmp(bitmap_pget(img, x1, y), _old)) {
        if(y < h - 1 && color_cmp(bitmap_pget(img, x1, y + 1), _old))
            _flood(img, x1, y + 1, _new, _old);
        x1++;
    }

    x1 = x - 1;
    while(x1 >= 0 && color_cmp(bitmap_pget(img, x1, y), _old)) {
        if(y < h - 1 && color_cmp(bitmap_pget(img, x1, y + 1), _old))
            _flood(img, x1, y + 1, _new, _old);
        x1--;
    }
}

bool bitmap_flood(bitmap_t img, int x, int y, color_t color) {
    if (!img || x < 0 || y < 0)
        return false;
    int w, h;
    bitmap_size(img, &w, &h);
    if (x >= w || y >= h || w <= 0 || h <= 0)
        return false;
    _flood(img, x, y, color, bitmap_pget(img, x, y));
    return true;
}

bool bitmap_paste(bitmap_t dst, const bitmap_t src, int x, int y) {
    if (!src || !dst || x < 0 || y < 0)
        return false;
    int w, h;
    bitmap_size(src, &w, &h);
    int dw, dh;
    bitmap_size(dst, &dw, &dh);
    if (x >= dw || y >= dh || dw <= 0 || dh <= 0)
        return false;
    for (int ox = 0; ox < w; ++ox) {
        for (int oy = 0; oy < h; ++oy) {
            if (oy > h)
                break;
            bitmap_pset(dst, x + ox, y + oy, bitmap_pget(src, ox, oy));
        }
        if (ox > w)
            break;
    }
    return true;
}

bool bitmap_clipped_paste(bitmap_t dst, const bitmap_t src, int x, int y, int rx, int ry, int rw, int rh) {
    if (!src || !dst || x < 0 || y < 0 || rw <= 0 || rh <= 0 || rx < 0 || ry < 0)
        return false;
    int w, h;
    bitmap_size(src, &w, &h);
    if (rx + rw > w || ry + rh > h)
        return false;
    if (x + rw > bitmap_width(dst) || y + rh > bitmap_height(dst))
        return false;
    int dw, dh;
    bitmap_size(dst, &dw, &dh);
    if (x > dw || y > dh)
        return false;
    for (int ox = 0; ox < rw; ++ox)
        for (int oy = 0; oy < rh; ++oy)
            bitmap_pset(dst, ox + x, oy + y, bitmap_pget(src, ox + rx, oy + ry));
    return true;
}

bitmap_t bitmap_dupe(bitmap_t src) {
    if (!src)
        return NULL;
    int w, h;
    bitmap_size(src, &w, &h);
    bitmap_t result = bitmap_make(w, h);
    if (!result)
        return NULL;
    memcpy(result, src, w * h * sizeof(uint32_t));
    return result;
}

bitmap_t bitmap_resized(bitmap_t src, int nw, int nh) {
    if (!src || nw <= 0 || nh <= 0)
        return NULL;
    int w, h;
    bitmap_size(src, &w, &h);
    bitmap_t result = bitmap_make(nw, nh);
    if (!result)
        return false;
    int x_ratio = (int)((w << 16) / nw) + 1;
    int y_ratio = (int)((h << 16) / nh) + 1;
    int x2, y2, i, j;
    for (i = 0; i < nh; ++i) {
        color_t *t = result + i * nw;
        y2 = ((i * y_ratio) >> 16);
        color_t *p = src + y2 * w;
        int rat = 0;
        for (j = 0; j < nw; ++j) {
            x2 = (rat >> 16);
            *t++ = p[x2];
            rat += x_ratio;
        }
    }
    return result;
}

bool bitmap_resize(bitmap_t *src, int nw, int nh) {
    if (!src || !*src || nw < 0 || nh < 0)
        return false;
    int w, h;
    bitmap_size(*src, &w, &h);
    if (w == nw && h == nh)
        return false;
    bitmap_t result = bitmap_resized(*src, nw, nh);
    if (result)
        return false;
    bitmap_destroy(*src);
    *src = result;
    return true;
}

bitmap_t bitmap_rotated(bitmap_t src, float angle) {
    if (!src)
        return NULL;
    int w, h;
    bitmap_size(src, &w, &h);

    float theta = _RADIANS(angle);
    float c = cosf(theta), s = sinf(theta);
    float r[3][2] = {
        { -h * s, h * c },
        {  w * c - h * s, h * c + w * s },
        {  w * c, w * s }
    };

    float mm[2][2] = {{
        _MIN(0, _MIN(r[0][0], _MIN(r[1][0], r[2][0]))),
        _MIN(0, _MIN(r[0][1], _MIN(r[1][1], r[2][1])))
    }, {
        (theta > 1.5708  && theta < 3.14159 ? 0.f : _MAX(r[0][0], _MAX(r[1][0], r[2][0]))),
        (theta > 3.14159 && theta < 4.71239 ? 0.f : _MAX(r[0][1], _MAX(r[1][1], r[2][1])))
    }};

    int dw = (int)ceil(fabsf(mm[1][0]) - mm[0][0]);
    int dh = (int)ceil(fabsf(mm[1][1]) - mm[0][1]);
    bitmap_t result = bitmap_make(dw, dh);
    if (!result)
        return NULL;
    int x, y, sx, sy;
    for (x = 0; x < dw; ++x)
        for (y = 0; y < dh; ++y) {
            sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
            sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
            if (sx < 0 || sx >= w || sy < 0 || sy >= h)
                continue;
            bitmap_pset(result, x, y, bitmap_pget(src, sx, sy));
        }
    return result;
}

bool bitmap_rotate(bitmap_t *src, float angle) {
    if (!src || !*src)
        return false;
    int w, h;
    bitmap_size(*src, &w, &h);
    bitmap_t result = bitmap_rotated(*src, angle);
    if (!result)
        return false;
    bitmap_destroy(*src);
    *src = result;
    return true;
}

bitmap_t bitmap_clipped(bitmap_t src, int rx, int ry, int rw, int rh) {
    if (!src || rw <= 0 || rh <= 0 || rx < 0 || ry < 0)
        return NULL;
    int w, h;
    bitmap_size(src, &w, &h);
    if (rx >= w || ry >= h || rw <= 0 || rh <= 0)
        return NULL;

    int ox = _CLAMP(rx, 0, w);
    int oy = _CLAMP(ry, 0, h);
    if (ox >= w || oy >= h)
        return NULL;
    int mx = _MIN(ox + rw, w);
    int my = _MIN(oy + rh, h);
    int iw = mx - ox;
    int ih = my - oy;
    if (iw <= 0 || ih <= 0)
        return NULL;
    color_t *result = bitmap_make(rw, rh);
    if (!result)
        return NULL;
    for (int px = 0; px < iw; px++)
        for (int py = 0; py < ih; py++) {
            int cx = ox + px;
            int cy = oy + py;
            bitmap_pset(result, px, py, bitmap_pget(src, cx, cy));
        }
    return result;
}

bool bitmap_clip(bitmap_t *src, int rx, int ry, int rw, int rh) {
    if (!src || !*src || rw <= 0 || rh <= 0 || rx < 0 || ry < 0)
        return false;
    int w, h;
    bitmap_size(*src, &w, &h);
    if (rx >= w || ry >= h || rw <= 0 || rh <= 0)
        return false;
    bitmap_t result = bitmap_clipped(*src, rx, ry, rw, rh);
    if (!result)
        return false;
    bitmap_destroy(*src);
    *src = result;
    return true;
}

static inline void vline(bitmap_t img, int w, int h, int x, int y0, int y1, color_t color) {
    if (y1 < y0) {
        y0 += y1;
        y1  = y0 - y1;
        y0 -= y1;
    }

    if (x < 0 || x >= w || y0 >= h)
        return;

    if (y0 < 0)
        y0 = 0;
    if (y1 >= h)
        y1 = h - 1;

    for(int y = y0; y <= y1; y++)
        bitmap_pset(img, x, y, color);
}

static inline void hline(bitmap_t img, int w, int h, int y, int x0, int x1, color_t color) {
    if (x1 < x0) {
        x0 += x1;
        x1  = x0 - x1;
        x0 -= x1;
    }

    if (y < 0 || y >= h || x0 >= w)
        return;

    if (x0 < 0)
        x0 = 0;
    if (x1 >= w)
        x1 = w - 1;

    for(int x = x0; x <= x1; x++)
        bitmap_pset(img, x, y, color);
}

bool bitmap_draw_line(bitmap_t img, int x0, int y0, int x1, int y1, color_t color) {
    if (!img || x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0)
        return false;
    int w, h;
    bitmap_size(img, &w, &h);
    if (x0 >= w || y0 >= h || x1 >= w || y1 >= h || w <= 0 || h <= 0)
        return false;

    if (x0 == x1)
        vline(img, w, h, x0, y0, y1, color);
    else if (y0 == y1)
        hline(img, w, h, y0, x0, x1, color);
    else {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = (dx > dy ? dx : -dy) / 2;

        while (bitmap_pset(img, x0, y0, color), x0 != x1 || y0 != y1) {
            int e2 = err;
            if (e2 > -dx) { err -= dy; x0 += sx; }
            if (e2 <  dy) { err += dx; y0 += sy; }
        }
    }

    return true;
}

bool bitmap_draw_circle(bitmap_t img, int xc, int yc, int r, color_t color, int fill) {
    if (!img || xc < 0 || yc < 0 || r <= 0)
        return false;
    int w, h;
    bitmap_size(img, &w, &h);
    if (xc >= w || yc >= h || xc < 0 || yc < 0 || w <= 0 || h <= 0)
        return false;

    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        bitmap_pset(img, xc - x, yc + y, color);    /*   I. Quadrant */
        bitmap_pset(img, xc - y, yc - x, color);    /*  II. Quadrant */
        bitmap_pset(img, xc + x, yc - y, color);    /* III. Quadrant */
        bitmap_pset(img, xc + y, yc + x, color);    /*  IV. Quadrant */

        if (fill) {
            hline(img, w, h, yc - y, xc - x, xc + x, color);
            hline(img, w, h, yc + y, xc - x, xc + x, color);
        }

        r = err;
        if (r <= y)
            err += ++y * 2 + 1; /* e_xy+e_y < 0 */
        if (r > x || err > y)
            err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);

    return true;
}

bool bitmap_draw_rectangle(bitmap_t img, int x, int y, int w, int h, color_t color, int fill) {
    if (!img || w <= 0 || h <= 0)
        return false;
    int img_w, img_h;
    bitmap_size(img, &img_w, &img_h);
    if (x >= img_w || y >= img_h || x < 0 || y < 0 || img_w <= 0 || img_h <= 0)
        return false;

    if (x < 0) {
        w += x;
        x  = 0;
    }
    if (y < 0) {
        h += y;
        y  = 0;
    }

    w += x;
    h += y;
    if (w < 0 || h < 0 || x > img_w || y > img_h)
        return false;

    if (w > img_w)
        w = img_w;
    if (h > img_h)
        h = img_h;

    if (fill) {
        for (; y < h; ++y)
            hline(img, img_w, img_h, y, x, w, color);
    } else {
        hline(img, img_w, img_h, y, x, w, color);
        hline(img, img_w, img_h, h, x, w, color);
        vline(img, img_w, img_h, x, y, h, color);
        vline(img, img_w, img_h, w, y, h, color);
    }

    return true;
}

bool bitmap_draw_triangle(bitmap_t img, int x0, int y0, int x1, int y1, int x2, int y2, color_t color, int fill) {
    if (!img || x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
        return false;
    int w, h;
    bitmap_size(img, &w, &h);
    if (x0 >= w || y0 >= h || x1 >= w || y1 >= h || x2 >= w || y2 >= h || w <= 0 || h <= 0)
        return false;

    if (y0 ==  y1 && y0 ==  y2)
        return false;
    if (fill) {
        if (y0 > y1) {
            _SWAP(x0, x1);
            _SWAP(y0, y1);
        }
        if (y0 > y2) {
            _SWAP(x0, x2);
            _SWAP(y0, y2);
        }
        if (y1 > y2) {
            _SWAP(x1, x2);
            _SWAP(y1, y2);
        }

        int total_height = y2 - y0, i, j;
        for (i = 0; i < total_height; ++i) {
            int second_half = i > y1 - y0 || y1 == y0;
            int segment_height = second_half ? y2 - y1 : y1 - y0;
            float alpha = (float)i / total_height;
            float beta  = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;
            int ax = x0 + (x2 - x0) * alpha;
            int ay = y0 + (y2 - y0) * alpha;
            int bx = second_half ? x1 + (x2 - x1) : x0 + (x1 - x0) * beta;
            int by = second_half ? y1 + (y2 - y1) : y0 + (y1 - y0) * beta;
            if (ax > bx) {
                _SWAP(ax, bx);
                _SWAP(ay, by);
            }
            for (j = ax; j <= bx; ++j)
                bitmap_pset(img, j, y0 + i, color);
        }
    } else {
        bitmap_draw_line(img, x0, y0, x1, y1, color);
        bitmap_draw_line(img, x1, y1, x2, y2, color);
        bitmap_draw_line(img, x2, y2, x0, y0, color);
    }

    return true;
}

color_t bitmap_dominant_color(const bitmap_t img) {
    if (!img)
        return _black;
    
    int w, h;
    bitmap_size(img, &w, &h);
    int count = w * h;
    if (count <= 0)
        return _black;
    
    int max_count = 0;
    int dominant_index = -1;
    // Simple O(n²) approach - for better performance with large arrays,
    // consider using a hash table or sorting approach
    for (int i = 0; i < count; i++) {
        int current_count = 0;
        // Count occurrences of img[i]
        for (int j = 0; j < count; j++)
            if (img[i].rgba == img[j].rgba)
                current_count++;
        if (current_count > max_count) {
            max_count = current_count;
            dominant_index = i;
        }
    }
    return dominant_index < 0 ? _black : img[dominant_index];
}

int* bitmap_histogram(const bitmap_t img) {
    if (!img)
        return NULL;
    
    int w, h;
    bitmap_size(img, &w, &h);
    int count = w * h;
    if (count <= 0)
        return NULL;
    
    // Create a simple histogram array for each possible RGB value
    // This is a simplified version - a real histogram might be more complex
    int* histogram = (int*)calloc(256 * 3, sizeof(int)); // R, G, B histograms
    if (!histogram)
        return NULL;
    
    for (int i = 0; i < count; i++) {
        histogram[img[i].r]++;                    // Red histogram
        histogram[256 + img[i].g]++;              // Green histogram  
        histogram[512 + img[i].b]++;              // Blue histogram
    }
    
    return histogram;
}

color_t* bitmap_palette(const bitmap_t img, int count) {
    if (!img || count <= 0)
        return NULL;

    int w, h;
    bitmap_size(img, &w, &h);
    size_t img_size = w * h;
    if (img_size <= 0)
        return NULL;
    
    color_t *palette = (color_t*)malloc(count * sizeof(color_t));
    if (!palette)
        return NULL;
    
    if (count > img_size) {
        for (int i = 0; i < count; i++)
            palette[i] = i >= img_size ? _black : img[i];
        return palette;
    } else {
        for (int i = 0; i < count; i++)
            palette[i] = img[i * img_size / count];
    }

    // K-means iterations
    for (int iter = 0; iter < 10; iter++) {
        // Arrays to accumulate color values for each cluster
        int cluster_r[count];
        int cluster_g[count];
        int cluster_b[count];
        int cluster_a[count];
        int cluster_count[count];
        for (int i = 0; i < count; i++)
            cluster_r[i] = cluster_g[i] = cluster_b[i] = cluster_a[i] = cluster_count[i] = 0;

        // Assign each pixel to nearest palette color
        for (int p = 0; p < img_size; p++) {
            int closest_idx = 0;
            float min_distance = color_distance(img[p], palette[0]);

            for (int i = 1; i < count; i++) {
                float distance = color_distance(img[p], palette[i]);
                if (distance < min_distance) {
                    min_distance = distance;
                    closest_idx = i;
                }
            }

            // Add pixel to cluster
            cluster_r[closest_idx] += img[p].r;
            cluster_g[closest_idx] += img[p].g;
            cluster_b[closest_idx] += img[p].b;
            cluster_a[closest_idx] += img[p].a;
            cluster_count[closest_idx]++;
        }

        // Update palette colors to cluster centroids
        for (int i = 0; i < count; i++) {
            if (cluster_count[i] > 0) {
                palette[i].r = cluster_r[i] / cluster_count[i];
                palette[i].g = cluster_g[i] / cluster_count[i];
                palette[i].b = cluster_b[i] / cluster_count[i];
                palette[i].a = cluster_a[i] / cluster_count[i];
            }
        }
    }

    // Sort palette by brightness (optional, for consistent ordering)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            int brightness_i = palette[i].r + palette[i].g + palette[i].b;
            int brightness_j = palette[j].r + palette[j].g + palette[j].b;

            if (brightness_i > brightness_j) {
                color_t temp = palette[i];
                palette[i] = palette[j];
                palette[j] = temp;
            }
        }
    }

    return palette;
}

// Load image from raw pixel data with specified format
bitmap_t bitmap_load(const void* data, int width, int height, bitmap_format_t format) {
    if (!data || width <= 0 || height <= 0)
        return NULL;
    
    bitmap_t img = bitmap_make(width, height);
    if (!img)
        return NULL;
    
    const uint8_t* src = (const uint8_t*)data;
    
    for (int y = 0; y < height; y++) 
        for (int x = 0; x < width; x++) {
            color_t pixel = {0, 0, 0, 255}; // Default: black with full alpha
            switch (format) {
                case BITMAP_FORMAT_RGBA:
                    pixel.r = *src++;
                    pixel.g = *src++;
                    pixel.b = *src++;
                    pixel.a = *src++;
                    break;
                case BITMAP_FORMAT_RGB:
                    pixel.r = *src++;
                    pixel.g = *src++;
                    pixel.b = *src++;
                    pixel.a = 255;
                    break;
                case BITMAP_FORMAT_BGRA:
                    pixel.b = *src++;
                    pixel.g = *src++;
                    pixel.r = *src++;
                    pixel.a = *src++;
                    break;
                case BITMAP_FORMAT_BGR:
                    pixel.b = *src++;
                    pixel.g = *src++;
                    pixel.r = *src++;
                    pixel.a = 255;
                    break;
                case BITMAP_FORMAT_ARGB:
                    pixel.a = *src++;
                    pixel.r = *src++;
                    pixel.g = *src++;
                    pixel.b = *src++;
                    break;
                case BITMAP_FORMAT_ABGR:
                    pixel.a = *src++;
                    pixel.b = *src++;
                    pixel.g = *src++;
                    pixel.r = *src++;
                    break;
                case BITMAP_FORMAT_GRAY: {
                    uint8_t gray = *src++;
                    pixel.r = gray;
                    pixel.g = gray;
                    pixel.b = gray;
                    pixel.a = 255;
                    break;
                }
                case BITMAP_FORMAT_GRAY_ALPHA: {
                    uint8_t gray = *src++;
                    pixel.r = gray;
                    pixel.g = gray;
                    pixel.b = gray;
                    pixel.a = *src++;
                    break;
                }
                case BITMAP_FORMAT_RGB565: {
                    uint16_t rgb565 = *(const uint16_t*)src;
                    src += 2;
                    
                    // Extract 5-6-5 components
                    uint8_t r5 = (rgb565 >> 11) & 0x1F;
                    uint8_t g6 = (rgb565 >> 5) & 0x3F;
                    uint8_t b5 = rgb565 & 0x1F;
                    
                    // Scale to 8-bit
                    pixel.r = (r5 << 3) | (r5 >> 2);
                    pixel.g = (g6 << 2) | (g6 >> 4);
                    pixel.b = (b5 << 3) | (b5 >> 2);
                    pixel.a = 255;
                    break;
                }
                case BITMAP_FORMAT_RGB555: {
                    uint16_t rgb555 = *(const uint16_t*)src;
                    src += 2;
                    
                    // Extract 5-5-5 components
                    uint8_t r5 = (rgb555 >> 10) & 0x1F;
                    uint8_t g5 = (rgb555 >> 5) & 0x1F;
                    uint8_t b5 = rgb555 & 0x1F;
                    
                    // Scale to 8-bit
                    pixel.r = (r5 << 3) | (r5 >> 2);
                    pixel.g = (g5 << 3) | (g5 >> 2);
                    pixel.b = (b5 << 3) | (b5 >> 2);
                    pixel.a = 255;
                    break;
                }
                case BITMAP_FORMAT_ARGB1555: {
                    uint16_t argb1555 = *(const uint16_t*)src;
                    src += 2;
                    
                    // Extract 1-5-5-5 components
                    uint8_t a1 = (argb1555 >> 15) & 0x1;
                    uint8_t r5 = (argb1555 >> 10) & 0x1F;
                    uint8_t g5 = (argb1555 >> 5) & 0x1F;
                    uint8_t b5 = argb1555 & 0x1F;
                    
                    // Scale to 8-bit
                    pixel.r = (r5 << 3) | (r5 >> 2);
                    pixel.g = (g5 << 3) | (g5 >> 2);
                    pixel.b = (b5 << 3) | (b5 >> 2);
                    pixel.a = a1 ? 255 : 0;
                    break;
                }
                default:
                    // Unknown format, fill with black
                    pixel.r = pixel.g = pixel.b = 0;
                    pixel.a = 255;
                    break;
            }
            bitmap_pset(img, x, y, pixel);
        }
    
    return img;
}
#endif
