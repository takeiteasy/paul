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

#ifndef JEFF_HEAD // 👺
#define JEFF_HEAD // 👺
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define COMPILER_CL

#ifdef _WIN32
#define PLATFORM_WINDOWS
#else
#error compiler/platform combo is not supported yet
#endif

#if defined(_M_AMD64)
#define ARCH_X64
#elif defined(_M_IX86)
#define ARCH_X86
#elif defined(_M_ARM64)
#define ARCH_ARM64
#elif defined(_M_ARM)
#define ARCH_ARM32
#else
#error architecture not supported yet
#endif

#if _MSC_VER >= 1920
#define COMPILER_CL_YEAR 2019
#elif _MSC_VER >= 1910
#define COMPILER_CL_YEAR 2017
#elif _MSC_VER >= 1900
#define COMPILER_CL_YEAR 2015
#elif _MSC_VER >= 1800
#define COMPILER_CL_YEAR 2013
#elif _MSC_VER >= 1700
#define COMPILER_CL_YEAR 2012
#elif _MSC_VER >= 1600
#define COMPILER_CL_YEAR 2010
#elif _MSC_VER >= 1500
#define COMPILER_CL_YEAR 2008
#elif _MSC_VER >= 1400
#define COMPILER_CL_YEAR 2005
#else
#define COMPILER_CL_YEAR 0
#endif
// defined(_MSC_VER)
#elif defined(__clang__)
#define COMPILER_CLANG

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MAC
#elif defined(__gnu_linux__)
#define PLATFORM_LINUX
#else
#error compiler/platform combo is not supported yet
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86
#elif defined(__aarch64__)
#define ARCH_ARM64
#elif defined(__arm__)
#define ARCH_ARM32
#else
#error architecture not supported yet
#endif
// defined(__clang__)
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC

#ifdef __gnu_linux__
#define PLATFORM_LINUX
#else
#error compiler/platform combo is not supported yet
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86
#elif defined(__aarch64__)
#define ARCH_ARM64
#elif defined(__arm__)
#define ARCH_ARM32
#else
#error architecture not supported yet
#endif
#else
#error compiler is not supported yet
#endif

#if defined(ARCH_X64)
#define ARCH_64BIT
#elif defined(ARCH_X86)
#define ARCH_32BIT
// defined(__GNUC__) || defined(__GNUP__)
#endif

#ifdef __cplusplus
#define LANG_CPP
#else
#define LANG_C
#endif

#ifndef PLATFORM_WINDOWS
#define PLATFORM_POSIX
#endif

#ifndef __has_include
#define __has_include(x) 1
#endif

#if __has_include("jeff_config.h")
#include "jeff_config.h"
#endif

#if !defined(JEFF_SCENES) && !defined(JEFF_FIRST_SCENE)
#error "No scenes specified"
#endif

#ifndef JEFF_SCENES
#define JEFF_SCENES X(JEFF_FIRST_SCENE)
#endif

#ifndef DEFAULT_WINDOW_WIDTH
#define DEFAULT_WINDOW_WIDTH 640
#endif

#ifndef DEFAULT_WINDOW_HEIGHT
#define DEFAULT_WINDOW_HEIGHT 480
#endif

#ifndef DEFAULT_WINDOW_TITLE
#ifdef JEFF_NAME
#define DEFAULT_WINDOW_TITLE JEFF_NAME
#else
#define DEFAULT_WINDOW_TITLE "👺"
#endif
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

#ifndef DEFAULT_CONFIG_NAME
#ifdef PLATFORM_POSIX
#define DEFAULT_CONFIG_NAME ".jeff.json"
#else
#define DEFAULT_CONFIG_NAME "jeff.json"
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

#ifndef LANG_CPP
#if defined(COMPILER_CL) && COMPILER_CL_YEAR < 2017
#include <windef.h>
#define bool BOOL
#define true 1
#define false 0
#else
#if defined(__STDC__) && __STDC_VERSION__ < 199901L
typedef enum bool { false = 0, true = !false } bool;
#else
#include <stdbool.h>
#endif // __STDC__
#endif // COMPILER_CL
#endif // LANG_CPP
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

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#include <emscripten.h>
#endif

#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_audio.h"
#include "sokol/sokol_log.h"
#include "sokol/sokol_time.h"
#include "pthread_shim.h"

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

#ifndef N_ARGS
#define N_ARGS(...) _NARG_(__VA_ARGS__, _RSEQ())
#define _NARG_(...) _SEQ(__VA_ARGS__)
#define _SEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,N,...) N
#define _RSEQ() 127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
#endif

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
typedef struct input_state {
    int keys[MAX_INPUT_STATE_KEYS];
    int modifiers;
} input_state_t;

