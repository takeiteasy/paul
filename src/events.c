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

// Copyright 2020 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
//
// match returns true if str matches pattern. This is a very
// simple wildcard match where '*' matches on any number characters
// and '?' matches on any one character.
//
// pattern:
//   { term }
// term:
//      '*'         matches any sequence of non-Separator characters
//      '?'         matches any single non-Separator character
//      c           matches character c (c != '*', '?')
//     '\\' c       matches character c
static bool match(const char *pat, long plen, const char *str, long slen)  {
    if (plen < 0) plen = strlen(pat);
    if (slen < 0) slen = strlen(str);
    while (plen > 0) {
        if (pat[0] == '\\') {
            if (plen == 1)
                return false;
            pat++;
            plen--;
        } else if (pat[0] == '*') {
            if (plen == 1)
                return true;
            if (pat[1] == '*') {
                pat++;
                plen--;
                continue;
            }
            if (match(pat+1, plen-1, str, slen))
                return true;
            if (slen == 0)
                return false;
            str++;
            slen--;
            continue;
        }
        if (slen == 0)
            return false;
        if (pat[0] != '?' && str[0] != pat[0])
            return false;
        pat++;
        plen--;
        str++;
        slen--;
    }
    return slen == 0 && plen == 0;
}

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

static timer_t *_timers = NULL;
static listener_t *_events = NULL;

static void add_event(const char *name, bool once, event_callback_t cb, void *userdata) {
    listener_t listener = {
        .name = name,
        .cb = cb,
        .once = once,
        .userdata = userdata
    };
    garry_append(_events, listener);
}

void on_event(const char *name, event_callback_t cb, void *userdata) {
    add_event(name, false, cb, userdata);
}

void on_event_once(const char *name, event_callback_t cb, void *userdata) {
    add_event(name, false, cb, userdata);
}

void remove_event_named(const char *name) {
    size_t name_length = name ? strlen(name) : 0;
    for (int i = garry_count(_events) - 1; i >= 0; i--)
        if (name) {
            if (match(name, name_length, _events[i].name, 0))
                garry_remove_at(_events, i);
        } else
            if (!_events[i].name)
                garry_remove_at(_events, i);
    if (!garry_count(_events)) {
        garry_free(_events);
        _events = NULL;
    }
}

void emit_event(const char *name) {
    for (int i = garry_count(_events) - 1; i >= 0; i--) {
        listener_t *l = &_events[i];
        l->cb(l->userdata);
        if (l->once)
            garry_remove_at(_events, i);
    }
}

void clear_all_events(void) {
    if (_events) {
        garry_free(_events);
        _events = NULL;
    }
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
    garry_append(_timers, timer);
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
    size_t name_length = name ? strlen(name) : 0;
    for (int i = garry_count(_timers) - 1; i >= 0; i--)
        if (name) {
            if (match(name, name_length, _timers[i].name, 0))
                garry_remove_at(_timers, i);
        } else
            if (!_timers[i].name)
                garry_remove_at(_timers, i);
    if (!garry_count(_timers)) {
        garry_free(_timers);
        _timers = NULL;
    }
}

void clear_all_timers(void) {
    if (_timers) {
        garry_free(_timers);
        _timers = NULL;
    }
}

void update_timers(void) {
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
    if (!garry_count(_timers)) {
        garry_free(_timers);
        _timers = NULL;
    }
}
