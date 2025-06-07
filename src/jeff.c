/* jeff.c -- https://github.com/takeiteasy/jeff 

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

#define SOKOL_IMPL
#include "jeff.h"
#ifndef JEFF_NO_ARGUMENTS
#include "sokol/sokol_args.h"
#endif
#define JIM_IMPLEMENTATION
#include "jim.h"
#define MJSON_IMPLEMENTATION
#include "mjson.h"
#define GARRY_IMPLEMENTATION
#include "garry.h"
#define TABLE_IMPLEMENTATION
#include "table.h"

#ifdef PLATFORM_MAC
#include <Cocoa/Cocoa.h>
#endif

#define _STRINGIFY(s) #s
#define STRINGIFY(S) _STRINGIFY(S)

#ifndef MAX_PATH
#if defined(PLATFORM_MAC)
#define MAX_PATH 255
#elif defined(PLATFORM_WINDOWS)
#define MAX_PATH 256
#elif defined(PLATFORM_LINUX)
#define MAX_PATH 4096
#endif
#endif

#define SETTINGS                                                                                 \
    X("width", integer, width, DEFAULT_WINDOW_WIDTH, "Set window width")                         \
    X("height", integer, height, DEFAULT_WINDOW_HEIGHT, "Set window height")                     \
    X("sampleCount", integer, sample_count, 4, "Set the MSAA sample count of the   framebuffer") \
    X("swapInterval", integer, swap_interval, 1, "Set the preferred swap interval")              \
    X("highDPI", boolean, high_dpi, true, "Enable high-dpi compatability")                       \
    X("fullscreen", boolean, fullscreen, false, "Set fullscreen")                                \
    X("alpha", boolean, alpha, false, "Enable/disable alpha channel on framebuffers")            \
    X("clipboard", boolean, enable_clipboard, false, "Enable clipboard support")                 \
    X("clipboardSize", integer, clipboard_size, 1024, "Size of clipboard buffer (in bytes)")     \
    X("drapAndDrop", boolean, enable_dragndrop, false, "Enable drag-and-drop files")             \
    X("maxDroppedFiles", integer, max_dropped_files, 1, "Max number of dropped files")           \
    X("maxDroppedFilesPathLength", integer, max_dropped_file_path_length, MAX_PATH, "Max path length for dropped files")

struct {
    sg_color clear_color;
    sapp_desc desc;
    sg_pass_action pass_action;

    const char *dropped[MAX_DROPPED_FILES];
    int dropped_count;
    char clipboard[CLIPBOARD_SIZE];
    scene_t *scene_prev, *scene_current, *next_scene;
} state = {
    .desc = (sapp_desc) {
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) .VAL = DEFAULT,
        SETTINGS
#undef X
        .window_title = DEFAULT_WINDOW_TITLE
    },
    .pass_action = {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {0}
        }
    }
};

int sapp_framebuffer_width(void) {
    return sapp_width() * (int)sapp_framebuffer_scalefactor();
}

int sapp_framebuffer_height(void) {
    return sapp_height() * (int)sapp_framebuffer_scalefactor();
}

float sapp_framebuffer_widthf(void) {
    return sapp_widthf() * sapp_framebuffer_scalefactor();
}

float sapp_framebuffer_heightf(void) {
    return sapp_heightf() * sapp_framebuffer_scalefactor();
}

float sapp_framebuffer_scalefactor(void) {
#ifdef PLATFORM_MAC
    return [[NSScreen mainScreen] backingScaleFactor];
#else
    return 1.f; // I don't know if this is a thing for Windows/Linux so return 1
#endif
}

#ifdef JEFF_NO_INPUT
#define X(NAME)                         \
void NAME##_init(void);                 \
void NAME##_enter(void);                \
void NAME##_exit(void);                 \
void NAME##_event(const sapp_event*);   \
void NAME##_step(void);                 \
scene_t NAME##_scene = {                \
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
scene_t NAME##_scene = {                \
    .name = #NAME,                      \
    .enter = NAME##_enter,              \
    .exit = NAME##_exit,                \
    .step = NAME##_step                 \
};
#endif
JEFF_SCENES
#undef X

static scene_t* find_scene(const char *name) {
    size_t name_len = strlen(name);
#define X(NAME) \
    if (!strncmp(name, #NAME, name_len)) \
        return &NAME##_scene;
    JEFF_SCENES
#undef X
    fprintf(stderr, "[ERROR] Unknown scene '%s'", name);
    abort();
}

void jeff_set_scene(scene_t *scene) {
    if (!scene) {
        sapp_quit();
        return;
    }

    if (!state.scene_current) {
        state.scene_current = scene;
        state.scene_current->enter();
    } else
        if (strncmp(scene->name, state.scene_current->name, strlen(scene->name)))
            state.next_scene = scene;
}

void jeff_set_scene_named(const char *name) {
    jeff_set_scene(find_scene(name));
}

#ifndef JEFF_NO_CONFIG
extern unsigned char* read_file(const char *path, size_t *length);

// TODO: Add ini parser
static int load_config(const char *path) {
    unsigned char *data = read_file(path, NULL);
    if (data)
        return 0;

    const struct json_attr_t config_attr[] = {
#define X(NAME, TYPE, VAL, DEFAULT,DOCS) \
        {(char*)#NAME, t_##TYPE, .addr.TYPE=&state.desc.VAL},
        SETTINGS
#undef X
        {NULL}
    };
    int status = json_read_object((const char*)data, config_attr, NULL);
    if (!status)
        return 0;
    free((void*)data);
    return 1;
}

#define jim_boolean jim_bool

static int export_config(const char *path) {
    FILE *fh = fopen(path, "w");
    if (!fh)
        return 0;
    Jim jim = {
        .sink = fh,
        .write = (Jim_Write)fwrite
    };
    jim_object_begin(&jim);
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) \
    jim_member_key(&jim, NAME);           \
    jim_##TYPE(&jim, state.desc.VAL);
    SETTINGS
#undef X
    jim_object_end(&jim);
    fclose(fh);
    return 1;
}
#endif

#ifndef JEFF_NO_ARGUMENTS
static void usage(const char *name, int exit_code) {
    printf("  usage: %s [options]\n\n  options:\n", name);
    printf("\t  help (flag) -- Show this message\n");
    printf("\t  config (string) -- Path to .json config file\n");
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) \
    printf("\t  %s (%s) -- %s (default: %d)\n", NAME, #TYPE, DOCS, DEFAULT);
    SETTINGS
#undef X
    exit(exit_code);
}

static int parse_arguments(int argc, char *argv[]) {
    const char *name = argv[0];
    sargs_desc desc = (sargs_desc) {
#ifdef EMSCRIPTEN
        .argc = argc,
        .argv = (char**)argv
#else
        .argc = argc - 1,
        .argv = (char**)(argv + 1)
#endif
    };
    sargs_setup(&desc);

#define boolean 1
#define integer 0
#define X(NAME, TYPE, VAL, DEFAULT, DOCS)                                               \
    if (sargs_exists(NAME))                                                             \
    {                                                                                   \
        const char *tmp = sargs_value_def(NAME, #DEFAULT);                              \
        if (!tmp)                                                                       \
        {                                                                               \
            fprintf(stderr, "[ARGUMENT ERROR] No value passed for \"%s\"\n", NAME);     \
            usage(name, 1);                                                             \
            return 0;                                                                   \
        }                                                                               \
        if (TYPE == 1)                                                                  \
            state.desc.VAL = (int)atoi(tmp);                                            \
        else                                                                            \
            state.desc.VAL = sargs_boolean(NAME);                                       \
    }
    SETTINGS
#undef X
#undef boolean
#undef integer
    return 1;
}
#endif

extern void sapp_input_clear(void);
extern void sapp_input_handler(const sapp_event* e);
extern void sapp_input_update(void);
extern void update_timers(void);
extern void init_keymap(void);
extern void check_keymaps(void);

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    sapp_input_clear();
    init_keymap();
    rng_srand(JEFF_RNG_SEED);
#ifdef JEFF_DEFAULT_PATH
    jeff_set_working_dir(JEFF_DEFAULT_PATH);
#endif
#ifdef JEFF_ASSET_PATH
    vfs_mount(JEFF_ASSET_PATH);
#endif

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
    if (!state.scene_current) {
        sapp_quit();
        return;
    }
    update_timers();
    check_keymaps();
    sg_begin_pass(&(sg_pass) {
        .action = state.pass_action,
        .swapchain = sglue_swapchain()
    });
    state.scene_current->step();
    sg_end_pass();
    sg_commit();
    sapp_input_update();

    if (state.next_scene) {
        if ((state.scene_prev = state.scene_current)) {
            state.scene_current->exit();
            clear_all_events();
            clear_all_timers();
        }
        if ((state.scene_current = state.next_scene))
            state.scene_current->enter();
        state.next_scene = NULL;
    }
}

static void event(const sapp_event *event) {
#ifdef JEFF_NO_INPUT
    state.scene_current->event(event);
#else
    sapp_input_handler(event);
#endif
}

static void cleanup(void) {
    if (state.scene_current)
        state.scene_current->exit();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
#ifndef JEFF_NO_CONFIG
    const char *configPath = JEFF_CONFIG_PATH;
    if (!access(configPath, F_OK)) {
        if (!load_config(configPath)) {
            fprintf(stderr, "[IMPORT CONFIG ERROR] Failed to import config from \"%s\"\n", configPath);
            fprintf(stderr, "errno (%d): \"%s\"\n", errno, strerror(errno));
            goto EXPORT_CONFIG;
        }
    } else {
    EXPORT_CONFIG:
        if (!export_config(configPath)) {
            fprintf(stderr, "[EXPORT CONFIG ERROR] Failed to export config to \"%s\"\n", configPath);
            fprintf(stderr, "errno (%d): \"%s\"\n", errno, strerror(errno));
            abort();
        }
    }
#endif
#ifndef JEFF_NO_ARGUMENTS
    if (argc > 1)
        if (!parse_arguments(argc, argv)) {
            fprintf(stderr, "[PARSE ARGUMENTS ERROR] Failed to parse arguments\n");
            abort();
        }
#endif

    state.desc.init_cb = init;
    state.desc.frame_cb = frame;
    state.desc.event_cb = event;
    state.desc.cleanup_cb = cleanup;
    return state.desc;
}