bool sapp_is_key_down(int key);
// This will be true if a key is held for more than 1 second
// TODO: Make the duration configurable
bool sapp_is_key_held(int key);
// Returns true if key is down this frame and was up last frame
bool sapp_was_key_pressed(int key);
// Returns true if key is up and key was down last frame
bool sapp_was_key_released(int key);
// If any of the keys passed are not down returns false
bool __sapp_are_keys_down(int n, ...);
#define sapp_are_keys_down(...) __sapp_are_keys_down(N_ARGS(__VA_ARGS__), __VA_ARGS__)
// If none of the keys passed are down returns false
bool __sapp_any_keys_down(int n, ...);
#define sapp_any_keys_down(...) __sapp_any_keys_down(N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool sapp_is_button_down(int button);
bool sapp_is_button_held(int button);
bool sapp_was_button_pressed(int button);
bool sapp_was_button_released(int button);
bool __sapp_are_buttons_down(int n, ...);
#define sapp_are_buttons_down(...) __sapp_are_buttons_down(N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool __sapp_any_buttons_down(int n, ...);
#define sapp_any_buttons_down(...) __sapp_any_buttons_down(N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool sapp_has_mouse_move(void);
bool sapp_modified_equals(int mods);
bool sapp_modifier_down(int mod);
bool sapp_check_input(const char *str); // WIP
void __sapp_create_input(input_state_t *dst, int modifiers, int n, ...);
#define sapp_create_input(dst, mods ...) __sapp_create_input(dst, mods, N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool sapp_create_input_str(input_state_t *dst, const char *str);
bool sapp_check_state(input_state_t *state);
int sapp_cursor_x(void);
int sapp_cursor_y(void);
bool sapp_cursor_delta_x(void);
bool sapp_cursor_delta_y(void);
bool sapp_check_scrolled(void);
float sapp_scroll_x(void);
float sapp_scroll_y(void);

// TODO: Import/export keymap to JSON
// bool keymap_import(const char *path);
// bool keymap_export(const char *path);
// TODO: Add 'action' type to keymap_set (match sapp events)
bool keymap_set(const char *input_str, const char *event, void *userdata);
void keymap_unset(const char *event);
#endif

bool jeff_set_working_dir(const char *path);
void vfs_mount(const char *path);
bool vfs_exists(const char *filename);
unsigned char *vfs_read(const char *filename, size_t *size);
// TODO: vfs_write
// bool vfs_write(const char *filename, unsigned char *data, size_t size);

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

#ifndef JEFF_NO_COLORS
enum {
    IndianRed = -3318692,
    LightCoral = -1015680,
    Salmon = -360334,
    DarkSalmon = -1468806,
    LightSalmon = -24454,
    Crimson = -2354116,
    Red = -65536,
    FireBrick = -5103070,
    DarkRed = -7667712,
    Pink = -16181,
    LightPink = -18751,
    HotPink = -38476,
    DeepPink = -60269,
    MediumVioletRed = -3730043,
    PaleVioletRed = -2396013,
    Coral = -32944,
    Tomato = -40121,
    OrangeRed = -47872,
    DarkOrange = -29696,
    Orange = -23296,
    Gold = -10496,
    Yellow = -256,
    LightYellow = -32,
    LemonChiffon = -1331,
    LightGoldenrodYellow = -329006,
    PapayaWhip = -4139,
    Moccasin = -6987,
    PeachPuff = -9543,
    PaleGoldenrod = -1120086,
    Khaki = -989556,
    DarkKhaki = -4343957,
    Lavender = -1644806,
    Thistle = -2572328,
    Plum = -2252579,
    Violet = -1146130,
    Orchid = -2461482,
    Fuchsia = -65281,
    Magenta = -65281,
    MediumOrchid = -4565549,
    MediumPurple = -7114533,
    RebeccaPurple = -10079335,
    BlueViolet = -7722014,
    DarkViolet = -7077677,
    DarkOrchid = -6737204,
    DarkMagenta = -7667573,
    Purple = -8388480,
    Indigo = -11861886,
    SlateBlue = -9807155,
    DarkSlateBlue = -12042869,
    MediumSlateBlue = -8689426,
    GreenYellow = -5374161,
    Chartreuse = -8388864,
    LawnGreen = -8586240,
    Lime = -16711936,
    LimeGreen = -13447886,
    PaleGreen = -6751336,
    LightGreen = -7278960,
    MediumSpringGreen = -16713062,
    SpringGreen = -16711809,
    MediumSeaGreen = -12799119,
    SeaGreen = -13726889,
    ForestGreen = -14513374,
    Green = -16744448,
    DarkGreen = -16751616,
    YellowGreen = -6632142,
    OliveDrab = -9728477,
    Olive = -8355840,
    DarkOliveGreen = -11179217,
    MediumAquamarine = -10039894,
    DarkSeaGreen = -7357301,
    LightSeaGreen = -14634326,
    DarkCyan = -16741493,
    Teal = -16744320,
    Aqua = -16711681,
    Cyan = -16711681,
    LightCyan = -2031617,
    PaleTurquoise = -5247250,
    Aquamarine = -8388652,
    Turquoise = -12525360,
    MediumTurquoise = -12004916,
    DarkTurquoise = -16724271,
    CadetBlue = -10510688,
    SteelBlue = -12156236,
    LightSteelBlue = -5192482,
    PowderBlue = -5185306,
    LightBlue = -5383962,
    SkyBlue = -7876885,
    LightSkyBlue = -7876870,
    DeepSkyBlue = -16728065,
    DodgerBlue = -14774017,
    CornflowerBlue = -10185235,
    RoyalBlue = -12490271,
    Blue = -16776961,
    MediumBlue = -16777011,
    DarkBlue = -16777077,
    Navy = -16777088,
    MidnightBlue = -15132304,
    Cornsilk = -1828,
    BlanchedAlmond = -5171,
    Bisque = -6972,
    NavajoWhite = -8531,
    Wheat = -663885,
    BurlyWood = -2180985,
    Tan = -2968436,
    RosyBrown = -4419697,
    SandyBrown = -744352,
    Goldenrod = -2448096,
    DarkGoldenrod = -4684277,
    Peru = -3308225,
    Chocolate = -2987746,
    SaddleBrown = -7650029,
    Sienna = -6270419,
    Brown = -5952982,
    Maroon = -8388608,
    White = -1,
    Snow = -1286,
    HoneyDew = -983056,
    MintCream = -655366,
    Azure = -983041,
    AliceBlue = -984833,
    GhostWhite = -460545,
    WhiteSmoke = -657931,
    SeaShell = -2578,
    Beige = -657956,
    OldLace = -133658,
    FloralWhite = -1296,
    Ivory = -16,
    AntiqueWhite = -332841,
    Linen = -331546,
    LavenderBlush = -3851,
    MistyRose = -6943,
    Gainsboro = -2302756,
    LightGray = -2894893,
    Silver = -4144960,
    DarkGray = -5658199,
    Gray = -8355712,
    DimGray = -9868951,
    LightSlateGray = -8943463,
    SlateGray = -9404272,
    DarkSlateGray = -13676721,
    Black = -16777216,
};
#endif // JEFF_NO_COLORS

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

