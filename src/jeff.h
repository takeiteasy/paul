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

#if defined(__cplusplus)
#define JEFF_HAS_CONSTEXPR
#else
#if __STDC_VERSION__ >= 202000L
#define JEFF_HAS_CONSTEXPR
#endif
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

#define sg_make(OBJ)                                                           \
  _Generic((OBJ),                                                              \
      const sg_buffer_desc *: sg_make_buffer,                                  \
      const sg_image_desc *: sg_make_image,                                    \
      const sg_sampler_desc *: sg_make_sampler,                                \
      const sg_shader_desc *: sg_make_shader,                                  \
      const sg_pipeline_desc *: sg_make_pipeline,                              \
      const sg_attachments_desc *: sg_make_attachments)(OBJ)

#define sg_destroy(OBJ)                                                        \
  _Generic((OBJ),                                                              \
      sg_buffer: sg_destroy_buffer,                                            \
      sg_image: sg_destroy_image,                                              \
      sg_sampler: sg_destroy_sampler,                                          \
      sg_shader: sg_destroy_shader,                                            \
      sg_pipeline: sg_destroy_pipeline,                                        \
      sg_attachments: sg_destroy_attachments)(OBJ)

#define sg_destroy_if_valid(OBJ)                                               \
  do {                                                                         \
    if (sg_query_state((OBJ)))                                                 \
      sg_destroy((OBJ));                                                       \
  } while (0)

#define sg_query_state(OBJ)                                                    \
  _Generic((OBJ),                                                              \
      sg_buffer: sg_query_buffer_state,                                        \
      sg_image: sg_query_image_state,                                          \
      sg_sampler: sg_query_sampler_state,                                      \
      sg_shader: sg_query_shader_state,                                        \
      sg_pipeline: sg_query_pipeline_state,                                    \
      sg_attachments: sg_query_attachments_state)(OBJ)

#define sg_query_info(OBJ)                                                     \
  _Generic((OBJ),                                                              \
      sg_buffer: sg_query_buffer_info,                                         \
      sg_image: sg_query_image_info,                                           \
      sg_sampler: sg_query_sampler_info,                                       \
      sg_shader: sg_query_shader_info,                                         \
      sg_pipeline: sg_query_pipeline_info,                                     \
      sg_attachments: sg_query_attachments_info)(OBJ)

#define sg_query_setup_desc(OBJ)                                               \
  _Generic((OBJ),                                                              \
      sg_buffer: sg_query_buffer_desc,                                         \
      sg_image: sg_query_image_desc,                                           \
      sg_sampler: sg_query_sampler_desc,                                       \
      sg_shader: sg_query_shader_desc,                                         \
      sg_pipeline: sg_query_pipeline_desc,                                     \
      sg_attachments: sg_query_attachments_desc)(OBJ)

#define sg_query_defaults(OBJ)                                                 \
  _Generic((OBJ),                                                              \
      const sg_buffer *: sg_query_buffer_defaults,                             \
      const sg_image *: sg_query_image_defaults,                               \
      const sg_sampler *: sg_query_sampler_defaults,                           \
      const sg_shader *: sg_query_shader_defaults,                             \
      const sg_pipeline *: sg_query_pipeline_defaults,                         \
      const sg_attachments *: sg_query_attachments_defaults)(OBJ)

#define sg_query_defaults(OBJ)                                                 \
  _Generic((OBJ),                                                              \
      const sg_buffer *: sg_query_buffer_defaults,                             \
      const sg_image *: sg_query_image_defaults,                               \
      const sg_sampler *: sg_query_sampler_defaults,                           \
      const sg_shader *: sg_query_shader_defaults,                             \
      const sg_pipeline *: sg_query_pipeline_defaults,                         \
      const sg_attachments *: sg_query_attachments_defaults)(OBJ)

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

