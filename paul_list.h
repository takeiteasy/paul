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

/*!
 @header paul_list.h
 @copyright George Watson GPLv3
 @updated 2025-09-29
 @brief Dynamic array (stretchy-buffer) macros for C/C++.
 @discussion
    Implementation is included when PAUL_LIST_IMPLEMENTATION or PAUL_IMPLEMENTATION is defined.
*/

#ifndef PAUL_LIST_HEADER
#define PAUL_LIST_HEADER
#ifdef __cplusplus
extern "C" {
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

/*!
 @define list_free
 @brief Free the underlying buffer used by a dynamic list created with these macros.
 @param a The list pointer previously used with the list macros. May be NULL.
 @discussion If 'a' is non-NULL this macro frees the internal allocation backing the list. After calling this macro the caller must not use 'a' unless it is reinitialized. The macro evaluates to 0.
*/
#define list_free(a)                ((a) ? free(__list_raw__(a)),0 : 0)

/*!
 @define list_append
 @brief Append a value to the end of the list.
 @param a The list variable (pointer-to-element-type). May be NULL; it will be grown as needed.
 @param v The value to append (assigned by value into the list element slot).
 @discussion This macro may reallocate the list to accommodate the new element. After growing, it stores the value at the next index and increments the list count.
*/
#define list_append(a, v)           (__list_maybegrow__(a,1), (a)[__list_n__(a)++] = (v))

/*!
 @define list_count
 @brief Return the number of elements currently stored in the list.
 @param a The list variable (pointer-to-element-type). May be NULL.
 @result The number of elements in the list (0 if 'a' is NULL).
*/
#define list_count(a)               ((a) ? __list_n__(a) : 0)

/*!
 @define list_insert
 @brief Insert a value at the specified index, shifting subsequent elements.
 @param a The list variable (pointer-to-element-type). May be NULL; it will be grown as needed.
 @param idx Zero-based index at which to insert the value. Must be <= current count.
 @param v The value to insert.
 @discussion This macro grows the list if necessary, shifts elements starting at 'idx' one slot to the right, stores 'v' at 'idx', and increments the stored element count.
*/
#define list_insert(a, idx, v)      (__list_maybegrow__(a,1), memmove(&a[idx+1], &a[idx], (__list_n__(a)++ - idx) * sizeof(*(a))), a[idx] = (v))

/*!
 @define list_push
 @brief Push a value to the front of the list (insert at index 0).
 @param a The list variable (pointer-to-element-type).
 @param v The value to push.
*/
#define list_push(a, v)             (list_insert(a,0,v))

/*!
 @define list_cdr
 @brief Return a pointer to the list starting at the second element (the "cdr").
 @param a The list variable (pointer-to-element-type).
 @result Pointer to the second element of the list, or NULL if the list has fewer than two elements.
*/
#define list_cdr(a)                 (void*)(list_count(a) > 1 ? &(a+1) : NULL)

/*!
 @define list_car
 @brief Return a pointer to the first element of the list (the "car").
 @param a The list variable (pointer-to-element-type).
 @result Pointer to the first element, or NULL if the list is NULL.
*/
#define list_car(a)                 (void*)((a) ? &(a)[0] : NULL)

/*!
 @define list_last
 @brief Return a pointer to the last element in the list.
 @param a The list variable (pointer-to-element-type).
 @result Pointer to the last element, or NULL if the list is NULL or empty.
*/
#define list_last(a)                (void*)((a) ? &(a)[__list_n__(a)-1] : NULL)

/*!
 @define list_pop
 @brief Remove the last element from the list (decrement count) and possibly shrink the backing storage.
 @param a The list variable (pointer-to-element-type).
 @discussion This macro only adjusts the stored element count; it does not return the popped value. It may shrink the backing allocation when the list becomes sparse.
*/
#define list_pop(a)                 (--__list_n__(a), __list_maybeshrink__(a))

/*!
 @define list_remove_at
 @brief Remove the element at the given index, shifting subsequent elements left.
 @param a The list variable (pointer-to-element-type).
 @param idx The zero-based index of the element to remove.
 @discussion The macro shifts elements after 'idx' down and adjusts the stored count. It will also attempt to shrink the backing allocation when appropriate.
*/
#define list_remove_at(a, idx)      (idx == __list_n__(a)-1 ? memmove(&a[idx], &arr[idx+1], (--__list_n__(a) - idx) * sizeof(*(a))) : list_pop(a), __list_maybeshrink__(a))

/*!
 @define list_shift
 @brief Remove the first element of the list (shift).
 @param a The list variable (pointer-to-element-type).
*/
#define list_shift(a)               (list_remove_at(a, 0))

/*!
 @define list_clear
 @brief Clear all elements from the list and shrink its backing allocation.
 @param a The list variable (pointer-to-element-type).
*/
#define list_clear(a)               ((a) ? (__list_n__(a) = 0) : 0, __list_shrink__(a))

/*!
 @define list_shuffle
 @brief Randomly shuffle the elements of the list in-place using rand().
 @param a The list variable (pointer-to-element-type).
 @discussion Uses the Fisher–Yates shuffle algorithm. The caller is responsible for seeding the PRNG (e.g. srand()) if reproducible behaviour is required.
*/
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

/*!
 @define list_reverse
 @brief Reverse the order of elements in the list in-place.
 @param a The list variable (pointer-to-element-type).
*/
#define list_reverse(a)                             \
    do {                                            \
        int len = list_count(a);                    \
        for (int i = 0; i < len / 2; i++) {         \
            int j = (len - i) - 1;                  \
            typeof(a[i]) tmp  = a[i];               \
            a[i] = a[j];                            \
            a[j] = tmp;                             \
        }                                           \
    } while(0)

#ifndef always_inline
#if defined(__clang__) || defined(__GNUC__)
#define always_inline __attribute__((always_inline))
#else
#define always_inline
#endif
#endif

always_inline static inline void __list_growf__(void **arr, int increment, int itemsize) {
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

always_inline static inline void __list_shrinkf__(void **arr, int itemsize) {
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