typedef struct {
    unsigned int width, height;
    int *buffer;
} image_t;

image_t* image_empty(unsigned int w, unsigned int h);
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
void image_pass_thru(image_t *img, int(*fn)(int x, int y, int col));
void image_resize(image_t *src, int nw, int nh);
void image_rotate(image_t *src, float angle);

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

typedef struct Wave {
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
float* audio_samples(audio_t *audio);
float audio_length(audio_t *audio);

typedef void(*event_callback_t)(void*);

void on_event(const char *name, event_callback_t cb, void *userdata);
void on_event_once(const char *name, event_callback_t cb, void *userdata);
void remove_event_named(const char *name);
int event_listener_count(const char *name);
void emit_event(const char *name);
void clear_all_events(void);

typedef void(*timer_callback_t)(void*);

void timer_every(const char *name, int64_t ms, timer_callback_t cb, void *userdata);
void timer_emit_every(const char *name, int64_t ms, const char *event, void *userdata);
void timer_after(const char *name, int64_t ms, timer_callback_t cb, void *userdata);
void timer_emit_after(const char *name, int64_t ms, const char *event, void *userdata);
void remove_timer_named(const char *name);
void clear_all_timers(void);

void thread_sleep(const struct timespec *timeout);
void thread_yield(void);
struct timespec thread_timeout(unsigned int milliseconds);

typedef struct thread_work {
    void(*func)(void*);
    void *arg;
    struct thread_work *next;
} thread_work_t;

typedef struct thread_pool {
    thread_work_t *head, *tail;
    pthread_mutex_t workMutex;
    pthread_cond_t workCond;
    pthread_cond_t workingCond;
    size_t workingCount;
    size_t threadCount;
    int kill;
} thread_pool_t;

thread_pool_t* thread_pool(size_t maxThreads);
void thread_pool_destroy(thread_pool_t *pool);
int thread_pool_push_work(thread_pool_t *pool, void(*func)(void*), void *arg);
// Adds work to the front of the queue
int thread_pool_push_work_priority(thread_pool_t *pool, void(*func)(void*), void *arg);
void thread_pool_join(thread_pool_t *pool);

typedef struct thread_queue_entry {
    void *data;
    struct thread_queue_entry *next;
} thread_queue_entry_t;

typedef struct thread_queue {
    thread_queue_entry_t *head, *tail;
    pthread_mutex_t readLock, writeLock;
    size_t count;
} thread_queue_t;

thread_queue_t* thread_queue(void);
void thread_queue_push(thread_queue_t *queue, void *data);
void* thread_queue_pop(thread_queue_t *queue);
void thread_queue_destroy(thread_queue_t *queue);

#ifdef __cplusplus
}
#endif
#endif // JEFF_HEAD 👺
