/* paul/paul_list.h -- https://github.com/takeiteasy/paul

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

#ifndef PAUL_LIST_HEADER
#define PAUL_LIST_HEADER
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define PAUL_NO_BLOCKS
#endif
#ifndef PAUL_NO_BLOCKS
#if !__has_extension(blocks)
#define PAUL_NO_BLOCKS
#else
#if __has_include(<Block.h>)
#include <Block.h>
#endif
#endif
#endif

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define __list_raw__(a)             ((int*)(a) - 2)
#define __list_m__(a)               __list_raw__(a)[0]
#define __list_n__(a)               __list_raw__(a)[1]

#define __list_needgrow__(a,n)      ((a)==0 || __list_n__(a)+n >= __list_m__(a))
#define __list_grow__(a,n)          __list_growf__((void **) &(a), (n), sizeof(*(a)))
#define __list_maybegrow__(a,n)     (__list_needgrow__(a,(n)) ? __list_grow__(a,n) : 0)

#define __list_needshrink__(a)      (__list_m__(a) > 4 && __list_n__(a) <= __list_m__(a) / 4)
#define __list_maybeshrink__(a)     (__list_needshrink__(a) ? __list_shrink__(a) : 0)
#define __list_shrink__(a)          __list_shrinkf__((void **) &(a), sizeof(*(a)))

#define list_free(a)                ((a) ? free(__list_raw__(a)),0 : 0)
#define list_append(a, v)           (__list_maybegrow__(a,1), (a)[__list_n__(a)++] = (v))
#define list_count(a)               ((a) ? __list_n__(a) : 0)
#define list_insert(a, idx, v)      (__list_maybegrow__(a,1), memmove(&a[idx+1], &a[idx], (__list_n__(a)++ - idx) * sizeof(*(a))), a[idx] = (v))
#define list_push(a, v)             (list_insert(a,0,v))
#define list_cdr(a)                 (void*)(list_count(a) > 1 ? &(a+1) : NULL)
#define list_car(a)                 (void*)((a) ? &(a)[0] : NULL)
#define list_last(a)                (void*)((a) ? &(a)[__list_n__(a)-1] : NULL)
#define list_pop(a)                 (--__list_n__(a), __list_maybeshrink__(a))
#define list_remove_at(a, idx)      (idx == __list_n__(a)-1 ? memmove(&a[idx], &arr[idx+1], (--__list_n__(a) - idx) * sizeof(*(a))) : list_pop(a), __list_maybeshrink__(a))
#define list_shift(a)               (list_remove_at(a, 0))
#define list_clear(a)               ((a) ? (__list_n__(a) = 0) : 0, __list_shrink__(a))

#define list_shuffle(a)                             \
    do {                                            \
        int i, j, n = list_count(a);                \
        for (i = n - 1; i > 0; i--) {               \
            j = rand() % (i + 1);                   \
            typeof(a[i]) tmp  = a[j];               \
            a[j] = a[i];                            \
            a[i] = tmp;                             \
        }                                           \
    } while(0)

#define list_reverse(a)                             \
    do {                                            \
        int len = list_count(a);                    \
        for (int i = 0; i < len; i++) {             \
            int j = (len - i) - 1;                  \
            if (i == j)                             \
                continue;                           \
            typeof(a[i]) tmp  = a[i];               \
            a[i] = a[j];                            \
            a[j] = tmp;                             \
        }                                           \
    } while(0)

#ifndef PAUL_NO_BLOCKS
#define list_shuffled(a)                            \
    ^(typeof(a) _a) {                               \
        typeof(a) result = NULL;                    \
        size_t n = list_count(a);                   \
        int *indices = NULL;                        \
        for (int i = 0; i < n; i++)                 \
            list_append(indices, i);                \
        list_shuffle(indices);                      \
        for (int i = 0; i < n; i++)                 \
            list_append(result, a[indices[i]]);     \
        list_free(indices);                         \
        return result;                              \
    }(a)

#define list_new(TYPE, ...)                         \
    ^(void) {                                       \
        TYPE *result = NULL;                        \
        list_append_va(result, __VA_ARGS__);        \
        return result;                              \
    }()

#define list_map(a, fn)                             \
    ^(typeof(a) _a) {                               \
        typeof(_a) result = NULL;                   \
        for (int i = 0; i < list_count(_a); i++)    \
            list_append(result, fn(_a[i]));         \
        return result;                              \
    }(a)

#define list_map_enumerate(a, fn)                   \
    ^(typeof(a) _a) {                               \
        typeof(_a) result = NULL;                   \
        for (int i = 0; i < list_count(_a); i++)    \
            list_append(result, fn(i, _a[i]));      \
        return result;                              \
    }(a)

#define list_reduce(a, initial, fn)                 \
    ^(typeof(a) _a) {                               \
        typeof(_a[0]) result = initial;             \
        for (int i = 0; i < list_count(_a); i++)    \
            fn(&result, _a[i]);                     \
        return result;                              \
    }(a)

#define list_reduce_enumerate(a, initial, fn)       \
    ^(typeof(a) _a) {                               \
        typeof(_a[0]) result = initial;             \
        for (int i = 0; i < list_count(_a); i++)    \
            fn(i, &result, _a[i]);                  \
        return result;                              \
    }(a)

#define list_filter(a, v)                           \
    ^(typeof(a) _a) {                               \
        typeof(a) result = NULL;                    \
        for (int i = 0; i < list_count(_a); i++)    \
            if (_a[i] == v)                         \
                list_append(result, _a[i]);         \
        return result;                              \
    }(a)

#define list_filter_fn(a, fn)                       \
    ^(typeof(a) _a) {                               \
        typeof(a) result = NULL;                    \
        for (int i = 0; i < list_count(_a); i++)    \
            if (fn(_a[i]))                          \
                list_append(result, _a[i]);         \
        return result;                              \
    }(a)

#define list_filter_indices(a, v)                   \
    ^(typeof(a) _a) {                               \
        int *result = NULL;                         \
        for (int i = 0; i < list_count(_a); i++)    \
            if (_a[i] == v)                         \
                list_append(result, i);             \
        return result;                              \
    }(a)

#define list_filter_indices_fn(a, fn)               \
    ^(typeof(a) _a) {                               \
        int *result = NULL;                         \
        for (int i = 0; i < list_count(_a); i++)    \
            if (fn(i, _a[i]))                       \
                list_append(result, i);             \
        return result;                              \
    }(a)

#define list_sample(a, n) \
    ^(typeof(a) _a) {                               \
        typeof(a) result = NULL;                    \
        int l = list_count(a);                      \
        if (n > l || !n || !l)                      \
            return result;                          \
        if (n == l)                                 \
            return a;                               \
        int x[n], j = 0;                            \
        for (int i = 0; i < n; i++)                 \
            x[i] = 0;                               \
        for (int i = 0; i < n; i++) {               \
            int y = rand() / (RAND_MAX / n + 1);    \
            int alreadyFound = 0;                   \
            for (int k = 0; k < j; k++)             \
                if (x[k] == y) {                    \
                    alreadyFound = 1;               \
                    break;                          \
                }                                   \
            if (alreadyFound)                       \
                i--;                                \
            else {                                  \
                list_append(result, _a[y]);         \
                x[j++] = y;                         \
            }                                       \
        }                                           \
        return result;                              \
    }(a)

#define list_slice(a, start, end)                   \
    ^(typeof(a) _a, int _min, int _max) {           \
        typeof(a) result = NULL;                    \
        if (_min < 0)                               \
            _min = 0;                               \
        int l = list_count(_a);                     \
        if (_max > l)                               \
            _max = l;                               \
        if (_min >= _max)                           \
            return result;                          \
        for (int i = _min; i < _max; i++)           \
            list_append(result, _a[i]);             \
        return result;                              \
    }(a, start, end)

#define list_unique(a)                              \
    ^(typeof(a) _a) {                               \
        typeof(a) result = NULL;                    \
        int i, j;                                   \
        for (i = 0; i < list_count(_a); i++) {      \
            for (j = 0; j < i; j++)                 \
                if (_a[i] == _a[j])                 \
                    break;                          \
            if (i == j)                             \
                list_append(result, _a[i]);         \
        }                                           \
        return result;                              \
    }(a)

#define list_unique_fn(a, fn)                       \
    ^(typeof(a) _a) {                               \
        typeof(a) result = NULL;                    \
        int i, j;                                   \
        for (i = 0; i < list_count(_a); i++) {      \
            for (j = 0; j < i; j++)                 \
                if (fn(_a[i], _a[j]))               \
                    break;                          \
            if (i == j)                             \
                list_append(result, _a[i]);         \
        }                                           \
        return result;                              \
    }(a)

#define list_clone(a)                               \
    ^(typeof(a) _a) {                               \
        typeof(_a) result = NULL;                   \
        for (int i = 0; i < list_count(_a); i++)    \
            list_append(result, _a[i]);             \
        return result;                              \
    }(a)

#define list_reversed(a)                            \
    ^(typeof(a) _a) {                               \
        typeof(_a) result = list_clone(_a);         \
        list_reverse(result);                       \
        return result;                              \
    }(a)
#endif // PAUL_NO_BLOCKS

#ifndef always_inline
#if defined(__clang__) || defined(__GNUC__)
#define always_inline __attribute__((always_inline))
#else
#define always_inline
#endif
#endif

always_inline static inline void __list_growf(void **arr, int increment, int itemsize) {
    int m = *arr ? 2 * __list_m__(*arr) + increment : increment + 1;
    void *p = (void*)realloc(*arr ? __list_raw__(*arr) : 0, itemsize * m + sizeof(int) * 2);
    assert(p);
    if (p) {
        if (!*arr)
            ((int *)p)[1] = 0;
        *arr = (void*)((int*)p + 2);
        __list_m__(*arr) = m;
    }
}

always_inline static inline void __list_shrinkf(void **arr, int itemsize) {
    int m = *arr ? __list_m__(*arr) / 2 : 0;
    void *p = (void*)realloc(*arr ? __list_raw__(*arr) : 0, itemsize * m + sizeof(int) * 2);
    assert(p);
    if (p) {
        *arr = (void*)((int*)p + 2);
        __list_m__(*arr) = m;
    }
}

#ifdef __cplusplus
}
#endif
#endif // PAUL_LIST_HEADER
