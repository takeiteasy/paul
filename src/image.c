/* jeff/image.h -- https://github.com/takeiteasy/jeff

 Copyright (C) 2024  George Watson

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

#include "jeff.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"

int RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint8_t)a << 24) | ((uint8_t)r << 16) | ((uint8_t)g << 8) | b;
}

int RGB(uint8_t r, uint8_t g, uint8_t b) {
    return RGBA(r, g, b, 255);
}

int RGBA1(uint8_t c, uint8_t a) {
    return RGBA(c, c, c, a);
}

int RGB1(uint8_t c) {
    return RGB(c, c, c);
}

uint8_t Rgba(int c) {
    return (uint8_t)((c >> 16) & 0xFF);
}

uint8_t rGba(int c) {
    return (uint8_t)((c >>  8) & 0xFF);
}

uint8_t rgBa(int c) {
    return (uint8_t)(c & 0xFF);
}

uint8_t rgbA(int c) {
    return (uint8_t)((c >> 24) & 0xFF);
}

int rGBA(int c, uint8_t r) {
    return (c & ~0x00FF0000) | (r << 16);
}

int RgBA(int c, uint8_t g) {
    return (c & ~0x0000FF00) | (g << 8);
}

int RGbA(int c, uint8_t b) {
    return (c & ~0x000000FF) | b;
}

int RGBa(int c, uint8_t a) {
    return (c & ~0x00FF0000) | (a << 24);
}

image_t* image_empty(unsigned int w, unsigned int h) {
    image_t *result = malloc(sizeof(image_t));
    result->width = w;
    result->height = h;
    result->buffer = malloc(w * h * sizeof(int));
    return result;
}

#define VALID_EXTS_SZ 11
static const char *valid_extensions[VALID_EXTS_SZ] = {
    "jpg", "jpeg", "png", "bmp", "psd", "tga", "hdr", "pic", "ppm", "pgm", "qoi"
};

static const char* file_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

static bool check_extension(const char *path) {
    const char *ext = file_extension(path);
    unsigned long ext_length = strlen(ext);
    char *dup = strdup(ext);
    for (int i = 0; i < ext_length; i++)
        if (dup[i] >= 'A' && dup[i] <= 'Z')
            dup[i] += 32;
    int found = 0;
    for (int i = 0; i < VALID_EXTS_SZ; i++) {
        if (!strncmp(dup, valid_extensions[i], ext_length)) {
            found = 1;
            break;
        }
    }
    free(dup);
    return found;
}

image_t* image_load(const char *path) {
    size_t size = 0;
    if (!check_extension(path))
        return NULL;
    unsigned char *data = vfs_read(path, &size);
    if (!data)
        return NULL;
    image_t* result = image_load_from_memory(data, (int)size);
    free(data);
    return result;
}

static int check_if_qoi(unsigned char *data) {
    return (data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]) == QOI_MAGIC;
}

image_t* image_load_from_memory(const void *data, size_t length) {
    int _w, _h, c;
    unsigned char *in = NULL;
    if (check_if_qoi((unsigned char*)data)) {
        qoi_desc desc;
        in = qoi_decode(data, (int)length, &desc, 4);
        _w = desc.width;
        _h = desc.height;
    } else
        in = stbi_load_from_memory(data, (int)length, &_w, &_h, &c, 4);
    assert(in && _w && _h);

    image_t *result = image_empty(_w, _h);
    for (int x = 0; x < _w; x++)
        for (int y = 0; y < _h; y++) {
            unsigned char *p = in + (x + _w * y) * 4;
            result->buffer[y * _w + x] = RGBA(p[0], p[1], p[2], p[3]);
        }
    free(in);
    return result;
}

bool image_save(image_t *img, const char *path) {
    // TODO: Integrate stb_image_write.h
    return false;
}

void image_destroy(image_t *img) {
    if (img) {
        if (img->buffer)
            free(img->buffer);
        free(img);
    }
}

void image_fill(image_t *img, int col) {
    for (int i = 0; i < img->width * img->height; ++i)
        img->buffer[i] = col;
}

static inline void flood_fn(image_t *img, int x, int y, int _new, int _old) {
    if (_new == _old || image_pget(img, x, y) != _old)
        return;

    int x1 = x;
    while (x1 < img->width && image_pget(img, x1, y) == _old) {
        image_pset(img, x1, y, _new);
        x1++;
    }

    x1 = x - 1;
    while (x1 >= 0 && image_pget(img, x1, y) == _old) {
        image_pset(img, x1, y, _new);
        x1--;
    }

    x1 = x;
    while (x1 < img->width && image_pget(img, x1, y) == _old) {
        if(y > 0 && image_pget(img, x1, y - 1) == _old)
            flood_fn(img, x1, y - 1, _new, _old);
        x1++;
    }

    x1 = x - 1;
    while(x1 >= 0 && image_pget(img, x1, y) == _old) {
        if(y > 0 && image_pget(img, x1, y - 1) == _old)
            flood_fn(img, x1, y - 1, _new, _old);
        x1--;
    }

    x1 = x;
    while(x1 < img->width && image_pget(img, x1, y) == _old) {
        if(y < img->height - 1 && image_pget(img, x1, y + 1) == _old)
            flood_fn(img, x1, y + 1, _new, _old);
        x1++;
    }

    x1 = x - 1;
    while(x1 >= 0 && image_pget(img, x1, y) == _old) {
        if(y < img->height - 1 && image_pget(img, x1, y + 1) == _old)
            flood_fn(img, x1, y + 1, _new, _old);
        x1--;
    }
}

void image_flood(image_t *img, int x, int y, int col) {
    if (x < 0 || y < 0 || x >= img->width || y >= img->height)
        return;
    flood_fn(img, x, y, col, image_pget(img, x, y));
}

#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

static int Blend(int _a, int _b) {
    int a   = rgbA(_a);
    int b   = rgbA(_b);
    return !a ? 0xFF000000 : a >= 255 ? RGBa(a, 255) : RGBA(BLEND(Rgba(_a), Rgba(_b), a, b),
                                                            BLEND(rGba(_a), rGba(_b), a, b),
                                                            BLEND(rgBa(_a), rgBa(_b), a, b),
                                                            a + (b * (255 - a) >> 8));
}

void image_pset(image_t *img, int x, int y, int col) {
    if (x >= 0 && y >= 0 && x < img->width && y < img->height) {
        int a = rgbA(col);
        img->buffer[y * img->width + x] = a == 255 ? col : a == 0 ? 0 : Blend(image_pget(img, x, y), col);
    }
}

int image_pget(image_t *img, int x, int y) {
    return (x >= 0 && y >= 0 && x < img->width && y < img->height) ? img->buffer[y * img->width + x] : 0;
}

void image_paste(image_t *dst, image_t *src, int x, int y) {
    for (int ox = 0; ox < src->width; ++ox) {
        for (int oy = 0; oy < src->height; ++oy) {
            if (oy > dst->height)
                break;
            image_pset(dst, x + ox, y + oy, image_pget(src, ox, oy));
        }
        if (ox > dst->width)
            break;
    }
}

void image_clipped_paste(image_t *dst, image_t *src, int x, int y, int rx, int ry, int rw, int rh) {
    for (int ox = 0; ox < rw; ++ox)
        for (int oy = 0; oy < rh; ++oy)
            image_pset(dst, ox + x, oy + y, image_pget(src, ox + rx, oy + ry));
}

image_t* image_dupe(image_t *src) {
    image_t *result = image_empty(src->width, src->height);
    memcpy(result->buffer, src->buffer, src->width * src->height * sizeof(int));
    return result;
}

void image_pass_thru(image_t *img, int(*fn)(int x, int y, int col)) {
    int x, y;
    for (x = 0; x < img->width; ++x)
        for (y = 0; y < img->height; ++y)
            img->buffer[y * img->width + x] = fn(x, y, image_pget(img, x, y));
}

image_t* image_resized(image_t *src, int nw, int nh) {
    image_t *result = image_empty(nw, nh);
    int x_ratio = (int)((src->width << 16) / result->width) + 1;
    int y_ratio = (int)((src->height << 16) / result->height) + 1;
    int x2, y2, i, j;
    for (i = 0; i < result->height; ++i) {
        int *t = result->buffer + i * result->width;
        y2 = ((i * y_ratio) >> 16);
        int *p = src->buffer + y2 * src->width;
        int rat = 0;
        for (j = 0; j < result->width; ++j) {
            x2 = (rat >> 16);
            *t++ = p[x2];
            rat += x_ratio;
        }
    }
    return result;
}

void image_resize(image_t *src, int nw, int nh) {
    image_t *result = image_resized(src, nw, nh);
    free(src->buffer);
    memcpy(src, result, sizeof(image_t));
}

#define __MIN(a, b) (((a) < (b)) ? (a) : (b))
#define __MAX(a, b) (((a) > (b)) ? (a) : (b))
#define __D2R(a) ((a) * M_PI / 180.0)

image_t* image_rotated(image_t *src, float angle) {
    float theta = __D2R(angle);
    float c = cosf(theta), s = sinf(theta);
    float r[3][2] = {
        { -src->height * s, src->height * c },
        {  src->width * c - src->height * s, src->height * c + src->width * s },
        {  src->width * c, src->width * s }
    };

    float mm[2][2] = {{
        __MIN(0, __MIN(r[0][0], __MIN(r[1][0], r[2][0]))),
        __MIN(0, __MIN(r[0][1], __MIN(r[1][1], r[2][1])))
    }, {
        (theta > 1.5708  && theta < 3.14159 ? 0.f : __MAX(r[0][0], __MAX(r[1][0], r[2][0]))),
        (theta > 3.14159 && theta < 4.71239 ? 0.f : __MAX(r[0][1], __MAX(r[1][1], r[2][1])))
    }};

    int dw = (int)ceil(fabsf(mm[1][0]) - mm[0][0]);
    int dh = (int)ceil(fabsf(mm[1][1]) - mm[0][1]);
    image_t *result = image_empty(dw, dh);

    int x, y, sx, sy;
    for (x = 0; x < dw; ++x)
        for (y = 0; y < dh; ++y) {
            sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
            sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
            if (sx < 0 || sx >= src->width || sy < 0 || sy >= src->height)
                continue;
            image_pset(result, x, y, image_pget(src, sx, sy));
        }
    return result;
}

void image_rotate(image_t *src, float angle) {
    image_t *result = image_rotated(src, angle);
    image_destroy(src);
    src = result;
}

#define __CLAMP(X, MINX, MAXX) __MIN(__MAX((X), (MINX)), (MAXX))

image_t* image_clipped(image_t *src, int rx, int ry, int rw, int rh) {
    int ox = __CLAMP(rx, 0, src->width);
    int oy = __CLAMP(ry, 0, src->height);
    if (ox >= src->width || oy >= src->height)
        return NULL;
    int mx = __MIN(ox + rw, src->width);
    int my = __MIN(oy + rh, src->height);
    int iw = mx - ox;
    int ih = my - oy;
    if (iw <= 0 || ih <= 0)
        return NULL;
    image_t *result = image_empty(iw, ih);
    for (int px = 0; px < iw; px++)
        for (int py = 0; py < ih; py++) {
            int cx = ox + px;
            int cy = oy + py;
            image_pset(result, px, py, image_pget(src, cx, cy));
        }
    return result;
}

static inline void vline(image_t *img, int x, int y0, int y1, int col) {
    if (y1 < y0) {
        y0 += y1;
        y1  = y0 - y1;
        y0 -= y1;
    }

    if (x < 0 || x >= img->width || y0 >= img->height)
        return;

    if (y0 < 0)
        y0 = 0;
    if (y1 >= img->height)
        y1 = img->height - 1;

    for(int y = y0; y <= y1; y++)
        image_pset(img, x, y, col);
}

static inline void hline(image_t *img, int y, int x0, int x1, int col) {
    if (x1 < x0) {
        x0 += x1;
        x1  = x0 - x1;
        x0 -= x1;
    }

    if (y < 0 || y >= img->height || x0 >= img->width)
        return;

    if (x0 < 0)
        x0 = 0;
    if (x1 >= img->width)
        x1 = img->width - 1;

    for(int x = x0; x <= x1; x++)
        image_pset(img, x, y, col);
}

void image_draw_line(image_t *img, int x0, int y0, int x1, int y1, int col) {
    if (x0 == x1)
        vline(img, x0, y0, y1, col);
    else if (y0 == y1)
        hline(img, y0, x0, x1, col);
    else {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = (dx > dy ? dx : -dy) / 2;

        while (image_pset(img, x0, y0, col), x0 != x1 || y0 != y1) {
            int e2 = err;
            if (e2 > -dx) { err -= dy; x0 += sx; }
            if (e2 <  dy) { err += dx; y0 += sy; }
        }
    }
}

void image_draw_circle(image_t *img, int xc, int yc, int r, int col, int fill) {
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        image_pset(img, xc - x, yc + y, col);    /*   I. Quadrant */
        image_pset(img, xc - y, yc - x, col);    /*  II. Quadrant */
        image_pset(img, xc + x, yc - y, col);    /* III. Quadrant */
        image_pset(img, xc + y, yc + x, col);    /*  IV. Quadrant */

        if (fill) {
            hline(img, yc - y, xc - x, xc + x, col);
            hline(img, yc + y, xc - x, xc + x, col);
        }

        r = err;
        if (r <= y)
            err += ++y * 2 + 1; /* e_xy+e_y < 0 */
        if (r > x || err > y)
            err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}

