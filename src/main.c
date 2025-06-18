/* main.c -- https://github.com/takeiteasy/jeff 

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

#ifndef JEFF_NO_SCENES
#define SOKOL_IMPL
#define SOKOL_INPUT_IMPL
#define SOKOL_MIXER_IMPL
#define SOKOL_IMAGE_IMPL
#define GARRY_IMPLEMENTATION
#define TABLE_IMPLEMENTATION
#include "jeff.h"

extern bool jeff_config(int argc, char* argv[]);
extern void jeff_init(void);
extern bool jeff_frame(void);
extern void jeff_event(const sapp_event*);
extern void jeff_shutdown(void);

#ifdef JEFF_NO_INPUT
#define X(NAME)                         \
void NAME##_init(void);                 \
void NAME##_enter(void);                \
void NAME##_exit(void);                 \
void NAME##_event(const sapp_event*);   \
void NAME##_step(void);                 \
jeff_scene_t NAME##_scene = {           \
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
jeff_scene_t NAME##_scene = {           \
    .name = #NAME,                      \
    .enter = NAME##_enter,              \
    .exit = NAME##_exit,                \
    .step = NAME##_step                 \
};
#endif
JEFF_SCENES
#undef X

#define _STRINGIFY(s) #s
#define STRINGIFY(S) _STRINGIFY(S)

static void init(void) {
    jeff_init();
#ifdef JEFF_FIRST_SCENE
    jeff_set_scene_named(STRINGIFY(JEFF_FIRST_SCENE));
#else
#ifndef JEFF_SCENES
#error "No scenes specified"
#else
    scene_t* tmp_scenes[] = {
#define X(NAME) &NAME##_scene,
#undef X
    };
#endif
    jeff_set_scene(tmp_scenes[0]);
#endif
}

static void frame(void) {
    jeff_frame();
}

static void event(const sapp_event *event) {
    jeff_event(event);
}

static void cleanup(void) {
    jeff_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    jeff_config(argc, argv);
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup
    };
}
#endif
