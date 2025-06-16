/* jeff/events.c -- https://github.com/takeiteasy/jeff

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
#ifndef JEFF_NO_INPUT
#define SUK_IMPL
#include "sokol_input.h"
#endif
#include "garry.h"
#include "table.h"

#ifndef JEFF_NO_INPUT
typedef struct keymap {
    input_state_t input;
    // TODO: Add action type (up/down)
    void *userdata;
} keymap_t;

typedef struct sevent {
    const char *event;
    sapp_event_callback_t cb;
} sevent;
#endif

typedef struct listener {
    const char *name;
    jeff_event_callback_t cb;
    bool once;
    void *userdata;
} listener_t;

typedef struct timer {
    const char *name;
    int64_t duration;
    int64_t start;
    bool loop;
    jeff_timer_callback_t cb;
    const char *event;
    void *userdata;
} timer_t;

// TODO: Thread safety
static struct {
    table_t timers;
    table_t events;
#ifndef JEFF_NO_INPUT
    table_t keymap;
    sevent* sevents[_SAPP_EVENTTYPE_NUM];
#endif
} _state;

void init_events(void) {
    memset(&_state.timers, 0, sizeof(table_t));
    memset(&_state.events, 0, sizeof(table_t));
#ifndef JEFF_NO_INPUT
    memset(&_state.sevents, 0, _SAPP_EVENTTYPE_NUM * sizeof(sevent*));
    memset(&_state.keymap, 0, sizeof(table_t));
    _state.keymap = table_new();
#endif
}

static void add_event(const char *name, bool once, jeff_event_callback_t cb, void *userdata) {
    listener_t listener = {
        .name = name,
        .cb = cb,
        .once = once,
        .userdata = userdata
    };
    listener_t *arr = NULL;
    if (table_has(&_state.events, name)) {
        table_get(&_state.events, name, (void**)&arr);
        garry_append(arr, listener);
    } else {
        garry_append(arr, listener);
        table_set(&_state.events, name, (void*)arr);
    }
}

void jeff_event_listen(const char *name, jeff_event_callback_t cb, void *userdata) {
    add_event(name, false, cb, userdata);
}

void jeff_event_listen_once(const char *name, jeff_event_callback_t cb, void *userdata) {
    add_event(name, false, cb, userdata);
}

static int _remove_match(const char *name, void *value, void *userdata) {
    table_t *_table = (table_t*)userdata;
    table_del(_table, name);
    garry_free(value);
    return 1;
}

void jeff_event_remove(const char *name) {
    table_find(&_state.events, name, _remove_match, NULL);
}

static int _fire_event(const char *name, void *value, void *userdata) {
    table_t *_table = (table_t*)userdata;
    listener_t *_listeners = (listener_t*)value;
    for (int i = garry_count(_listeners) - 1; i >= 0; i--) {
        _listeners[i].cb(_listeners[i].userdata);
        if (_listeners[i].once)
            garry_remove_at(_listeners, i);
    }
    table_del(_table, name);
    garry_free(value);
    return 1;
}

void jeff_event_emit(const char *name) {
    table_find(&_state.events, name, _fire_event, NULL);
}

int _free_event(table_pair_t *pair, void *userdata) {
    listener_t *arr = (listener_t*)pair->value;
    if (arr)
        garry_free(arr);
    return 1;
}

void jeff_events_clear(void) {
    table_each(&_state.events, _free_event, NULL);
    table_free(&_state.events);
    _state.events = table_new();
}

static void add_timer(const char *name, int64_t duration, bool loop, jeff_timer_callback_t cb, const char *event, void *userdata) {
    timer_t timer = {
        .name = name,
        .duration = duration,
        .start = stm_now(),
        .loop = loop,
        .cb = cb,
        .event = event
    };
    timer_t *arr = NULL;
    if (table_has(&_state.timers, name)) {
        table_get(&_state.timers, name, (void**)&arr);
        garry_append(arr, timer);
    } else {
        garry_append(arr, timer);
        table_set(&_state.timers, name, (void*)arr);
    }
}

void jeff_timer_every(const char *name, int64_t ms, jeff_timer_callback_t cb, void *userdata) {
    add_timer(name, ms, true, cb, NULL, userdata);
}

void jeff_timer_emit_every(const char *name, int64_t ms, const char *event, void *userdata) {
    add_timer(name, ms, true, NULL, event, userdata);
}

void jeff_timer_after(const char *name, int64_t ms, jeff_timer_callback_t cb, void *userdata) {
    add_timer(name, ms, false, cb, NULL, userdata);
}

void jeff_timer_emit_after(const char *name, int64_t ms, const char *event, void *userdata) {
    add_timer(name, ms, false, NULL, event, userdata);
}

void jeff_timer_remove(const char *name) {
    table_find(&_state.timers, name, _remove_match, NULL);
}

int _free_timer(table_pair_t *pair, void *userdata) {
    timer_t *arr = (timer_t*)pair->value;
    if (arr)
        garry_free(arr);
    return 1;
}

void jeff_timers_clear(void) {
    table_each(&_state.timers, _free_timer, NULL);
    table_free(&_state.timers);
    _state.events = table_new();
}

static int _check(table_pair_t *pair, void *userdata) {
    timer_t *_timers = (timer_t*)pair->value;
    for (int i = garry_count(_timers) - 1; i >= 0; i--) {
        timer_t *timer = &_timers[i];
        uint64_t diff = stm_ms(stm_diff(stm_now(), timer->start));
        if (diff >= timer->duration) {
            if (timer->cb)
                timer->cb(timer->userdata);
            else
                jeff_event_emit(timer->event);
            if (!timer->loop)
                garry_remove_at(_timers, i);
            else
                timer->start = stm_now();
        }
    }
    return 1;
}

void update_timers(void) {
    table_each(&_state.timers, _check, NULL);
}

#ifndef JEFF_NO_INPUT
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

static int __check(table_pair_t *pair, void *userdata) {
    keymap_t *keymap = (keymap_t*)pair->value;
    if (keymap && sapp_check_state(&keymap->input))
        jeff_event_emit(pair->key.string);
    return 1;
}

void check_keymaps(void) {
    table_each(&_state.keymap, __check, NULL);
}

void sapp_emit_on_event(sapp_event_type event_type, const char *event) {
    sevent e = {
        .event = strdup(event),
        .cb = NULL
    };
    garry_append(_state.sevents[event_type], e);
}

void sapp_on_event(sapp_event_type event_type, sapp_event_callback_t callback) {
    sevent e = {
        .event = NULL,
        .cb = callback
    };
    garry_append(_state.sevents[event_type], e);
}

int _free(table_pair_t *pair, void *userdata) {
    keymap_t *keymap = (keymap_t*)pair->value;
    if (keymap)
        garry_free(keymap);
    return 1;
}

void sapp_clear_events(void) {
    for (int i = 0; i < _SAPP_EVENTTYPE_NUM; i++)
        if (_state.sevents[i])
            garry_free(_state.sevents[i]);
    memset(&_state.events, 0, _SAPP_EVENTTYPE_NUM * sizeof(sevent*));
    table_each(&_state.keymap, _free, NULL);
    table_free(&_state.keymap);
    _state.keymap = table_new();
}

void check_event(sapp_event_type type) {
    if (!_state.sevents[type])
        return;
    for (int i = 0; i < garry_count(_state.sevents[type]); i++) {
        sevent *event = &_state.sevents[type][i];
        if (event->event != NULL)
            jeff_event_emit(event->event);
    }
}

void sapp_remove_event(sapp_event_type type) {
    if (_state.sevents[type])
        garry_free(_state.sevents[type]);
}
#endif // JEFF_NO_INPUT