bool sapp_is_key_down(int key);
// This will be true if a key is held for more than 1 second
// TODO: Make the duration configurable
bool sapp_is_key_held(int key);
// Returns true if key is down this frame and was up last frame
bool sapp_was_key_pressed(int key);
// Returns true if key is up and key was down last frame
bool sapp_was_key_released(int key);
// If any of the keys passed are not down returns false
bool sapp_are_keys_down(int n, ...);
// If none of the keys passed are down returns false
bool sapp_any_keys_down(int n, ...);
bool sapp_is_button_down(int button);
bool sapp_is_button_held(int button);
bool sapp_was_button_pressed(int button);
bool sapp_was_button_released(int button);
bool sapp_are_buttons_down(int n, ...);
bool sapp_any_buttons_down(int n, ...);
bool sapp_has_mouse_move(void);
bool sapp_modifier_equals(int mods);
bool sapp_modifier_down(int mod);
int sapp_cursor_x(void);
int sapp_cursor_y(void);
bool sapp_cursor_delta_x(void);
bool sapp_cursor_delta_y(void);
bool sapp_check_scrolled(void);
float sapp_scroll_x(void);
float sapp_scroll_y(void);

#ifdef JEFF_USE_BLOCKS
typedef int(^sapp_event_callback_t)(void*);
#else
typedef int(*sapp_event_callback_t)(void*);
#endif

void sapp_emit_on_event(sapp_event_type event_type, const char *event);
void sapp_on_event(sapp_event_type event_type, sapp_event_callback_t callback);
void sapp_remove_event(sapp_event_type event_type);
bool sapp_check_input_str_down(const char *str);
bool sapp_check_input_down(int modifiers, int n, ...);
bool sapp_check_input_str_released(const char *str);
bool sapp_check_input_released(int modifiers, int n, ...);
bool sapp_check_input_str_up(const char *str);
bool sapp_check_input_up(int modifiers, int n, ...);
// TODO: Add action type (up/down)
bool sapp_on_input_str(const char *input_str, const char *event, void *userdata);
bool sapp_on_input(const char *event, void *userdata, int modifiers, int n, ...);
void sapp_remove_input_event(const char *event);

void vfs_mount(const char *path);
bool vfs_exists(const char *filename);
unsigned char *vfs_read(const char *filename, size_t *size);
// TODO: vfs_write
// bool vfs_write(const char *filename, unsigned char *data, size_t size, bool overwrite);

void rng_srand(uint64_t seed);
uint64_t rng_rand_int(void);
float rng_rand_float(void);
int rng_rand_int_range(int min, int max);
float rng_rand_float_range(float min, float max);
int rng_rand_choice(int length);
int* rng_rand_sample(int length, int max);

uint8_t* cellular_automata_map(unsigned int width, unsigned int height, unsigned int fill_chance, unsigned int smooth_iterations, unsigned int survive, unsigned int starve);
uint8_t* perlin_noise_map(unsigned int width, unsigned int height, float z, float offset_x, float offset_y, float scale, float lacunarity, float gain, int octaves);
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
} image_t;

image_t* image_empty(unsigned int w, unsigned int h);
image_t* image_filled(unsigned int w, unsigned int h, int color);
image_t* image_load(const char *path);
image_t* image_load_from_memory(const void *data, size_t length);
bool image_save(image_t *img, const char *path);
void image_destroy(image_t *img);

void image_fill(image_t *img, int col);
void image_flood(image_t *img, int x, int y, int col);
void image_pset(image_t *img, int x, int y, int col);
int image_pget(image_t *img, int x, int y);
void image_paste(image_t *dst, image_t *src, int x, int y);
void image_clipped_paste(image_t *dst, image_t *src, int x, int y, int rx, int ry, int rw, int rh);
void image_resize(image_t *src, int nw, int nh);
void image_rotate(image_t *src, float angle);

#ifdef JEFF_USE_BLOCKS
typedef int(^image_callback_t)(int x, int y, int col);
#else
typedef int(*image_callback_t)(int x, int y, int col);
#endif
void image_pass_thru(image_t *img, image_callback_t fn);

image_t* image_dupe(image_t *src);
image_t* image_resized(image_t *src, int nw, int nh);
image_t* image_rotated(image_t *src, float angle);
image_t* image_clipped(image_t *src, int rx, int ry, int rw, int rh);

