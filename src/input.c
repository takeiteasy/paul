/* jeff/input.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2025  George Watson

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
#define SUK_IMPL
#include "sokol_input.h"
#include "garry.h"
#include "table.h"

typedef struct keymap {
    input_state_t input;
    // TODO: Add action type (up/down)
    void *userdata;
} keymap_t;

typedef struct sevent {
    const char *event;
    sapp_event_callback_t cb;
} sevent;

// TODO: Thread safety
static struct {
    table_t keymap;
    sevent* events[_SAPP_EVENTTYPE_NUM];
} _state;

void init_keymap(void) {
    memset(&_state.keymap, 0, sizeof(table_t));
    memset(&_state.events, 0, _SAPP_EVENTTYPE_NUM * sizeof(sevent*));
    _state.keymap = table_new();
    // TODO: Import/export keymap to JSON
}

bool sapp_on_input_str(const char *input_str, const char *event, void *userdata) {
    keymap_t *keymap = malloc(sizeof(keymap_t));
    if (!sapp_create_input_str(&keymap->input, input_str)) {
        free(keymap);
        return false;
    }
    keymap->userdata = userdata;
    table_set(&_state.keymap, event, (void*)keymap);
    return true;
}

bool sapp_on_input(const char *event, void *userdata, int modifiers, int n, ...) {
    keymap_t keymap;
    va_list args;
    va_start(args, n);
    if (!sapp_create_input(&keymap.input, modifiers, n, args))
        return false;
    keymap.userdata = userdata;
    keymap_t *arr = NULL;
    if (table_has(&_state.keymap, event)) {
        table_get(&_state.keymap, event, (void**)&arr);
        garry_append(arr, keymap);
    } else {
        garry_append(arr, keymap);
        table_set(&_state.keymap, event, (void*)arr);
    }
    return true;
}

void sapp_remove_input_event(const char *event) {
    if (table_has(&_state.keymap, event)) {
        keymap_t *old = NULL;
        table_get(&_state.keymap, event, (void**)&old);
        if (old)
            garry_free(old);
        table_del(&_state.keymap, event);
    }
}

static bool sapp_check_state(input_state_t *istate) {
    bool mod_check = true;
    bool key_check = true;
    if (istate->modifiers != 0)
        mod_check = sapp_modifier_equal(istate->modifiers);
    if (istate->keys > 0)
        for (int i = 0; i < garry_count(istate->keys); i++)
            if (!sapp_is_key_down(istate->keys[i])) {
                key_check = false;
                break;
            }
    return mod_check && key_check;
}

static int _check(table_pair_t *pair, void *userdata) {
    keymap_t *keymap = (keymap_t*)pair->value;
    if (keymap && sapp_check_state(&keymap->input))
        jeff_event_emit(pair->key.string);
    return 1;
}

void check_keymaps(void) {
    table_each(&_state.keymap, _check, NULL);
}

void sapp_emit_on_event(sapp_event_type event_type, const char *event) {
    sevent e = {
        .event = strdup(event),
        .cb = NULL
    };
    garry_append(_state.events[event_type], e);
}

void sapp_on_event(sapp_event_type event_type, sapp_event_callback_t callback) {
    sevent e = {
        .event = NULL,
        .cb = callback
    };
    garry_append(_state.events[event_type], e);
}

int _free(table_pair_t *pair, void *userdata) {
    keymap_t *keymap = (keymap_t*)pair->value;
    if (keymap)
        garry_free(keymap);
    return 1;
}

void sapp_clear_events(void) {
    for (int i = 0; i < _SAPP_EVENTTYPE_NUM; i++)
        if (_state.events[i])
            garry_free(_state.events[i]);
    memset(&_state.events, 0, _SAPP_EVENTTYPE_NUM * sizeof(sevent*));
    table_each(&_state.keymap, _free, NULL);
    table_free(&_state.keymap);
    _state.keymap = table_new();
}

void check_event(sapp_event_type type) {
    if (!_state.events[type])
        return;
    for (int i = 0; i < garry_count(_state.events[type]); i++) {
        sevent *event = &_state.events[type][i];
        if (event->event != NULL)
            jeff_event_emit(event->event);
    }
}

void sapp_remove_event(sapp_event_type type) {
    if (_state.events[type])
        garry_free(_state.events[type]);
}
