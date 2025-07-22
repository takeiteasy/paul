/* paul.h -- https://github.com/takeiteasy/paul 

 paul Copyright (C) 2025 George Watson

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

#ifndef PAUL_HEAD
#define PAUL_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __has_include
#define __has_include(x) 0
#endif
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_extension
#define __has_extension __has_feature
#endif

#if __STDC_VERSION__ >= 202311L
#define PAUL_HAS_TYPEOF
#elif defined(COMPILER_GCC)
#if __GNUC__ > 10 || (__GNUC__ == 10 && __GNUC_MINOR__ >= 4)
#define typeof(X) __typeof__((X))
#define PAUL_HAS_TYPEOF
#endif
#elif defined(COMPILER_CLANG)
#if __clang_major__ >= 15
#define typeof(X) __typeof__((X))
#define PAUL_HAS_TYPEOF
#endif
#endif

#if __has_extension(c_generic_selections)
#define PAUL_HAS_GENERICS
#else
#if defined(__STDC__) && __STDC_VERSION__ < 202311L
#define PAUL_HAS_GENERICS
#endif
#endif

#ifndef PAUL_HAS_GENERICS
#error paul requires _Generic support
#endif

#if defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#if __has_extension(blocks)
#define PAUL_HAS_BLOCKS
#endif
#endif

#ifdef PAUL_USE_BLOCKS
#ifndef PAUL_HAS_BLOCKS
#error This platform doesn't support blocks, undefine PAUL_USE_BLOCKS
#endif
#define GENERIC_USE_BLOCKS
#endif

#ifdef PAUL_USE_BLOCKS
#define _CALLBACK_TYPEDEF(RET, NAME, ...) typedef RET(^NAME)(__VA_ARGS__)
#else
#define _CALLBACK_TYPEDEF(RET, NAME, ...) typedef RET(*NAME)(__VA_ARGS__)
#endif

#if !defined(PAUL_NO_SCENES) && !defined(PAUL_NO_HEADER_CONFIG)
#if __has_include("paul_config.h")
#include "paul_config.h"
#endif
#endif

#define HAL_NO_CLIPBOARD
#define HAL_NO_STORAGE_PATH
#include "hal/hal.h"

#if !defined(PAUL_NO_SCENES) && !defined(PAUL_SCENES)
#ifndef PAUL_FIRST_SCENE
#error No scenes! Define PAUL_SCENES or PAUL_FIRST_SCENE first
#else
#define PAUL_SCENES X(PAUL_FIRST_SCENE)
#endif
#endif

#ifndef DEFAULT_WINDOW_WIDTH
#define DEFAULT_WINDOW_WIDTH 640
#endif

#ifndef DEFAULT_WINDOW_HEIGHT
#define DEFAULT_WINDOW_HEIGHT 480
#endif

#ifndef PAUL_NAME
#define PAUL_NAME "paul"
#endif

#ifndef DEFAULT_WINDOW_TITLE
#define DEFAULT_WINDOW_TITLE PAUL_NAME
#endif

#ifndef MAX_TEXTURE_STACK
#define MAX_TEXTURE_STACK 8
#endif

// TODO: Frame timing
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
#define DEFAULT_CONFIG_NAME "." PAUL_NAME ".json"
#else
#define DEFAULT_CONFIG_NAME PAUL_NAME ".json"
#endif
#endif
#ifndef PAUL_CONFIG_PATH
#define PAUL_CONFIG_PATH DEFAULT_CONFIG_NAME
#endif

#ifndef PAUL_RNG_SEED
#define PAUL_RNG_SEED 0
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
#ifndef PAUL_NO_ARGUMENTS
#include "sokol/sokol_args.h"
#endif
#ifndef PAUL_NO_INPUT
#include "sokol_input.h"
#endif
#include "bla.h"
#include "pat.h"
#include "garry.h"
#include "table.h"

#ifndef PAUL_NO_SCENES
typedef struct paul_scene_t {
    const char *name;
    void(*enter)(void);
    void(*exit)(void);
#ifdef PAUL_NO_INPUT
    void(*event)(const sapp_event*);
#endif
    void(*step)(void);
} paul_scene_t;

#define X(NAME) extern paul_scene_t NAME##_scene;
PAUL_SCENES
#undef X

void paul_set_scene(paul_scene_t *scene);
void paul_set_scene_named(const char *name);
#else
bool paul_config(int argc, char* argv[]);
void paul_init(void);
void paul_frame(void);
void paul_event(const sapp_event*);
void paul_shutdown(void);
#endif

_CALLBACK_TYPEDEF(void, paul_app_callback_t, void);
_CALLBACK_TYPEDEF(void, paul_app_event_callback_t, const sapp_event*);
void paul_set_init_callback(paul_app_callback_t);
void paul_set_frame_callback(paul_app_callback_t);
void paul_set_event_callback(paul_app_event_callback_t);
void paul_set_exit_callback(paul_app_callback_t);

void paul_srand(uint64_t seed);
uint64_t paul_rand(void);
float paul_randf(void);

void paul_cellular_automata(unsigned int width, unsigned int height, unsigned int fill_chance, unsigned int smooth_iterations, unsigned int survive, unsigned int starve, uint8_t* dst);
void paul_noise_fbm(unsigned int width, unsigned int height, float z, float offset_x, float offset_y, float scale, float lacunarity, float gain, int octaves, uint8_t* dst);
// TODO: Poisson disc sampling
// TODO: A*

#ifdef __cplusplus
}
#endif
#endif // PAUL_HEAD
