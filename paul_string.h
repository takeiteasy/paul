/* paul/paul_string.h -- https://github.com/takeiteasy/paul

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
 @header paul_string.h
 @copyright George Watson GPLv3
 @updated 2025-07-19
 @brief Unified ascii+wide string type for C/C++.
 @discussion
    Implementation is included when PAUL_STRING_IMPLEMENTATION or PAUL_IMPLEMENTATION is defined.
*/

#ifndef PAUL_STRING_H
#define PAUL_STRING_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdbool.h>

#if defined(__cpluscplus) || __STDC_VERSION__ < 201112L || !__has_extension(c_generic_selections)
#define PAUL_NO_GENERICS
#endif

#ifndef PAUL_NO_GENERICS
/*!
 @define str_from
 @brief Convenience generic constructor that selects `str_from_cstr` or `str_from_wcstr`.
 @discussion
     When the compiler supports C11 `_Generic` selections this macro dispatches
     at compile-time based on the type of the argument. It expands to
     `str_from_cstr(const char*)` for narrow C strings and to
     `str_from_wcstr(const wchar_t*)` for wide strings. If `_Generic` is not
     available then `PAUL_NO_GENERICS` is defined and these macros are not
     provided.
*/
#define str_from(S)                  \
_Generic((S),                        \
    const char *: str_from_cstr,     \
    const wchar_t *: str_from_wcstr, \
    char *: str_from_cstr,           \
    wchar_t *: str_from_wcstr,       \
    default: NULL)(S)
/*!
 @define str
 @brief Shortcut wrapper that calls `str_from` and constructs a `str_t` from a literal or pointer.
 @discussion
     See `str_from` for details. This macro is handy when passing string
     literals directly (it will pick the correct constructor based on the
     literal's type).
*/
#define str(S)                          \
_Generic((S),                           \
    const char *: str_from_cstr(S),     \
    const wchar_t *: str_from_wcstr(S), \
    char *: str_from_cstr(S),           \
    wchar_t *: str_from_wcstr(S),       \
    default: NULL)(S)
/*!
 @define str_wildcard
 @brief Generic wildcard matcher dispatcher.
 @discussion
     When `_Generic` is available this macro selects `str_wildcard_ascii` or
     `str_wildcard_wide` based on the type of the first argument. If
     `_Generic` is not available the macro is omitted and callers must call
     the correct helper directly.
*/
#define str_wildcard(S, P)              \
_Generic((S),                           \
    const char *: str_wildcard_ascii,   \
    char *: str_wildcard_ascii,         \
    const wchar_t *: str_wildcard_wide, \
    wchar_t *: str_wildcard_wide,       \
    default: NULL)(S, P)
#endif

/*!
 @typedef str_t
 @brief Opaque string handle used by the paul string API.
 @discussion
    A `str_t` points to an internal buffer allocated by the paul library
    that stores either an ASCII (char) or wide (wchar_t) string. The
    implementation stores a small header placed immediately before the
    returned data pointer to track the stored type and the character length.
    Callers must treat `str_t` as an opaque handle and use the provided
    API functions to manipulate strings.
*/
typedef void* str_t;

/*!
 @function str_dup
 @brief Duplicate a string handle.
 @param s A `str_t` previously returned by the library.
 @return A new `str_t` owning a copy of the string, or NULL on allocation failure.
*/
str_t str_dup(const str_t s);

/*!
 @function str_from_cstr
 @brief Create a `str_t` from a NUL-terminated C string (ASCII).
 @discussion This function is the target of the `str_from` / `str` helper
     macros when called with a `const char *` (see the macro documentation
     above). Use the macros when convenient; you may call this function
     directly when generics are not available.
 @param cstr NUL-terminated ASCII string.
 @return A new `str_t` representing the same contents.
*/
str_t str_from_cstr(const char* cstr);

/*!
 @function str_from_wcstr
 @brief Create a `str_t` from a NUL-terminated wide string (wchar_t).
 @discussion This function is the wide-string variant targeted by the
     `str_from` / `str` helper macros when called with `const wchar_t *`.
 @param cstr NUL-terminated wide string.
 @return A new `str_t` representing the same contents as wide characters.
*/
str_t str_from_wcstr(const wchar_t* cstr);

