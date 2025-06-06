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

#ifdef JEFF_NO_THREADS
#define PAUL_NO_THREADS
#endif
#define PAUL_NO_CLIPBOARD
#include "paul/paul.h"

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
#define DEFAULT_WINDOW_TITLE "jeff"
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
bool jeff_set_working_dir(const char *path);

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
bool sapp_modified_equals(int mods);
bool sapp_modifier_down(int mod);
bool sapp_check_input(const char *str); // WIP
void sapp_create_input(input_state_t *dst, int modifiers, int n, ...);
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
float* audio_read_all_samples(audio_t *audio);
float audio_sample(audio_t *audio, int frame);
void audio_read_samples(audio_t *audio, int start_frame, int end_frame, float *dst);
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

#ifndef JEFF_NO_THREADS
typedef struct thread_work {
    void(*func)(void*);
    void *arg;
    struct thread_work *next;
} thread_work_t;

typedef struct thread_pool {
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

typedef struct thread_queue_entry {
    void *data;
    struct thread_queue_entry *next;
} thread_queue_entry_t;

typedef struct thread_queue {
    thread_queue_entry_t *head, *tail;
    paul_mtx_t readLock, writeLock;
    size_t count;
} thread_queue_t;

thread_queue_t* thread_queue(void);
void thread_queue_push(thread_queue_t *queue, void *data);
void* thread_queue_pop(thread_queue_t *queue);
void thread_queue_destroy(thread_queue_t *queue);
#endif

#ifdef __cplusplus
}
#endif
#endif // JEFF_HEAD
