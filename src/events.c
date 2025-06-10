/* jeff/events.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2024  George Watson

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
#include "match.c"

typedef struct listener {
    const char *name;
    event_callback_t cb;
    bool once;
    void *userdata;
} listener_t;

typedef struct timer {
    const char *name;
    int64_t duration;
    int64_t start;
    bool loop;
    timer_callback_t cb;
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

static void add_event(const char *name, bool once, event_callback_t cb, void *userdata) {
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

void on_event(const char *name, event_callback_t cb, void *userdata) {
    add_event(name, false, cb, userdata);
}

void on_event_once(const char *name, event_callback_t cb, void *userdata) {
    add_event(name, false, cb, userdata);
}

void remove_event_named(const char *name) {
    if (table_has(&state.events, name)) {
        listener_t *arr = NULL;
        table_get(&state.events, name, (void**)&arr);
        garry_free(arr);
    }
}

void emit_event(const char *name) {
    if (!table_has(&state.events, name))
        return;
    listener_t *arr = NULL;
    table_get(&state.events, name, (void**)&arr);
    size_t name_len = strlen(name);
    for (int i = garry_count(arr) - 1; i >= 0; i--) {
        listener_t *l = &arr[i];
        if (!strncmp(name, l->name, name_len)) {
            l->cb(l->userdata);
            if (l->once)
                garry_remove_at(arr, i);
        }
    }
}

int _free_event(table_pair_t *pair, void *userdata) {
    listener_t *arr = (listener_t*)pair->value;
    if (arr)
        garry_free(arr);
    return 1;
}

void clear_all_events(void) {
    table_each(&state.events, _free_event, NULL);
    table_free(&state.events);
    state.events = table_new();
}

static void add_timer(const char *name, int64_t duration, bool loop, timer_callback_t cb, const char *event, void *userdata) {
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

void timer_every(const char *name, int64_t ms, timer_callback_t cb, void *userdata) {
    add_timer(name, ms, true, cb, NULL, userdata);
}

void timer_emit_every(const char *name, int64_t ms, const char *event, void *userdata) {
    add_timer(name, ms, true, NULL, event, userdata);
}

void timer_after(const char *name, int64_t ms, timer_callback_t cb, void *userdata) {
    add_timer(name, ms, false, cb, NULL, userdata);
}

void timer_emit_after(const char *name, int64_t ms, const char *event, void *userdata) {
    add_timer(name, ms, false, NULL, event, userdata);
}

void remove_timer_named(const char *name) {
    if (table_has(&state.timers, name)) {
        timer_t *arr = NULL;
        table_get(&state.timers, name, (void**)&arr);
        garry_free(arr);
    }
}

int _free_timer(table_pair_t *pair, void *userdata) {
    timer_t *arr = (timer_t*)pair->value;
    if (arr)
        garry_free(arr);
    return 1;
}

void clear_all_timers(void) {
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
                emit_event(timer->event);
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