/*!
 @function str_make_ascii
 @brief Convert a `str_t` to an ASCII representation.
 @discussion If the string is already ASCII this returns a duplicate. If the
    source contains non-ASCII characters the function will return NULL.
 @param s The source `str_t`.
 @return A new ASCII `str_t` or NULL on failure.
*/
str_t str_make_ascii(str_t s);

/*!
 @function str_make_utf16
 @brief Convert a `str_t` to a wide (wchar_t) representation.
 @discussion If the string is already wide this returns a duplicate.
 @param s The source `str_t`.
 @return A new wide `str_t` or NULL on failure.
*/
str_t str_make_utf16(str_t s);

/*!
 @function str_raw_cstr
 @brief Return a raw NUL-terminated C string pointer for ASCII `str_t`.
 @param s The `str_t` handle.
 @return Pointer to NUL-terminated char data when ASCII, otherwise NULL.
*/
const char* str_raw_cstr(const str_t s);

/*!
 @function str_raw_wcstr
 @brief Return a raw NUL-terminated wide string pointer for wide `str_t`.
 @param s The `str_t` handle.
 @return Pointer to NUL-terminated wchar_t data when wide, otherwise NULL.
*/
const wchar_t* str_raw_wcstr(const str_t s);

/*!
 @function str_is_ascii
 @brief Query whether the `str_t` stores ASCII (narrow) characters.
 @param s The `str_t` handle.
 @return true when ASCII, false otherwise.
*/
bool str_is_ascii(const str_t s);

/*!
 @function str_is_utf16
 @brief Query whether the `str_t` stores wide (wchar_t) characters.
 @param s The `str_t` handle.
 @return true when wide, false otherwise.
*/
bool str_is_utf16(const str_t s);

/* Mutating operations */
/*!
 @function str_resize
 @brief Resize a `str_t` in-place to new length (truncates or extends with NUL).
 @param s Pointer to the `str_t` handle to resize.
 @param new_len New length in characters (not including NUL).
*/
void str_resize(str_t* s, size_t new_len);

/*!
 @function str_copy
 @brief Copy the contents of src into dest. Both must be the same underlying type.
 @param dest Destination `str_t` data pointer.
 @param src Source `str_t`.
*/
void str_copy(str_t dest, const str_t src);

/*!
 @function str_concat
 @brief Concatenate src to the end of *dest (dest is resized as needed).
 @param dest Pointer to destination `str_t` handle.
 @param src Source `str_t` to append.
*/
void str_concat(str_t* dest, const str_t src);

/*!
 @function str_append_char
 @brief Append a single ASCII character to an ASCII `str_t`.
 @param s Pointer to an ASCII `str_t`.
 @param c Character to append.
*/
void str_append_char(str_t* s, char c);

/*!
 @function str_insert
 @brief Insert substr into *s at position pos.
 @param s Pointer to destination `str_t` handle.
 @param pos Insertion index (must be <= length).
 @param substr Substring to insert.
*/
void str_insert(str_t* s, size_t pos, const str_t substr);

/*!
 @function str_erase
 @brief Erase len characters from *s starting at pos.
 @param s Pointer to `str_t`.
 @param pos Start index.
 @param len Number of characters to remove.
*/
void str_erase(str_t* s, size_t pos, size_t len);

/*!
 @function str_replace
 @brief Replace the first occurrence of old_sub in *s with new_sub.
 @param s Pointer to `str_t` to modify.
 @param old_sub Substring to replace.
 @param new_sub Replacement substring.
*/
void str_replace(str_t* s, const str_t old_sub, const str_t new_sub);

/*!
 @function str_trim
 @brief Trim leading and trailing whitespace (ASCII only) from *s.
 @param s Pointer to ASCII `str_t`.
*/
void str_trim(str_t* s);

/*!
 @function str_to_upper
 @brief Convert the string in-place to upper-case.
 @discussion Works for ASCII and wide strings.
 @param s Pointer to `str_t`.
*/
void str_to_upper(str_t* s);