void image_draw_line(image_t *img, int x0, int y0, int x1, int y1, int col);
void image_draw_circle(image_t *img, int xc, int yc, int r, int col, int fill);
void image_draw_rectangle(image_t *img, int x, int y, int w, int h, int col, int fill);
void image_draw_triangle(image_t *img, int x0, int y0, int x1, int y1, int x2, int y2, int col, int fill);

image_t* image_perlin_noise(unsigned int width, unsigned int height, float z, float offset_x, float offset_y, float scale, float lacunarity, float gain, int octaves);

sg_image sg_empty_texture(unsigned int width, unsigned int height);
sg_image sg_load_texture(const char *path);
sg_image sg_load_texture_from_memory(unsigned char *data, size_t data_size);
sg_image sg_load_texture_ex(const char *path, unsigned int *width, unsigned int *height);
sg_image sg_load_texture_from_memory_ex(unsigned char *data, size_t data_size, unsigned int *width, unsigned int *height);
sg_image sg_load_texture_from_image(image_t *img);

typedef struct audio_t {
    unsigned int count;
    unsigned int rate;
    unsigned int size; // 8, 16, or 32
    unsigned int channels;
    void *buffer;
} audio_t;

audio_t* audio_load(const char *path);
audio_t* audio_load_from_memory(const unsigned char *data, int size);
bool audio_save(audio_t *audio, const char *path);
void audio_destroy(audio_t *audio);

audio_t* audio_dupe(audio_t *audio);
void audio_crop(audio_t *audio, int init_sample, int final_sample);
audio_t* audio_cropped(audio_t *audio, int init_sample, int final_sample);
float* audio_read_all_samples(audio_t *audio);
float audio_sample(audio_t *audio, int frame);
void audio_read_samples(audio_t *audio, int start_frame, int end_frame, float *dst);
float audio_length(audio_t *audio);

#ifdef JEFF_USE_BLOCKS
typedef void(^event_callback_t)(void*);
typedef void(^timer_callback_t)(void*);
#else
typedef void(*event_callback_t)(void*);
typedef void(*timer_callback_t)(void*);
#endif

void event_listen(const char *name, event_callback_t cb, void *userdata);
void event_listen_once(const char *name, event_callback_t cb, void *userdata);
void event_remove(const char *name);
void event_emit(const char *name);
void events_clear(void);

void timer_every(const char *name, int64_t ms, timer_callback_t cb, void *userdata);
void timer_emit_every(const char *name, int64_t ms, const char *event, void *userdata);
void timer_after(const char *name, int64_t ms, timer_callback_t cb, void *userdata);
void timer_emit_after(const char *name, int64_t ms, const char *event, void *userdata);
void timer_remove(const char *name);
void timers_clear(void);

#ifndef JEFF_NO_THREADS
typedef struct thread_work_t {
    void(*func)(void*);
    void *arg;
    struct thread_work_t *next;
} thread_work_t;

typedef struct thread_pool_t {
    thread_work_t *head, *tail;
    paul_mtx_t workMutex;
    paul_cnd_t workCond, workingCond;
    size_t workingCount, threadCount;
    int kill;
} thread_pool_t;

thread_pool_t* thread_pool(size_t maxThreads);
void thread_pool_destroy(thread_pool_t *pool);
int thread_pool_push_work(thread_pool_t *pool, void(*func)(void*), void *arg);
// Adds work to the front of the queue
int thread_pool_push_work_priority(thread_pool_t *pool, void(*func)(void*), void *arg);
void thread_pool_join(thread_pool_t *pool);

typedef struct thread_queue_entry_t {
    void *data;
    struct thread_queue_entry_t *next;
} thread_queue_entry_t;

typedef struct thread_queue_t {
    thread_queue_entry_t *head, *tail;
    paul_mtx_t readLock, writeLock;
    size_t count;
} thread_queue_t;

thread_queue_t* thread_queue(void);
void thread_queue_push(thread_queue_t *queue, void *data);
void* thread_queue_pop(thread_queue_t *queue);
void thread_queue_destroy(thread_queue_t *queue);
#endif

