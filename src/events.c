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
#include "garry.h"
#include "table.h"

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
} state;

void init_events(void) {
    memset(&state.timers, 0, sizeof(table_t));
    memset(&state.events, 0, sizeof(table_t));
}

static void add_event(const char *name, bool once, jeff_event_callback_t cb, void *userdata) {
    listener_t listener = {
        .name = name,
        .cb = cb,
        .once = once,
        .userdata = userdata
    };
    listener_t *arr = NULL;
    if (table_has(&state.events, name)) {
        table_get(&state.events, name, (void**)&arr);
        garry_append(arr, listener);
    } else {
        garry_append(arr, listener);
        table_set(&state.events, name, (void*)arr);
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
    table_find(&state.events, name, _remove_match, NULL);
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
    table_find(&state.events, name, _fire_event, NULL);
}

int _free_event(table_pair_t *pair, void *userdata) {
    listener_t *arr = (listener_t*)pair->value;
    if (arr)
        garry_free(arr);
    return 1;
}

void jeff_events_clear(void) {
    table_each(&state.events, _free_event, NULL);
    table_free(&state.events);
    state.events = table_new();
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
    if (table_has(&state.timers, name)) {
        table_get(&state.timers, name, (void**)&arr);
        garry_append(arr, timer);
    } else {
        garry_append(arr, timer);
        table_set(&state.timers, name, (void*)arr);
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
    table_find(&state.timers, name, _remove_match, NULL);
}

int _free_timer(table_pair_t *pair, void *userdata) {
    timer_t *arr = (timer_t*)pair->value;
    if (arr)
        garry_free(arr);
    return 1;
}

void jeff_timers_clear(void) {
    table_each(&state.timers, _free_timer, NULL);
    table_free(&state.timers);
    state.events = table_new();
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
    table_each(&state.timers, _check, NULL);
}