/*!
 @function str_to_lower
 @brief Convert the string in-place to lower-case.
 @discussion Works for ASCII and wide strings.
 @param s Pointer to `str_t`.
*/
void str_to_lower(str_t* s);

/* Comparison / search helpers */
/*!
 @function str_cmp
 @brief Compare two `str_t` values. Returns <0, 0, >0 similar to strcmp/wcscmp.
 @param a First string.
 @param b Second string.
*/
int str_cmp(const str_t a, const str_t b);

/*!
 @function str_equal
 @brief Equality test between two `str_t` values.
 @param a First string.
 @param b Second string.
 @return true when equal, false otherwise.
*/
bool str_equal(const str_t a, const str_t b);

/*!
 @function str_find
 @brief Find the first occurrence of substr in s.
 @param s The string to search.
 @param substr The substring to find.
 @return Index of first match or (size_t)-1 if not found.
*/
size_t str_find(const str_t s, const str_t substr);

/*!
 @function str_find_char
 @brief Find the first occurrence of ASCII character c in s (ASCII only).
 @param s The ASCII string to search.
 @param c Character to find.
 @return Index or (size_t)-1 if not found.
*/
size_t str_find_char(const str_t s, char c);

/*!
 @function str_starts_with
 @brief Return true when s starts with prefix.
 @param s String to test.
 @param prefix Prefix string.
*/
bool str_starts_with(const str_t s, const str_t prefix);

/*!
 @function str_ends_with
 @brief Return true when s ends with suffix.
 @param s String to test.
 @param suffix Suffix string.
*/
bool str_ends_with(const str_t s, const str_t suffix);

/*!
 @function str_wildcard_ascii
 @brief Match ASCII `str_t` against a glob-style pattern using '*', '?', and
     character classes like [a-z] or [^aeiou].
 @discussion This is the ASCII helper invoked by the `str_wildcard` macro
     when `_Generic` is available and the argument is an ASCII `const char *`.
 @param s The ASCII `str_t` to test.
 @param pattern NUL-terminated pattern using ASCII characters.
 @return true on match, false otherwise.
*/
bool str_wildcard_ascii(const str_t s, const char *pattern);

/*!
 @function str_wildcard_wide
 @brief Match wide `str_t` against a wide glob-style pattern (wchar_t pattern).
 @discussion This is the wide helper invoked by the `str_wildcard` macro when
     `_Generic` is available and the argument is a `const wchar_t *`.
 @param s The wide `str_t` to test.
 @param pattern NUL-terminated wide pattern using `wchar_t` characters.
 @return true on match, false otherwise.
*/
bool str_wildcard_wide(const str_t s, const wchar_t *pattern);

/*!
 @function str_length
 @brief Get the length of the string in characters.
 @param s The str_t handle.
 @return The number of characters (not including NUL).
*/
size_t str_length(const str_t s);

/*!
 @function str_char_at
 @brief Get the ASCII character at the specified index.
 @param s The ASCII str_t handle.
 @param index The index (0-based).
 @return The character, or 0 if out of bounds or not ASCII.
*/
char str_char_at(const str_t s, size_t index);

/*!
 @function str_wchar_at
 @brief Get the wide character at the specified index.
 @param s The wide str_t handle.
 @param index The index (0-based).
 @return The wide character, or 0 if out of bounds or not wide.
*/
wchar_t str_wchar_at(const str_t s, size_t index);

#ifdef __cplusplus
}
#endif
#endif // PAUL_STRING_H

#if defined(PAUL_STRING_IMPLEMENTATION) || defined(PAUL_IMPLEMENTATION)
typedef struct str_header {
    uint32_t type;
    uint32_t length;
    char data[];
} str_header_t;

typedef struct wstr_header {
    uint32_t type;
    uint32_t length;
    wchar_t data[];
} wstr_header_t;

/* Helper macros to access string header fields */
#define STR_TYPE(s) (*(uint32_t *)((char *)(s) - 8))
#define STR_LENGTH(s) (*(uint32_t *)((char *)(s) - 4))
#define STR_HEADER_SIZE(type) ((type) == 0 ? sizeof(str_header_t) : sizeof(wstr_header_t))
#define STR_ELEM_SIZE(type) ((type) == 0 ? sizeof(char) : sizeof(wchar_t))

