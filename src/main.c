/* main.c -- https://github.com/takeiteasy/paul 

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

#ifndef PAUL_NO_SCENES
#define SOKOL_IMPL
#define SOKOL_INPUT_IMPL
#define GARRY_IMPLEMENTATION
#define TABLE_IMPLEMENTATION
#define BLA_IMPLEMENTATION
#define PAT_IMPLEMENTATION
#include "paul.h"

extern bool paul_config(int argc, char* argv[]);
extern void paul_init(void);
extern bool paul_frame(void);
extern void paul_event(const sapp_event*);
extern void paul_shutdown(void);

#ifdef PAUL_NO_INPUT
#define X(NAME)                         \
void NAME##_init(void);                 \
void NAME##_enter(void);                \
void NAME##_exit(void);                 \
void NAME##_event(const sapp_event*);   \
void NAME##_step(void);                 \
paul_scene_t NAME##_scene = {           \
    .name = #NAME,                      \
    .enter = NAME##_enter,              \
    .exit = NAME##_exit,                \
    .event = NAME##_event,              \
    .step = NAME##_step                 \
};
#else
#define X(NAME)                         \
void NAME##_init(void);                 \
void NAME##_enter(void);                \
void NAME##_exit(void);                 \
void NAME##_event(const sapp_event*);   \
void NAME##_step(void);                 \
paul_scene_t NAME##_scene = {           \
    .name = #NAME,                      \
    .enter = NAME##_enter,              \
    .exit = NAME##_exit,                \
    .step = NAME##_step                 \
};
#endif
PAUL_SCENES
#undef X

#define _STRINGIFY(s) #s
#define STRINGIFY(S) _STRINGIFY(S)

static void init(void) {
    paul_init();
#ifdef PAUL_FIRST_SCENE
    paul_set_scene_named(STRINGIFY(PAUL_FIRST_SCENE));
#else
#ifndef PAUL_SCENES
#error "No scenes specified"
#else
    scene_t* tmp_scenes[] = {
#define X(NAME) &NAME##_scene,
#undef X
    };
#endif
    paul_set_scene(tmp_scenes[0]);
#endif
}

static void frame(void) {
    paul_frame();
}

static void event(const sapp_event *event) {
    paul_event(event);
}

static void cleanup(void) {
    paul_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
#if defined(PAUL_NO_CONFIG) || defined(PAUL_NO_ARGUMENTS)
    paul_config(argc, argv);
#endif
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup
    };
}
#endif