void image_draw_rectangle(image_t *img, int x, int y, int w, int h, int col, int fill) {
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
    if (w < 0 || h < 0 || x > img->width || y > img->height)
        return;

    if (w > img->width)
        w = img->width;
    if (h > img->height)
        h = img->height;

    if (fill) {
        for (; y < h; ++y)
            hline(img, y, x, w, col);
    } else {
        hline(img, y, x, w, col);
        hline(img, h, x, w, col);
        vline(img, x, y, h, col);
        vline(img, w, y, h, col);
    }
}

#define __SWAP(a, b)  \
    do                \
    {                 \
        int temp = a; \
        a = b;        \
        b = temp;     \
    } while (0)

void image_draw_triangle(image_t *img, int x0, int y0, int x1, int y1, int x2, int y2, int col, int fill) {
    if (y0 ==  y1 && y0 ==  y2)
        return;
    if (fill) {
        if (y0 > y1) {
            __SWAP(x0, x1);
            __SWAP(y0, y1);
        }
        if (y0 > y2) {
            __SWAP(x0, x2);
            __SWAP(y0, y2);
        }
        if (y1 > y2) {
            __SWAP(x1, x2);
            __SWAP(y1, y2);
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
                __SWAP(ax, bx);
                __SWAP(ay, by);
            }
            for (j = ax; j <= bx; ++j)
                image_pset(img, j, y0 + i, col);
        }
    } else {
        image_draw_line(img, x0, y0, x1, y1, col);
        image_draw_line(img, x1, y1, x2, y2, col);
        image_draw_line(img, x2, y2, x0, y0, col);
    }
}

