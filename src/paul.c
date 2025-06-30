/* paul.c -- https://github.com/takeiteasy/paul

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

#ifdef PAUL_NO_SCENES
#define SOKOL_NO_ENTRY
#define SOKOL_IMPL
#define SOKOL_INPUT_IMPL
#define SOKOL_MIXER_IMPL
#define SOKOL_IMAGE_IMPL
#define JIM_IMPLEMENTATION
#define MJSON_IMPLEMENTATION
#endif
#include "paul.h"
#define GARRY_IMPLEMENTATION
#include "garry.h"
#define TABLE_IMPLEMENTATION
#include "table.h"
#ifndef PAUL_NO_CONFIG
#define JIM_IMPLEMENTATION
#include "jim.h"
#define MJSON_IMPLEMENTATION
#include "mjson.h"
#endif
#ifdef _WIN32
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

#define _PAUL_SETTINGS                                                                           \
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

extern void update_timers(void);
#ifndef PAUL_NO_INPUT
extern void sapp_input_init(void);
extern void sapp_input_flush(void);
extern void sapp_input_event(const sapp_event*);
extern void check_keymaps(void);
#endif
extern void init_events(void);
extern void check_event(sapp_event_type type);
extern void init_vfs(void);

static struct {
    sapp_desc desc;
    const char *dropped[MAX_DROPPED_FILES];
    int dropped_count;
    char clipboard[CLIPBOARD_SIZE];
#ifndef PAUL_NO_SCENES
    paul_scene_t *scene_prev, *scene_current, *next_scene;
#endif
    paul_app_callback_t init_cb;
    paul_app_callback_t frame_cb;
    paul_app_event_callback_t event_cb;
    paul_app_callback_t exit_cb;
} state = {
    .desc = (sapp_desc) {
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) .VAL = DEFAULT,
        _PAUL_SETTINGS
#undef X
        .window_title = DEFAULT_WINDOW_TITLE
    }
};

#ifndef PAUL_NO_CONFIG
// TODO: Whole config loading needs looking at
// TODO: Add ini parser
static int load_config(const char *path) {
    const char *data = hal_read_file(path, NULL);
    if (data)
        return 0;

    const struct json_attr_t config_attr[] = {
#define X(NAME, TYPE, VAL, DEFAULT,DOCS) \
        {(char*)#NAME, t_##TYPE, .addr.TYPE=&state.desc.VAL},
        _PAUL_SETTINGS
#undef X
        {NULL}
    };
    int status = json_read_object(data, config_attr, NULL);
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
    _PAUL_SETTINGS
#undef X
    jim_object_end(&jim);
    fclose(fh);
    return 1;
}
#endif

#ifndef PAUL_NO_ARGUMENTS
static void usage(const char *name, int exit_code) {
    printf("  usage: %s [options]\n\n  options:\n", name);
    printf("\t  help (flag) -- Show this message\n");
    printf("\t  config (string) -- Path to .json config file\n");
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) \
    printf("\t  %s (%s) -- %s (default: %d)\n", NAME, #TYPE, DOCS, DEFAULT);
    _PAUL_SETTINGS
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
    _PAUL_SETTINGS
#undef X
#undef boolean
#undef integer
    return 1;
}
#endif

bool paul_config(int argc, char* argv[]) {
#ifndef PAUL_NO_CONFIG
    const char *configPath = PAUL_CONFIG_PATH;
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
            return false;
        }
    }
#endif
#ifndef PAUL_NO_ARGUMENTS
    if (argc > 1)
        if (!parse_arguments(argc, argv)) {
            fprintf(stderr, "[PARSE ARGUMENTS ERROR] Failed to parse arguments\n");
            return false;
        }
#endif
    return true;
}

void paul_init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    sapp_input_init();
#ifdef PAUL_WORKING_PATH
    hal_set_working_dir(PAUL_WORKING_PATH);
#endif
#ifndef PAUL_NO_VFS
    init_vfs();
#endif
    init_events();
    paul_srand(PAUL_RNG_SEED);

    if (state.init_cb)
        state.init_cb();

    sapp_run(&state.desc);
}

void paul_frame(void) {
#ifndef PAUL_NO_SCENES
    if (!state.scene_current) {
        sapp_quit();
        return;
    }
#endif

    update_timers();
#ifndef PAUL_NO_SCENES
    state.scene_current->step();
#endif

    if (state.frame_cb)
        state.frame_cb();

    sg_commit();
#ifndef PAUL_NO_INPUT
    sapp_input_flush();
#endif

#ifndef PAUL_NO_SCENES
    if (state.next_scene) {
        if ((state.scene_prev = state.scene_current)) {
            state.scene_current->exit();
            paul_events_clear();
            paul_timers_clear();
        }
        if ((state.scene_current = state.next_scene))
            state.scene_current->enter();
        state.next_scene = NULL;
    }
#endif
}

void paul_event(const sapp_event *event) {
#ifdef PAUL_NO_INPUT
    state.scene_current->event(event);
#else
    sapp_input_event(event);
#endif
    if (state.event_cb)
        state.event_cb(event);
}

void paul_shutdown(void) {
#ifndef PAUL_NO_SCENES
    if (state.scene_current)
        state.scene_current->exit();
#endif
    if (state.exit_cb)
        state.exit_cb();
    sg_shutdown();
}

void paul_set_init_callback(paul_app_callback_t callback) {
    state.init_cb = callback;
}

void paul_set_frame_callback(paul_app_callback_t callback) {
    state.frame_cb = callback;
}

void paul_set_event_callback(paul_app_event_callback_t callback) {
    state.event_cb = callback;
}

void paul_set_exit_callback(paul_app_callback_t callback) {
    state.exit_cb = callback;
}

#ifndef PAUL_NO_SCENES
static paul_scene_t* find_scene(const char *name) {
    size_t name_len = strlen(name);
#define X(NAME) \
    if (!strncmp(name, #NAME, name_len)) \
        return &NAME##_scene;
    PAUL_SCENES
#undef X
    fprintf(stderr, "[ERROR] Unknown scene '%s'", name);
    abort();
}

void paul_set_scene(paul_scene_t *scene) {
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

void paul_set_scene_named(const char *name) {
    paul_set_scene(find_scene(name));
}
#endif
