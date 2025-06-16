/* jeff.h -- https://github.com/takeiteasy/jeff 

jeff Copyright (C) 2025 George Watson

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

#ifndef JEFF_HEAD
#define JEFF_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __has_include
#define __has_include(x) 1
#endif
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_extension
#define __has_extension __has_feature
#endif

#if __STDC_VERSION__ >= 202311L
#define JEFF_HAS_TYPEOF
#elif defined(COMPILER_GCC)
#if __GNUC__ > 10 || (__GNUC__ == 10 && __GNUC_MINOR__ >= 4)
#define typeof(X) __typeof__((X))
#define JEFF_HAS_TYPEOF
#endif
#elif defined(COMPILER_CLANG)
#if __clang_major__ >= 15
#define typeof(X) __typeof__((X))
#define JEFF_HAS_TYPEOF
#endif
#endif

#if __has_extension(c_generic_selections)
#define JEFF_HAS_GENERICS
#else
#if defined(__STDC__) && __STDC_VERSION__ < 201112L
#define JEFF_HAS_GENERICS
#endif
#endif

#ifndef JEFF_HAS_GENERICS
#error jeff requires _Generic support
#endif

#if defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#if __has_extension(blocks)
#define JEFF_HAS_BLOCKS
#endif
#endif

#if defined(JEFF_USE_BLOCKS) && !defined(JEFF_HAS_BLOCKS)
#error This platform doesn't support blocks, undefine JEFF_USE_BLOCKS
#endif

#if __has_include("jeff_config.h")
#include "jeff_config.h"
#endif

#ifdef JEFF_NO_THREADS
#define PAUL_NO_THREADS
#endif
#define PAUL_NO_CLIPBOARD
#include "paul/paul.h"

#if !defined(JEFF_SCENES)
#ifndef JEFF_FIRST_SCENE
#error No scenes! Define JEFF_SCENES or JEFF_FIRST_SCENE first
#else
#define JEFF_SCENES X(JEFF_FIRST_SCENE)
#endif
#endif

#ifndef DEFAULT_WINDOW_WIDTH
#define DEFAULT_WINDOW_WIDTH 640
#endif

#ifndef DEFAULT_WINDOW_HEIGHT
#define DEFAULT_WINDOW_HEIGHT 480
#endif

#ifndef JEFF_NAME
#define JEFF_NAME "jeff"
#endif

#ifndef DEFAULT_WINDOW_TITLE
#define DEFAULT_WINDOW_TITLE JEFF_NAME
#endif

#ifndef MAX_TEXTURE_STACK
#define MAX_TEXTURE_STACK 8
#endif

#ifndef DEFAULT_TARGET_FPS
#define DEFAULT_TARGET_FPS 60.f
#endif

#ifndef MAX_DROPPED_FILES
#define MAX_DROPPED_FILES 1 // sapp default
#endif

#ifndef CLIPBOARD_SIZE
#define CLIPBOARD_SIZE 8192 // sapp default
#endif

#define _STRINGIFY(s) #s
#define STRINGIFY(S) _STRINGIFY(S)

#ifndef DEFAULT_CONFIG_NAME
#ifdef PLATFORM_POSIX
#define DEFAULT_CONFIG_NAME "." JEFF_NAME ".json"
#else
#define DEFAULT_CONFIG_NAME JEFF_NAME ".json"
#endif
#endif
#ifndef JEFF_CONFIG_PATH
#define JEFF_CONFIG_PATH DEFAULT_CONFIG_NAME
#endif

#ifndef MAX_INPUT_STATE_KEYS
#define MAX_INPUT_STATE_KEYS 8
#endif

#ifndef JEFF_RNG_SEED
#define JEFF_RNG_SEED 0
#endif

#ifndef MAX_PATH
#if defined(PLATFORM_MAC)
#define MAX_PATH 255
#elif defined(PLATFORM_WINDOWS)
#define MAX_PATH 256
#elif defined(PLATFORM_LINUX)
#define MAX_PATH 4096
#endif
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include <limits.h>

#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_audio.h"
#include "sokol/sokol_log.h"
#include "sokol/sokol_time.h"
//#include "sokol/sokol_fetch.h"
#include "sokol/util/sokol_color.h"
#include "sokol/util/sokol_shape.h"
#ifndef JEFF_NO_INPUT
#include "sokol_input.h"
#endif
#include "sokol_generic.h"

typedef struct scene_t scene_t;

struct scene_t {
    const char *name;
    void(*enter)(void);
    void(*exit)(void);
#ifdef JEFF_NO_INPUT
    void(*event)(const sapp_event*);
#endif
    void(*step)(void);
};

#define X(NAME) extern scene_t NAME##_scene;
JEFF_SCENES
#undef X

void jeff_set_scene(scene_t *scene);
void jeff_set_scene_named(const char *name);

#ifdef JEFF_USE_BLOCKS
typedef void(^jeff_exit_callback_t)(void*);
#else
typedef void(*jeff_exit_callback_t)(void*);
#endif
void jeff_atexit(jeff_exit_callback_t callback, void *userdata);

#define _PI 3.14159265358979323846264338327950288
#define _TWO_PI 6.28318530717958647692 // 2 * pi
#define _TAU TWO_PI
#define _HALF_PI 1.57079632679489661923132169163975144     // pi / 2
#define _QUARTER_PI 0.785398163397448309615660845819875721 // pi / 4
#define _PHI 1.61803398874989484820
#define _INV_PHI 0.61803398874989484820
#define _EULER 2.71828182845904523536
#define _EPSILON 0.000001f
#define _SQRT2 1.41421356237309504880168872420969808
#define _BYTES(N) (N)
#define _KILOBYTES(N) ((N) << 10)
#define _MEGABYTES(N) ((N) << 20)
#define _GIGABYTES(N) (((unsigned long long)(N)) << 30)
#define _TERABYTES(N) (((unsigned long long)(N)) << 40)
#define _THOUSAND(N)  ((N) * 1000)
#define _MILLION(N)   ((N) * 1000000)
#define _BILLION(N)   (((unsigned long long)(N)) * 1000000000LL)
#define _DEGREES(N)   (((double)(N)) * (180. / PI))
#define _RADIANS(N)   (((double)(N)) * (_PI / 180.))
#define _MIN(A, B)    ((A) < (B) ? (A) : (B))
#define _MAX(A, B)    ((A) > (B) ? (A) : (B))
#define _SWAP(A, B)   ((A)^=(B)^=(A)^=(B))
#define _REMAP(X, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX) ((OUT_MIN) + (((X) - (IN_MIN)) * ((OUT_MAX) - (OUT_MIN)) / ((IN_MAX) - (IN_MIN))))
#define _CLAMP(x, low, high) _MIN(_MAX(x, low), high)

enum jeff_easing_fn {
    JEFF_EASING_LINEAR = 0,
    JEFF_EASING_SINE,
    JEFF_EASING_CIRCULAR,
    JEFF_EASING_CUBIC,
    JEFF_EASING_QUAD,
    JEFF_EASING_EXPONENTIAL,
    JEFF_EASING_BACK,
    JEFF_EASING_BOUNCE,
    JEFF_EASING_ELASTIC
};

enum jeff_easing_t {
    EASE_IN = 1,
    EASE_OUT,
    EASE_INOUT
};

float jeff_ease(enum jeff_easing_fn func, enum jeff_easing_t type, float t, float b, float c, float d);
bool jeff_flt_cmp(float a, float b);
bool jeff_dbl_cmp(double a, double b);

// returns sapp_width + scale
int sapp_framebuffer_width(void);
// returns sapp_height + scale
int sapp_framebuffer_height(void);
// returns sapp_widthf + scale
float sapp_framebuffer_widthf(void);
// returns sapp_heightf + scale
float sapp_framebuffer_heightf(void);
// returns monitor scale factor (Mac) or 1.f (other platforms)
float sapp_framebuffer_scalefactor(void);

#ifndef JEFF_NO_INPUT
#ifdef JEFF_USE_BLOCKS
typedef int(^sapp_event_callback_t)(void*);
#else
typedef int(*sapp_event_callback_t)(void*);
#endif

void sapp_emit_on_event(sapp_event_type event_type, const char *event);
void sapp_on_event(sapp_event_type event_type, sapp_event_callback_t callback);
void sapp_remove_event(sapp_event_type event_type);
// TODO: Add action type (up/down)
bool sapp_on_input_str(const char *input_str, const char *event, void *userdata);
bool sapp_on_input(const char *event, void *userdata, int modifiers, int n, ...);
void sapp_remove_input_event(const char *event);
void sapp_clear_events(void);
#endif

bool jeff_vfs_mount(const char *src, const char *dst);
bool jeff_vfs_unmount(const char *name);
void jeff_vfs_unmount_all(void);
unsigned char *vfs_read(const char *filename, size_t *size);
#ifdef JEFF_USE_BLOCKS
typedef int(^jeff_glob_callback_t)(const char*);
#else
typedef int(*jeff_glob_callback_t)(const char*);
#endif
void jeff_vfs_glob(const char *glob, jeff_glob_callback_t callback);

void jeff_srand(uint64_t seed);
uint64_t jeff_rand_int(void);
float jeff_rand_float(void);
int jeff_rand_int_range(int min, int max);
float jeff_rand_float_range(float min, float max);
int jeff_rand_choice(int length);
int* jeff_rand_sample(int length, int max);

uint8_t* jeff_cellular_automata(unsigned int width, unsigned int height, unsigned int fill_chance, unsigned int smooth_iterations, unsigned int survive, unsigned int starve);
uint8_t* jeff_perlin_fbm(unsigned int width, unsigned int height, float z, float offset_x, float offset_y, float scale, float lacunarity, float gain, int octaves);
float perlin_noise(float x, float y, float z);
// TODO: Poisson disc sampling

int RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int RGB(uint8_t r, uint8_t g, uint8_t b);
int RGBA1(uint8_t c, uint8_t a);
int RGB1(uint8_t c);

uint8_t Rgba(int c);
uint8_t rGba(int c);
uint8_t rgBa(int c);
uint8_t rgbA(int c);

int rGBA(int c, uint8_t r);
int RgBA(int c, uint8_t g);
int RGbA(int c, uint8_t b);
int RGBa(int c, uint8_t a);

typedef struct image_t {
    unsigned int width, height;
    int *buffer;
} jeff_image_t;

jeff_image_t* jeff_image_empty(unsigned int w, unsigned int h);
jeff_image_t* jeff_image_filled(unsigned int w, unsigned int h, int color);
jeff_image_t* jeff_image_load(const char *path);
jeff_image_t* jeff_image_load_from_memory(const void *data, size_t length);
bool jeff_image_save(jeff_image_t *img, const char *path);
void jeff_image_destroy(jeff_image_t *img);

void jeff_image_fill(jeff_image_t *img, int col);
void jeff_image_flood(jeff_image_t *img, int x, int y, int col);
void jeff_image_pset(jeff_image_t *img, int x, int y, int col);
int jeff_image_pget(jeff_image_t *img, int x, int y);
void jeff_image_paste(jeff_image_t *dst, jeff_image_t *src, int x, int y);
void jeff_image_clipped_paste(jeff_image_t *dst, jeff_image_t *src, int x, int y, int rx, int ry, int rw, int rh);
void jeff_image_resize(jeff_image_t *src, int nw, int nh);
void jeff_image_rotate(jeff_image_t *src, float angle);

#ifdef JEFF_USE_BLOCKS
typedef int(^jeff_image_callback_t)(int x, int y, int col);
#else
typedef int(*jeff_image_callback_t)(int x, int y, int col);
#endif
void jeff_image_pass_thru(jeff_image_t *img, jeff_image_callback_t fn);

jeff_image_t* jeff_image_dupe(jeff_image_t *src);
jeff_image_t* jeff_image_resized(jeff_image_t *src, int nw, int nh);
jeff_image_t* jeff_image_rotated(jeff_image_t *src, float angle);
jeff_image_t* jeff_image_clipped(jeff_image_t *src, int rx, int ry, int rw, int rh);

void jeff_image_draw_line(jeff_image_t *img, int x0, int y0, int x1, int y1, int col);
void jeff_image_draw_circle(jeff_image_t *img, int xc, int yc, int r, int col, int fill);
void jeff_image_draw_rectangle(jeff_image_t *img, int x, int y, int w, int h, int col, int fill);
void jeff_image_draw_triangle(jeff_image_t *img, int x0, int y0, int x1, int y1, int x2, int y2, int col, int fill);

jeff_image_t* jeff_image_from_perlin(unsigned int width, unsigned int height, float z, float offset_x, float offset_y, float scale, float lacunarity, float gain, int octaves);

sg_image sg_empty_texture(unsigned int width, unsigned int height);
sg_image sg_load_texture(const char *path);
sg_image sg_load_texture_from_memory(unsigned char *data, size_t data_size);
sg_image sg_load_texture_ex(const char *path, unsigned int *width, unsigned int *height);
sg_image sg_load_texture_from_memory_ex(unsigned char *data, size_t data_size, unsigned int *width, unsigned int *height);
sg_image sg_load_texture_from_image(jeff_image_t *img);

typedef struct audio_t {
    unsigned int count;
    unsigned int rate;
    unsigned int size; // 8, 16, or 32
    unsigned int channels;
    void *buffer;
} jeff_audio_t;

jeff_audio_t* jeff_audio_load(const char *path);
jeff_audio_t* jeff_audio_load_from_memory(const unsigned char *data, int size);
bool jeff_audio_save(jeff_audio_t *audio, const char *path);
void jeff_audio_destroy(jeff_audio_t *audio);

jeff_audio_t* jeff_audio_dupe(jeff_audio_t *audio);
void jeff_audio_crop(jeff_audio_t *audio, int init_sample, int final_sample);
jeff_audio_t* jeff_audio_cropped(jeff_audio_t *audio, int init_sample, int final_sample);
float* jeff_audio_read_all_samples(jeff_audio_t *audio);
float jeff_audio_sample(jeff_audio_t *audio, int frame);
void jeff_audio_read_samples(jeff_audio_t *audio, int start_frame, int end_frame, float *dst);
float jeff_audio_length(jeff_audio_t *audio);

#ifdef JEFF_USE_BLOCKS
typedef void(^jeff_event_callback_t)(void*);
typedef void(^jeff_timer_callback_t)(void*);
#else
typedef void(*jeff_event_callback_t)(void*);
typedef void(*jeff_timer_callback_t)(void*);
#endif

void jeff_event_listen(const char *name, jeff_event_callback_t cb, void *userdata);
void jeff_event_listen_once(const char *name, jeff_event_callback_t cb, void *userdata);
void jeff_event_remove(const char *name);
void jeff_event_emit(const char *name);
void jeff_events_clear(void);

void jeff_timer_every(const char *name, int64_t ms, jeff_timer_callback_t cb, void *userdata);
void jeff_timer_emit_every(const char *name, int64_t ms, const char *event, void *userdata);
void jeff_timer_after(const char *name, int64_t ms, jeff_timer_callback_t cb, void *userdata);
void jeff_timer_emit_after(const char *name, int64_t ms, const char *event, void *userdata);
void jeff_timer_remove(const char *name);
void jeff_timers_clear(void);

#ifndef JEFF_NO_THREADS
typedef struct thread_work_t {
    void(*func)(void*);
    void *arg;
    struct thread_work_t *next;
} jeff_thrd_work_t;

typedef struct thread_pool_t {
    jeff_thrd_work_t *head, *tail;
    paul_mtx_t workMutex;
    paul_cnd_t workCond, workingCond;
    size_t workingCount, threadCount;
    int kill;
} jeff_thrd_pool_t;

jeff_thrd_pool_t* jeff_thrd_pool(size_t maxThreads);
void jeff_thrd_pool_destroy(jeff_thrd_pool_t *pool);
int jeff_thrd_pool_push_work(jeff_thrd_pool_t *pool, void(*func)(void*), void *arg);
// Adds work to the front of the queue
int jeff_thrd_pool_push_work_priority(jeff_thrd_pool_t *pool, void(*func)(void*), void *arg);
void jeff_thrd_pool_join(jeff_thrd_pool_t *pool);

typedef struct thread_queue_entry_t {
    void *data;
    struct thread_queue_entry_t *next;
} jeff_thrd_queue_entry_t;

typedef struct thread_queue_t {
    jeff_thrd_queue_entry_t *head, *tail;
    paul_mtx_t readLock, writeLock;
    size_t count;
} jeff_thrd_queue_t;

jeff_thrd_queue_t* jeff_thrd_queue(void);
void jeff_thrd_queue_push(jeff_thrd_queue_t *queue, void *data);
void* jeff_thrd_queue_pop(jeff_thrd_queue_t *queue);
void jeff_thrd_queue_destroy(jeff_thrd_queue_t *queue);
#endif

#ifdef __cplusplus
}
#endif
#endif // JEFF_HEAD