image_t* image_perlin_noise(unsigned int width, unsigned int height, float z, float offset_x, float offset_y, float scale, float lacunarity, float gain, int octaves) {
    image_t *result = image_empty(width, height);
    uint8_t *map = perlin_noise_map(width, height, z, offset_x, offset_y, scale, lacunarity, gain, octaves);
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            image_pset(result, x, y, RGBA1(map[y * width + x], 255));
    free(map);
    return result;
}

sg_image sg_empty_texture(unsigned int width, unsigned int height) {
    assert(width && height);
    sg_image_desc desc = {
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .usage = SG_USAGE_STREAM
    };
    return sg_make_image(&desc);
}

sg_image sg_load_texture(const char *path) {
    return sg_load_texture_ex(path, NULL, NULL);
}

sg_image sg_load_texture_from_memory(unsigned char *data, size_t data_size) {
    return sg_load_texture_from_memory_ex(data, data_size, NULL, NULL);
}

static sg_image image_to_sg(image_t *img) {
    sg_image texture = sg_empty_texture(img->width, img->height);
    sg_image_data desc = {
        .subimage[0][0] = (sg_range) {
            .ptr = img->buffer,
            .size = img->width * img->height * sizeof(int)
        }
    };
    sg_update_image(texture, &desc);
    return texture;
}

sg_image sg_load_texture_ex(const char *path, unsigned int *width, unsigned int *height) {
    image_t *tmp = image_load(path);
    if (!tmp)
        return (sg_image){SG_INVALID_ID};
    sg_image texture = image_to_sg(tmp);
    if (width)
        *width = tmp->width;
    if (height)
        *height = tmp->height;
    free(tmp);
    return texture;
}

sg_image sg_load_texture_from_memory_ex(unsigned char *data, size_t data_size, unsigned int *width, unsigned int *height) {
    image_t *tmp = image_load_from_memory(data, data_size);
    if (!tmp)
        return (sg_image){SG_INVALID_ID};
    sg_image texture = image_to_sg(tmp);
    if (width)
        *width = tmp->width;
    if (height)
        *height = tmp->height;
    free(tmp);
    return texture;
}