static str_t _make_ascii(const char *s, size_t len) {
    struct str_header *h = (struct str_header *)malloc(sizeof(struct str_header) + (len + 1) * sizeof(char));
    h->type = 0;
    h->length = (uint32_t)len;
    memcpy(h->data, s, len);
    h->data[len] = '\0';
    return (str_t)h->data;
}

static str_t _make_utf16(const wchar_t *s, size_t len) {
    struct wstr_header *h = (struct wstr_header *)malloc(sizeof(struct wstr_header) + (len + 1) * sizeof(wchar_t));
    h->type = 1;
    h->length = (uint32_t)len;
    memcpy(h->data, s, len * sizeof(wchar_t));
    h->data[len] = L'\0';
    return (str_t)h->data;
}

static str_t _str_dup(const str_t *s, size_t header_size, size_t elem_size) {
    void *p = *s;
    void *header_start = (char *)p - header_size;
    uint32_t len = STR_LENGTH(p);
    size_t alloc_sz = header_size + (len + 1) * elem_size;
    void *new_p = malloc(alloc_sz);
    memcpy(new_p, header_start, alloc_sz);
    return (str_t)((char *)new_p + header_size);
}

str_t str_dup(const str_t s) {
    uint32_t type = STR_TYPE(s);
    return type == 0 ? _str_dup(&s, sizeof(str_header_t), sizeof(char)) : _str_dup(&s, sizeof(wstr_header_t), sizeof(wchar_t));
}

str_t str_from_cstr(const char *cstr) {
    return _make_ascii(cstr, strlen(cstr));
}

str_t str_from_wcstr(const wchar_t *cstr) {
    return _make_utf16(cstr, wcslen(cstr));
}

str_t str_make_ascii(str_t s) {
    uint32_t type = STR_TYPE(s);
    uint32_t len = STR_LENGTH(s);
    if (type == 0)
        return str_dup(s);
    else
    {
        // Convert wide to narrow, assume ASCII
        const wchar_t *ws = (const wchar_t *)s;
        char *narrow = (char *)malloc(len + 1);
        for (size_t i = 0; i < len; i++) {
            if (ws[i] > 127) {
                free(narrow);
                return NULL; // Cannot convert
            }
            narrow[i] = (char)ws[i];
        }
        narrow[len] = '\0';
        str_t result = _make_ascii(narrow, len);
        free(narrow);
        return result;
    }
}

str_t str_make_utf16(str_t s) {
    uint32_t type = STR_TYPE(s);
    uint32_t len = STR_LENGTH(s);
    if (type == 1)
        return str_dup(s);
    else
    {
        // Convert narrow to wide
        const char *ns = (const char *)s;
        wchar_t *wide = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
        for (size_t i = 0; i < len; i++)
            wide[i] = (wchar_t)ns[i];
        wide[len] = L'\0';
        str_t result = _make_utf16(wide, len);
        free(wide);
        return result;
    }
}

const char *str_raw_cstr(const str_t s) {
    uint32_t type = STR_TYPE(s);
    return type == 0 ? (const char *)s : NULL;
}

const wchar_t *str_raw_wcstr(const str_t s) {
    uint32_t type = STR_TYPE(s);
    return type == 1 ? (const wchar_t *)s : NULL;
}

bool str_is_ascii(const str_t s) {
    uint32_t type = STR_TYPE(s);
    return type == 0;
}

bool str_is_utf16(const str_t s) {
    uint32_t type = STR_TYPE(s);
    return type == 1;
}

void str_resize(str_t *s, size_t new_len) {
    uint32_t type = STR_TYPE(*s);
    uint32_t old_len = STR_LENGTH(*s);
    size_t header_size = STR_HEADER_SIZE(type);
    size_t elem_size = STR_ELEM_SIZE(type);
    size_t new_alloc_sz = header_size + (new_len + 1) * elem_size;
    void *new_p = realloc((char *)*s - header_size, new_alloc_sz);
    void *new_data = (char *)new_p + header_size;
    STR_LENGTH(new_data) = (uint32_t)new_len;
    if (type == 0)
        ((char *)new_data)[new_len] = '\0';
    else
        ((wchar_t *)new_data)[new_len] = L'\0';
    *s = (str_t)new_data;
}