#ifdef JEFF_HAS_CONSTEXPR
constexpr double PI = 3.14159265358979323846264338327950288;
constexpr double TWO_PI = 6.28318530717958647692; // 2 * pi;
constexpr double TAU = TWO_PI;
constexpr double HALF_PI = 1.57079632679489661923132169163975144;     // pi / 2
constexpr double QUARTER_PI = 0.785398163397448309615660845819875721; // pi / 4
constexpr double PHI = 1.61803398874989484820;
constexpr double INV_PHI = 0.61803398874989484820;
constexpr double EULER = 2.71828182845904523536;
constexpr double EPSILON = 0.000001;
constexpr double SQRT2 = 1.41421356237309504880168872420969808;
#else // -- JEFF_HAS_CONSTEXPR --
#define PI 3.14159265358979323846264338327950288
#define TWO_PI 6.28318530717958647692 // 2 * pi
#define TAU TWO_PI
#define HALF_PI 1.57079632679489661923132169163975144     // pi / 2
#define QUARTER_PI 0.785398163397448309615660845819875721 // pi / 4
#define PHI 1.61803398874989484820
#define INV_PHI 0.61803398874989484820
#define EULER 2.71828182845904523536
#define EPSILON 0.000001f
#define SQRT2 1.41421356237309504880168872420969808
#endif // JEFF_HAS_CONSTEXPR

#ifndef JEFF_HAS_TYPEOF
#define BYTES(N) (N)
#define KILOBYTES(N) ((N) << 10)
#define MEGABYTES(N) ((N) << 20)
#define GIGABYTES(N) (((unsigned long long)(N)) << 30)
#define TERABYTES(N) (((unsigned long long)(N)) << 40)
#define THOUSAND(N)  ((N) * 1000)
#define MILLION(N)   ((N) * 1000000)
#define BILLION(N)   (((unsigned long long)(N)) * 1000000000LL)
#define DEGREES(N)   (((double)(N)) * (180. / PI))
#define RADIANS(N)   (((double)(N)) * (PI / 180.))
#define MIN(A, B)    ((A) < (B) ? (A) : (B))
#define MAX(A, B)    ((A) > (B) ? (A) : (B))
#define SWAP(A, B)   ((A)^=(B)^=(A)^=(B))
#define REMAP(X, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX) ((OUT_MIN) + (((X) - (IN_MIN)) * ((OUT_MAX) - (OUT_MIN)) / ((IN_MAX) - (IN_MIN))))
#else // --- JEFF_HAS_TYPEOF ---
#define BYTES(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       _n; })
#define KILOBYTES(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       _n << 10; })
#define MEGABYTES(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       _n << 20; })
#define GIGABYTES(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       ((unsigned long long)_n) << 30; })
#define TERABYTES(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       ((unsigned long long)_n) << 40; })
#define THOUSAND(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       _n * 1000; })
#define MILLION(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       _n * 1000000; })
#define BILLION(n) \
    ({ typeof (n) _n = (n); \
       static_assert(is_integral(_n)); \
       (unsigned long long)_n * 1000000000LL; })
#define DEGREES(R) \
    ({ typeof (R) n = (R); \
       static_assert(is_floating_point(n)); \
        n * (180. / PI); })
#define RADIANS(D) \
    ({ typeof (D) n = (D); \
       static_assert(is_integral(n)); \
       n * (PI / 180.); })
#define MIN(a, b) \
    ({ \
        typeof (a) _min = (a); \
        _min = (b) < _min ? (b) : _min; \
        _min; \
    })
#define MAX(a, b) \
    ({ \
        typeof (a) _max = (a); \
        _max = (b) > _max ? (b) : _max; \
        _max; \
    })
#define SWAP(a, b) \
    do { \
        typeof (a) temp = (a); \
        (a) = (b); \
        (b) = temp; \
    } while (0)
#define REMAP(x, in_min, in_max, out_min, out_max) \
    ({ \
        typeof (x) _x = ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min); \
        _x; \
    })
#endif // JEFF_HAS_TYPEOF

#define CLAMP(x, low, high) MIN(MAX(x, low), high)

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

float easing(enum jeff_easing_fn func, enum jeff_easing_t type, float t, float b, float c, float d);
bool flt_cmp(float a, float b);
bool dbl_cmp(double a, double b);

#ifdef __cplusplus
}
#endif
#endif // JEFF_HEAD