void str_copy(str_t dest, const str_t src) {
    uint32_t type_dest = STR_TYPE(dest);
    uint32_t type_src = STR_TYPE(src);
    if (type_dest != type_src)
        return; // or handle conversion
    uint32_t len_src = STR_LENGTH(src);
    size_t elem_size = STR_ELEM_SIZE(type_dest);
    memcpy((void *)dest, (void *)src, len_src * elem_size);
    if (type_dest == 0)
        ((char *)dest)[len_src] = '\0';
    else
        ((wchar_t *)dest)[len_src] = L'\0';
    STR_LENGTH(dest) = len_src;
}

void str_concat(str_t *dest, const str_t src) {
    uint32_t type_dest = STR_TYPE(*dest);
    uint32_t type_src = STR_TYPE(src);
    if (type_dest != type_src)
        return;
    uint32_t len_dest = STR_LENGTH(*dest);
    uint32_t len_src = STR_LENGTH(src);
    size_t new_len = len_dest + len_src;
    str_resize(dest, new_len);
    size_t elem_size = STR_ELEM_SIZE(type_dest);
    memcpy((char *)*dest + len_dest * elem_size, (void *)src, len_src * elem_size);
}

void str_append_char(str_t *s, char c) {
    uint32_t type = STR_TYPE(*s);
    if (type != 0)
        return; // only for ASCII
    uint32_t len = STR_LENGTH(*s);
    str_resize(s, len + 1);
    ((char *)*s)[len] = c;
}

void str_insert(str_t *s, size_t pos, const str_t substr) {
    uint32_t type_s = STR_TYPE(*s);
    uint32_t type_sub = STR_TYPE(substr);
    if (type_s != type_sub)
        return;
    uint32_t len_s = STR_LENGTH(*s);
    uint32_t len_sub = STR_LENGTH(substr);
    if (pos > len_s)
        return;
    size_t new_len = len_s + len_sub;
    str_resize(s, new_len);
    size_t elem_size = STR_ELEM_SIZE(type_s);
    memmove((char *)*s + (pos + len_sub) * elem_size, (char *)*s + pos * elem_size, (len_s - pos) * elem_size);
    memcpy((char *)*s + pos * elem_size, (void *)substr, len_sub * elem_size);
}

void str_erase(str_t *s, size_t pos, size_t len) {
    uint32_t type = STR_TYPE(*s);
    uint32_t len_s = STR_LENGTH(*s);
    if (pos >= len_s || len == 0)
        return;
    if (pos + len > len_s)
        len = len_s - pos;
    size_t elem_size = STR_ELEM_SIZE(type);
    memmove((char *)*s + pos * elem_size, (char *)*s + (pos + len) * elem_size, (len_s - pos - len) * elem_size);
    str_resize(s, len_s - len);
}

void str_replace(str_t *s, const str_t old_sub, const str_t new_sub) {
    // Simplified: find and replace first occurrence
    size_t pos = str_find(*s, old_sub);
    if (pos != (size_t)-1) {
        uint32_t len_old = STR_LENGTH(old_sub);
        str_erase(s, pos, len_old);
        str_insert(s, pos, new_sub);
    }
}

void str_trim(str_t *s) {
    uint32_t type = STR_TYPE(*s);
    if (type != 0)
        return; // only for ASCII
    char *data = (char *)*s;
    uint32_t len = STR_LENGTH(*s);
    size_t start = 0, end = len;
    while (start < end && (data[start] == ' ' || data[start] == '\t' || data[start] == '\n'))
        start++;
    while (end > start && (data[end - 1] == ' ' || data[end - 1] == '\t' || data[end - 1] == '\n'))
        end--;
    if (start > 0 || end < len) {
        memmove(data, data + start, end - start);
        str_resize(s, end - start);
    }
}

void str_to_upper(str_t *s) {
    uint32_t type = STR_TYPE(*s);
    uint32_t len = STR_LENGTH(*s);
    if (type == 0)
        for (size_t i = 0; i < len; i++)
            ((char *)*s)[i] = toupper(((char *)*s)[i]);
    else
        for (size_t i = 0; i < len; i++)
            ((wchar_t *)*s)[i] = towupper(((wchar_t *)*s)[i]);
}

void str_to_lower(str_t *s) {
    uint32_t type = STR_TYPE(*s);
    uint32_t len = STR_LENGTH(*s);
    if (type == 0)
        for (size_t i = 0; i < len; i++)
            ((char *)*s)[i] = tolower(((char *)*s)[i]);
    else
        for (size_t i = 0; i < len; i++)
            ((wchar_t *)*s)[i] = towlower(((wchar_t *)*s)[i]);
}

int str_cmp(const str_t a, const str_t b) {
    uint32_t type_a = STR_TYPE(a);
    uint32_t type_b = STR_TYPE(b);
    if (type_a != type_b)
        return type_a < type_b ? -1 : 1;
    uint32_t len_a = STR_LENGTH(a);
    uint32_t len_b = STR_LENGTH(b);
    if (type_a == 0)
        return strncmp((const char *)a, (const char *)b, len_a < len_b ? len_a : len_b) ?: (len_a - len_b);
    else
        return wcsncmp((const wchar_t *)a, (const wchar_t *)b, len_a < len_b ? len_a : len_b) ?: (len_a - len_b);
}

bool str_equal(const str_t a, const str_t b) {
    uint32_t type_a = STR_TYPE(a);
    uint32_t type_b = STR_TYPE(b);
    if (type_a != type_b)
        return false;
    uint32_t len_a = STR_LENGTH(a);
    uint32_t len_b = STR_LENGTH(b);
    if (len_a != len_b)
        return false;
    size_t elem_size = STR_ELEM_SIZE(type_a);
    return memcmp((void *)a, (void *)b, len_a * elem_size) == 0;
}

size_t str_find(const str_t s, const str_t substr) {
    uint32_t type_s = STR_TYPE(s);
    uint32_t type_sub = STR_TYPE(substr);
    if (type_s != type_sub)
        return (size_t)-1;
    uint32_t len_s = STR_LENGTH(s);
    uint32_t len_sub = STR_LENGTH(substr);
    if (len_sub == 0)
        return 0;
    if (len_sub > len_s)
        return (size_t)-1;
    size_t elem_size = STR_ELEM_SIZE(type_s);
    const void *data_s = (const void *)s;
    const void *data_sub = (const void *)substr;
    for (size_t i = 0; i <= len_s - len_sub; i++) {
        const void *p1 = (const char *)data_s + i * elem_size;
        if (memcmp(p1, data_sub, len_sub * elem_size) == 0)
            return i;
    }
    return (size_t)-1;
}

size_t str_find_char(const str_t s, char c) {
    uint32_t type = STR_TYPE(s);
    if (type != 0)
        return (size_t)-1; // only for ASCII
    uint32_t len = STR_LENGTH(s);
    for (size_t i = 0; i < len; i++)
        if (((const char *)s)[i] == c)
            return i;
    return (size_t)-1;
}

bool str_starts_with(const str_t s, const str_t prefix) {
    uint32_t type_s = STR_TYPE(s);
    uint32_t type_p = STR_TYPE(prefix);
    if (type_s != type_p)
        return false;
    uint32_t len_p = STR_LENGTH(prefix);
    uint32_t len_s = STR_LENGTH(s);
    if (len_p > len_s)
        return false;
    size_t elem_size = STR_ELEM_SIZE(type_s);
    return memcmp((void *)s, (void *)prefix, len_p * elem_size) == 0;
}

bool str_ends_with(const str_t s, const str_t suffix) {
    uint32_t type_s = STR_TYPE(s);
    uint32_t type_suf = STR_TYPE(suffix);
    if (type_s != type_suf)
        return false;
    uint32_t len_suf = STR_LENGTH(suffix);
    uint32_t len_s = STR_LENGTH(s);
    if (len_suf > len_s)
        return false;
    size_t elem_size = STR_ELEM_SIZE(type_s);
    return memcmp((char *)s + (len_s - len_suf) * elem_size, (void *)suffix, len_suf * elem_size) == 0;
}

/* Generic wildcard implementation generated via macro for char/wchar_t.
 * Supports '*', '?', and character classes like [abc], [a-z], and negation [^...].
 */
#define WILDCARD_IMPL(fname, CHAR)                                                    \
    bool fname(const str_t s, const CHAR *pattern)                                    \
    {                                                                                 \
        const CHAR *str = (const CHAR *)s;                                            \
        const CHAR *p = pattern;                                                      \
        const CHAR *st = str;                                                         \
        const CHAR *star = NULL;                                                      \
        const CHAR *ss = NULL;                                                        \
        while (*st)                                                                   \
        {                                                                             \
            if (*p == (CHAR)'*')                                                      \
            {                                                                         \
                star = p++;                                                           \
                ss = st;                                                              \
            }                                                                         \
            else if (*p && (*p == (CHAR)'?' || *p == *st))                            \
            {                                                                         \
                p++;                                                                  \
                st++;                                                                 \
            }                                                                         \
            else if (*p == (CHAR)'[')                                                 \
            {                                                                         \
                const CHAR *q = p + 1;                                                \
                bool negate = false;                                                  \
                if (*q == (CHAR)'^')                                                  \
                {                                                                     \
                    negate = true;                                                    \
                    q++;                                                              \
                }                                                                     \
                bool matched = false;                                                 \
                CHAR prev = 0;                                                        \
                while (*q && *q != (CHAR)']')                                         \
                {                                                                     \
                    if (*q == (CHAR)'-' && prev && *(q + 1) && *(q + 1) != (CHAR)']') \
                    {                                                                 \
                        CHAR start = prev;                                            \
                        CHAR end = *(q + 1);                                          \
                        if (*st >= start && *st <= end)                               \
                            matched = true;                                           \
                        q += 2;                                                       \
                        prev = 0;                                                     \
                        continue;                                                     \
                    }                                                                 \
                    else                                                              \
                    {                                                                 \
                        if (*st == *q)                                                \
                            matched = true;                                           \
                        prev = *q;                                                    \
                    }                                                                 \
                    q++;                                                              \
                }                                                                     \
                /* advance p past bracket */                                          \
                while (*p && *p != (CHAR)']')                                         \
                    p++;                                                              \
                if (*p == (CHAR)']')                                                  \
                    p++;                                                              \
                if (negate)                                                           \
                    matched = !matched;                                               \
                if (matched)                                                          \
                {                                                                     \
                    st++;                                                             \
                    continue;                                                         \
                }                                                                     \
                if (star)                                                             \
                {                                                                     \
                    p = star + 1;                                                     \
                    st = ++ss;                                                        \
                    continue;                                                         \
                }                                                                     \
                return false;                                                         \
            }                                                                         \
            else                                                                      \
            {                                                                         \
                if (star)                                                             \
                {                                                                     \
                    p = star + 1;                                                     \
                    st = ++ss;                                                        \
                    continue;                                                         \
                }                                                                     \
                return false;                                                         \
            }                                                                         \
        }                                                                             \
        while (*p == (CHAR)'*')                                                       \
            p++;                                                                      \
        return *p == (CHAR)'\0';                                                      \
    }
WILDCARD_IMPL(str_wildcard_ascii, char)
WILDCARD_IMPL(str_wildcard_wide, wchar_t)
#undef WILDCARD_IMPL

size_t str_length(const str_t s) {
    return STR_LENGTH(s);
}

char str_char_at(const str_t s, size_t index) {
    if (!str_is_ascii(s))
        return 0;
    uint32_t len = str_length(s);
    if (index >= len)
        return 0;
    return ((const char *)s)[index];
}

wchar_t str_wchar_at(const str_t s, size_t index) {
    if (!str_is_utf16(s))
        return 0;
    uint32_t len = str_length(s);
    if (index >= len)
        return 0;
    return ((const wchar_t *)s)[index];
}

#endif // PAUL_STRING_IMPLEMENTATION
