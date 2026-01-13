/* paul_format.h -- https://github.com/takeiteasy/paul

 Copyright (C) 2025 George Watson

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
 @header paul_format.h
 @copyright George Watson GPLv3
 @updated 2025-09-29
 @brief Common Lisp-style format function for C.
 @discussion
    A single-header library implementing CL-style format directives using:
    - _Generic for type-polymorphic formatting
    - Array-based arguments for navigation support

    Implementation is included when FORMAT_IMPLEMENTATION is defined.

    Supported directives:
    ~A    - Aesthetic (human-readable)
    ~S    - Standard (with escape chars for strings)
    ~D    - Decimal integer
    ~B    - Binary
    ~O    - Octal
    ~X    - Hexadecimal
    ~F    - Fixed-point floating
    ~E    - Exponential notation
    ~G    - General float (chooses ~F or ~E)
    ~$    - Monetary (2 decimal places)
    ~C    - Character
    ~P    - Plural (adds 's' if arg != 1)
    ~:P   - Plural using previous arg
    ~%    - Newline
    ~&    - Fresh line
    ~T    - Tab
    ~|    - Page separator
    ~~    - Literal tilde
    ~R    - Radix (English words: "forty-two")
    ~:R   - Ordinal numbers ("forty-second")
    ~@R   - Roman numerals ("MCMXCIV")
    ~nR   - Print in base n (2-36), e.g. ~2R for binary
    ~[..~;..~]   - Conditional (numeric selector)
    ~:[..~;..~]  - Boolean conditional
    ~@[..~]      - Optional (include if non-nil)
    ~{..~}       - Iteration over array
    ~^           - Escape iteration if no more items
    ~*           - Skip argument forward
    ~:*          - Skip argument backward
    ~n@*         - Go to absolute argument position n
    ~(..~)       - Lowercase
    ~:(..~)      - Capitalize
    ~@(..~)      - Uppercase
    ~?           - Indirect (format string from arg, args from array arg)
    ~@?          - Indirect (format string from arg, uses remaining args)

    Padding parameters (optional):
    ~10A         - Pad to 10 chars (right-pad)
    ~10@A        - Pad to 10 chars (left-pad)
    ~10,'0D      - Pad with character '0'
    ~,2F         - 2 decimal places
    ~:D          - With comma separators (1,000,000)
    ~,,'_:D      - With custom separator (1_000_000)
    ~vD          - Width from next argument
    ~v,vF        - Width and precision from arguments
    ~#D          - Width = number of remaining arguments
*/

#ifndef PAUL_FORMAT_H
#define PAUL_FORMAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/*!
 * @enum FormatError
 * @brief Error codes returned by formatting functions.
 * @constant FORMAT_OK No error occurred.
 * @constant FORMAT_ERR_NULL_BUFFER The provided output buffer was NULL.
 * @constant FORMAT_ERR_ZERO_SIZE The provided buffer size was zero.
 * @constant FORMAT_ERR_NULL_FORMAT The format string was NULL.
 * @constant FORMAT_ERR_BUFFER_OVERFLOW The output would exceed the buffer capacity.
 * @constant FORMAT_ERR_INVALID_DIRECTIVE An unknown or malformed directive was encountered.
 * @constant FORMAT_ERR_MISSING_ARG A directive required an argument but none were available.
 * @constant FORMAT_ERR_UNCLOSED_BRACKET A bracketing directive (e.g., ~{, ~[) was not closed.
 * @constant FORMAT_ERR_INVALID_RADIX An invalid radix was specified for the ~R directive.
 */
typedef enum {
    FORMAT_OK = 0,
    FORMAT_ERR_NULL_BUFFER,
    FORMAT_ERR_ZERO_SIZE,
    FORMAT_ERR_NULL_FORMAT,
    FORMAT_ERR_BUFFER_OVERFLOW,
    FORMAT_ERR_INVALID_DIRECTIVE,
    FORMAT_ERR_MISSING_ARG,
    FORMAT_ERR_UNCLOSED_BRACKET,
    FORMAT_ERR_INVALID_RADIX,
} FormatError;

/*!
 * @enum FormatArgType
 * @brief Types of arguments supported by the formatting system.
 * @constant FORMAT_TYPE_NIL Represents a nil/null value.
 * @constant FORMAT_TYPE_INT Signed integer.
 * @constant FORMAT_TYPE_UINT Unsigned integer.
 * @constant FORMAT_TYPE_LONG Signed long.
 * @constant FORMAT_TYPE_ULONG Unsigned long.
 * @constant FORMAT_TYPE_LLONG Signed long long.
 * @constant FORMAT_TYPE_ULLONG Unsigned long long.
 * @constant FORMAT_TYPE_DOUBLE Double precision floating point.
 * @constant FORMAT_TYPE_CHAR Single character.
 * @constant FORMAT_TYPE_STRING Null-terminated C string.
 * @constant FORMAT_TYPE_POINTER Generic pointer.
 * @constant FORMAT_TYPE_BOOL Boolean value.
 * @constant FORMAT_TYPE_ARRAY Array argument for iteration support (~{).
 * @constant FORMAT_TYPE_CUSTOM User-defined custom type.
 */
typedef enum {
    FORMAT_TYPE_NIL,
    FORMAT_TYPE_INT,
    FORMAT_TYPE_UINT,
    FORMAT_TYPE_LONG,
    FORMAT_TYPE_ULONG,
    FORMAT_TYPE_LLONG,
    FORMAT_TYPE_ULLONG,
    FORMAT_TYPE_DOUBLE,
    FORMAT_TYPE_CHAR,
    FORMAT_TYPE_STRING,
    FORMAT_TYPE_POINTER,
    FORMAT_TYPE_BOOL,
    FORMAT_TYPE_ARRAY,       /* For iteration support */
    FORMAT_TYPE_CUSTOM,      /* User-defined types */
} FormatArgType;

/* Forward declaration for custom formatter */
struct FormatArg;
struct FormatState;

/*!
 * @typedef FormatCustomFunc
 * @brief Function pointer type for custom argument formatting.
 * @param state The current format state.
 * @param data User-provided data for the custom formatter.
 */
typedef void (*FormatCustomFunc)(struct FormatState *state, void *data);

/*!
 * @struct FormatArg
 * @brief Represents a single argument passed to the format function.
 * @field type The type of the argument.
 * @field value Union containing the argument value based on its type.
 */
typedef struct FormatArg {
    FormatArgType type;
    union {
        int as_int;
        unsigned int as_uint;
        long as_long;
        unsigned long as_ulong;
        long long as_llong;
        unsigned long long as_ullong;
        double as_double;
        char as_char;
        const char *as_string;
        void *as_pointer;
        bool as_bool;
        struct {
            struct FormatArg *items;
            size_t count;
        } as_array;
        struct {
            FormatCustomFunc func;
            void *data;
        } as_custom;
    } value;
} FormatArg;

/*!
 * @struct FormatState
 * @brief Maintains the state of the formatting process.
 * @field output Pointer to the output buffer.
 * @field capacity Total capacity of the output buffer.
 * @field pos Current writing position in the output buffer.
 * @field args Array of arguments passed to the format function.
 * @field arg_count Total number of arguments.
 * @field arg_index Current argument index being processed.
 * @field error Current error state.
 * @field error_pos Pointer to the position in the format string where the error occurred.
 */
typedef struct FormatState {
    char *output;
    size_t capacity;
    size_t pos;
    FormatArg *args;
    size_t arg_count;
    size_t arg_index;
    FormatError error;
    const char *error_pos;  /* Position in format string where error occurred */
} FormatState;

/*!
 * @typedef FormatFunc
 * @brief Function pointer type for registered directives (~/func/).
 * @param state The current format state.
 * @param arg The argument to be formatted.
 */
typedef void (*FormatFunc)(FormatState *state, FormatArg *arg);

/*!
 * @function format_register_func
 * @brief Registers a named function for use with the ~/func/ directive.
 * @param name The name of the function to register (max 32 functions total).
 * @param func The function pointer to register.
 */
void format_register_func(const char *name, FormatFunc func);

/*!
 * @function format_clear_funcs
 * @brief Clears all registered custom format functions.
 */
void format_clear_funcs(void);

static inline FormatArg format_arg_bool(bool x) {
    return (FormatArg){.type = FORMAT_TYPE_BOOL, .value.as_bool = x};
}

static inline FormatArg format_arg_char(char x) {
    return (FormatArg){.type = FORMAT_TYPE_CHAR, .value.as_char = x};
}

static inline FormatArg format_arg_int(int x) {
    return (FormatArg){.type = FORMAT_TYPE_INT, .value.as_int = x};
}

static inline FormatArg format_arg_uint(unsigned int x) {
    return (FormatArg){.type = FORMAT_TYPE_UINT, .value.as_uint = x};
}

static inline FormatArg format_arg_long(long x) {
    return (FormatArg){.type = FORMAT_TYPE_LONG, .value.as_long = x};
}

static inline FormatArg format_arg_ulong(unsigned long x) {
    return (FormatArg){.type = FORMAT_TYPE_ULONG, .value.as_ulong = x};
}

static inline FormatArg format_arg_llong(long long x) {
    return (FormatArg){.type = FORMAT_TYPE_LLONG, .value.as_llong = x};
}

static inline FormatArg format_arg_ullong(unsigned long long x) {
    return (FormatArg){.type = FORMAT_TYPE_ULLONG, .value.as_ullong = x};
}

static inline FormatArg format_arg_double(double x) {
    return (FormatArg){.type = FORMAT_TYPE_DOUBLE, .value.as_double = x};
}

static inline FormatArg format_arg_float(float x) {
    return (FormatArg){.type = FORMAT_TYPE_DOUBLE, .value.as_double = (double)x};
}

static inline FormatArg format_arg_string(const char *x) {
    return (FormatArg){.type = FORMAT_TYPE_STRING, .value.as_string = x};
}

static inline FormatArg format_arg_ptr(void *x) {
    return (FormatArg){.type = FORMAT_TYPE_POINTER, .value.as_pointer = x};
}

static inline FormatArg format_arg_cptr(const void *x) {
    return (FormatArg){.type = FORMAT_TYPE_POINTER, .value.as_pointer = (void*)x};
}

/*!
 * @defined FORMAT_ARG
 * @brief Macro to wrap a value into a FormatArg structure using _Generic.
 * @param x The value to wrap.
 */
#define FORMAT_ARG(x) _Generic((x), \
_Bool: format_arg_bool, \
char: format_arg_char, \
signed char: format_arg_int, \
unsigned char: format_arg_uint, \
short: format_arg_int, \
unsigned short: format_arg_uint, \
int: format_arg_int, \
unsigned int: format_arg_uint, \
long: format_arg_long, \
unsigned long: format_arg_ulong, \
long long: format_arg_llong, \
unsigned long long: format_arg_ullong, \
float: format_arg_float, \
double: format_arg_double, \
char*: format_arg_string, \
const char*: format_arg_string, \
void*: format_arg_ptr, \
const void*: format_arg_cptr, \
default: format_arg_ptr)(x)

/*!
 * @defined FORMAT_NIL
 * @brief Represents a NIL argument (placeholder).
 */
#define FORMAT_NIL ((FormatArg){.type = FORMAT_TYPE_NIL})

/*!
 * @defined FORMAT_ARRAY
 * @brief Creates a FormatArg representing an array for iteration (~{).
 * @param arr Pointer to an array of FormatArg structures.
 * @param cnt Number of elements in the array.
 */
#define FORMAT_ARRAY(arr, cnt) \
((FormatArg){.type = FORMAT_TYPE_ARRAY, .value.as_array = {(arr), (cnt)}})

/*!
 * @defined FORMAT_CUSTOM
 * @brief Creates a FormatArg with a custom formatting function.
 * @param func_ptr Pointer to the custom format function.
 * @param data_ptr User data pointer passed to the function.
 */
#define FORMAT_CUSTOM(func_ptr, data_ptr) \
((FormatArg){.type = FORMAT_TYPE_CUSTOM, .value.as_custom = {(func_ptr), (data_ptr)}})

/* Count variadic arguments (up to 32) */
#define FORMAT_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, \
_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32, N, ...) N
#define FORMAT_NARGS_(...) FORMAT_ARG_N(__VA_ARGS__)
#define FORMAT_NARGS(...) FORMAT_NARGS_(__VA_ARGS__, \
32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17, \
16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

/* Overloaded macros for wrapping args */
#define FORMAT_W1(a) FORMAT_ARG(a)
#define FORMAT_W2(a,b) FORMAT_ARG(a), FORMAT_ARG(b)
#define FORMAT_W3(a,b,c) FORMAT_ARG(a), FORMAT_ARG(b), FORMAT_ARG(c)
#define FORMAT_W4(a,b,c,d) FORMAT_ARG(a), FORMAT_ARG(b), FORMAT_ARG(c), FORMAT_ARG(d)
#define FORMAT_W5(a,b,c,d,e) FORMAT_W4(a,b,c,d), FORMAT_ARG(e)
#define FORMAT_W6(a,b,c,d,e,f) FORMAT_W5(a,b,c,d,e), FORMAT_ARG(f)
#define FORMAT_W7(a,b,c,d,e,f,g) FORMAT_W6(a,b,c,d,e,f,g), FORMAT_ARG(g)
#define FORMAT_W8(a,b,c,d,e,f,g,h) FORMAT_W7(a,b,c,d,e,f,g), FORMAT_ARG(h)
#define FORMAT_W9(a,b,c,d,e,f,g,h,i) FORMAT_W8(a,b,c,d,e,f,g,h), FORMAT_ARG(i)
#define FORMAT_W10(a,b,c,d,e,f,g,h,i,j) FORMAT_W9(a,b,c,d,e,f,g,h,i), FORMAT_ARG(j)
#define FORMAT_W11(a,b,c,d,e,f,g,h,i,j,k) FORMAT_W10(a,b,c,d,e,f,g,h,i,j), FORMAT_ARG(k)
#define FORMAT_W12(a,b,c,d,e,f,g,h,i,j,k,l) FORMAT_W11(a,b,c,d,e,f,g,h,i,j,k), FORMAT_ARG(l)
#define FORMAT_W13(a,b,c,d,e,f,g,h,i,j,k,l,m) FORMAT_W12(a,b,c,d,e,f,g,h,i,j,k,l), FORMAT_ARG(m)
#define FORMAT_W14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) FORMAT_W13(a,b,c,d,e,f,g,h,i,j,k,l,m), FORMAT_ARG(n)
#define FORMAT_W15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) FORMAT_W14(a,b,c,d,e,f,g,h,i,j,k,l,m,n), FORMAT_ARG(o)
#define FORMAT_W16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) FORMAT_W15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o), FORMAT_ARG(p)

/* Select wrapper based on arg count */
#define FORMAT_WRAP_N(N) FORMAT_W##N
#define FORMAT_WRAP_(N, ...) FORMAT_WRAP_N(N)(__VA_ARGS__)
#define FORMAT_WRAP(...) FORMAT_WRAP_(FORMAT_NARGS(__VA_ARGS__), __VA_ARGS__)

/*!
 * @function format_error_string
 * @brief Returns a string description of a format error code.
 * @param err The error code.
 * @return A string describing the error.
 */
const char *format_error_string(FormatError err);

/*!
 * @function format_get_error
 * @brief Retrieves the last error that occurred on the current thread.
 * @return The last FormatError code.
 */
FormatError format_get_error(void);

/*!
 * @function format_impl
 * @brief Internal implementation of the format function.
 * @discussion Use the format() macro instead of calling this directly.
 * @param buf Output buffer.
 * @param bufsize Size of the output buffer.
 * @param fmt_string Format string.
 * @param args Array of format arguments.
 * @param arg_count Number of arguments.
 * @return Number of characters written (excluding null terminator), or -1 on error.
 */
int format_impl(char *buf, size_t bufsize, const char *fmt_string,
                FormatArg *args, size_t arg_count);

/*!
 * @function format_impl_ex
 * @brief Extended implementation of the format function with error reporting.
 * @param buf Output buffer.
 * @param bufsize Size of the output buffer.
 * @param fmt_string Format string.
 * @param args Array of format arguments.
 * @param arg_count Number of arguments.
 * @param err_out Pointer to store the error code (optional).
 * @return Number of characters written (excluding null terminator), or -1 on error.
 */
int format_impl_ex(char *buf, size_t bufsize, const char *fmt_string,
                   FormatArg *args, size_t arg_count, FormatError *err_out);

/*!
 * @function format_alloc
 * @brief Formats a string and allocates memory for the result.
 * @discussion The returned string must be freed by the caller.
 * @param fmt_string Format string.
 * @param args Array of format arguments.
 * @param arg_count Number of arguments.
 * @return Pointer to the allocated formatted string, or NULL on error.
 */
char *format_alloc(const char *fmt_string, FormatArg *args, size_t arg_count);

/*!
 * @function format_print
 * @brief Formats a string and prints it to stdout.
 * @param fmt_string Format string.
 * @param args Array of format arguments.
 * @param arg_count Number of arguments.
 * @return Number of characters printed.
 */
int format_print(const char *fmt_string, FormatArg *args, size_t arg_count);

/*!
 * @function format_fprint
 * @brief Formats a string and prints it to a file stream.
 * @param fp File pointer to write to.
 * @param fmt_string Format string.
 * @param args Array of format arguments.
 * @param arg_count Number of arguments.
 * @return Number of characters written.
 */
int format_fprint(FILE *fp, const char *fmt_string, FormatArg *args, size_t arg_count);

/*!
 * @defined format
 * @brief Formats a string into a buffer using CL-style directives.
 * @param buf The output buffer.
 * @param bufsize The size of the output buffer.
 * @param fmt_str The format string.
 * @param ... Variadic arguments to be formatted.
 */
#define format(buf, bufsize, fmt_str, ...) \
format_impl(buf, bufsize, fmt_str, \
(FormatArg[]){FORMAT_WRAP(__VA_ARGS__)}, \
FORMAT_NARGS(__VA_ARGS__))

/*!
 * @defined format0
 * @brief Formats a string with no arguments into a buffer.
 * @param buf The output buffer.
 * @param bufsize The size of the output buffer.
 * @param fmt_str The format string.
 */
#define format0(buf, bufsize, fmt_str) \
format_impl(buf, bufsize, fmt_str, NULL, 0)

/*!
 * @defined format_new
 * @brief Formats a string into a newly allocated buffer.
 * @param fmt_str The format string.
 * @param ... Variadic arguments to be formatted.
 * @return Pointer to allocated string (caller must free).
 */
#define format_new(fmt_str, ...) \
format_alloc(fmt_str, \
(FormatArg[]){FORMAT_WRAP(__VA_ARGS__)}, \
FORMAT_NARGS(__VA_ARGS__))

/*!
 * @defined format_new0
 * @brief Formats a string with no arguments into a newly allocated buffer.
 * @param fmt_str The format string.
 * @return Pointer to allocated string (caller must free).
 */
#define format_new0(fmt_str) \
format_alloc(fmt_str, NULL, 0)

/*!
 * @defined formatf
 * @brief Formats a string and prints it to stdout.
 * @param fmt_str The format string.
 * @param ... Variadic arguments to be formatted.
 */
#define formatf(fmt_str, ...) \
format_print(fmt_str, \
(FormatArg[]){FORMAT_WRAP(__VA_ARGS__)}, \
FORMAT_NARGS(__VA_ARGS__))

/*!
 * @defined formatf0
 * @brief Formats a string with no arguments and prints it to stdout.
 * @param fmt_str The format string.
 */
#define formatf0(fmt_str) \
format_print(fmt_str, NULL, 0)

/*!
 * @defined formatfp
 * @brief Formats a string and prints it to a file stream.
 * @param fp The file pointer.
 * @param fmt_str The format string.
 * @param ... Variadic arguments to be formatted.
 */
#define formatfp(fp, fmt_str, ...) \
format_fprint(fp, fmt_str, \
(FormatArg[]){FORMAT_WRAP(__VA_ARGS__)}, \
FORMAT_NARGS(__VA_ARGS__))

#ifdef __cplusplus
}
#endif

#if defined(PAUL_IMPLEMENTATION) || defined(PAUL_FORMAT_IMPLEMENTATION)
/* Suppress unused function warnings for inline functions */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

/* Thread-local error storage */
#if defined(_MSC_VER)
#define FORMAT_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
#define FORMAT_THREAD_LOCAL __thread
#else
#define FORMAT_THREAD_LOCAL  /* fallback: not thread-safe */
#endif

static FORMAT_THREAD_LOCAL FormatError format_last_error = FORMAT_OK;

/* Get error string for error code */
const char *format_error_string(FormatError err) {
    switch (err) {
        case FORMAT_OK: return "No error";
        case FORMAT_ERR_NULL_BUFFER: return "NULL buffer";
        case FORMAT_ERR_ZERO_SIZE: return "Zero buffer size";
        case FORMAT_ERR_NULL_FORMAT: return "NULL format string";
        case FORMAT_ERR_BUFFER_OVERFLOW: return "Buffer overflow";
        case FORMAT_ERR_INVALID_DIRECTIVE: return "Invalid directive";
        case FORMAT_ERR_MISSING_ARG: return "Missing argument";
        case FORMAT_ERR_UNCLOSED_BRACKET: return "Unclosed bracket";
        case FORMAT_ERR_INVALID_RADIX: return "Invalid radix";
        default: return "Unknown error";
    }
}

/* Get last error (thread-local) */
FormatError format_get_error(void) {
    return format_last_error;
}

/* Set error */
static void format_set_error(FormatState *state, FormatError err, const char *pos) {
    state->error = err;
    state->error_pos = pos;
    format_last_error = err;
}

/* Function registry for ~/func/ directive */
#define FORMAT_MAX_FUNCS 32

typedef struct {
    const char *name;
    FormatFunc func;
} FormatFuncEntry;

static FormatFuncEntry format_func_registry[FORMAT_MAX_FUNCS];
static size_t format_func_count = 0;

void format_register_func(const char *name, FormatFunc func) {
    if (format_func_count < FORMAT_MAX_FUNCS && name && func) {
        format_func_registry[format_func_count].name = name;
        format_func_registry[format_func_count].func = func;
        format_func_count++;
    }
}

void format_clear_funcs(void) {
    format_func_count = 0;
}

static FormatFunc format_lookup_func(const char *name, size_t len) {
    for (size_t i = 0; i < format_func_count; i++) {
        if (strlen(format_func_registry[i].name) == len &&
            strncmp(format_func_registry[i].name, name, len) == 0)
            return format_func_registry[i].func;

    }
    return NULL;
}

/* Append string to output buffer */
static void format_append_str(FormatState *state, const char *str) {
    if (!str) return;
    size_t len = strlen(str);
    if (state->pos + len >= state->capacity)
        len = state->capacity - state->pos - 1;

    memcpy(state->output + state->pos, str, len);
    state->pos += len;
    state->output[state->pos] = '\0';
}

/* Append single character */
static void format_append_char(FormatState *state, char c) {
    if (state->pos + 1 < state->capacity) {
        state->output[state->pos++] = c;
        state->output[state->pos] = '\0';
    }
}

/* Append formatted string */
static void format_append_fmt(FormatState *state, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(state->output + state->pos,
                            state->capacity - state->pos, fmt, args);
    va_end(args);
    if (written > 0) {
        if ((size_t)written >= state->capacity - state->pos)
            state->pos = state->capacity - 1;
        else
            state->pos += written;

    }
}

/* Get integer value from argument (any integer type) */
static long long format_arg_to_int(FormatArg *arg) {
    switch (arg->type) {
        case FORMAT_TYPE_INT: return arg->value.as_int;
        case FORMAT_TYPE_UINT: return arg->value.as_uint;
        case FORMAT_TYPE_LONG: return arg->value.as_long;
        case FORMAT_TYPE_ULONG: return arg->value.as_ulong;
        case FORMAT_TYPE_LLONG: return arg->value.as_llong;
        case FORMAT_TYPE_ULLONG: return (long long)arg->value.as_ullong;
        case FORMAT_TYPE_CHAR: return arg->value.as_char;
        case FORMAT_TYPE_BOOL: return arg->value.as_bool ? 1 : 0;
        case FORMAT_TYPE_DOUBLE: return (long long)arg->value.as_double;
        default: return 0;
    }
}

/* Get double value from argument */
static double format_arg_to_double(FormatArg *arg) {
    switch (arg->type) {
        case FORMAT_TYPE_DOUBLE: return arg->value.as_double;
        case FORMAT_TYPE_INT: return arg->value.as_int;
        case FORMAT_TYPE_UINT: return arg->value.as_uint;
        case FORMAT_TYPE_LONG: return arg->value.as_long;
        case FORMAT_TYPE_ULONG: return arg->value.as_ulong;
        case FORMAT_TYPE_LLONG: return arg->value.as_llong;
        case FORMAT_TYPE_ULLONG: return (double)arg->value.as_ullong;
        default: return 0.0;
    }
}

/* Check if argument is "truthy" for conditionals */
static bool format_arg_is_true(FormatArg *arg) {
    switch (arg->type) {
        case FORMAT_TYPE_NIL: return false;
        case FORMAT_TYPE_BOOL: return arg->value.as_bool;
        case FORMAT_TYPE_INT: return arg->value.as_int != 0;
        case FORMAT_TYPE_UINT: return arg->value.as_uint != 0;
        case FORMAT_TYPE_LONG: return arg->value.as_long != 0;
        case FORMAT_TYPE_ULONG: return arg->value.as_ulong != 0;
        case FORMAT_TYPE_LLONG: return arg->value.as_llong != 0;
        case FORMAT_TYPE_ULLONG: return arg->value.as_ullong != 0;
        case FORMAT_TYPE_DOUBLE: return arg->value.as_double != 0.0;
        case FORMAT_TYPE_CHAR: return arg->value.as_char != 0;
        case FORMAT_TYPE_STRING: return arg->value.as_string != NULL && arg->value.as_string[0] != '\0';
        case FORMAT_TYPE_POINTER: return arg->value.as_pointer != NULL;
        case FORMAT_TYPE_ARRAY: return arg->value.as_array.count > 0;
        case FORMAT_TYPE_CUSTOM: return arg->value.as_custom.func != NULL;
    }
    return false;
}

/* Format integer with custom separator */
static void format_int_with_separator(FormatState *state, long long value, char sep) {
    char buf[64];
    char result[128];
    int neg = value < 0;
    if (neg) value = -value;

    snprintf(buf, sizeof(buf), "%lld", value);
    size_t len = strlen(buf);

    int j = 0;
    if (neg) result[j++] = '-';

    for (size_t i = 0; i < len; i++) {
        if (i > 0 && (len - i) % 3 == 0)
            result[j++] = sep;

        result[j++] = buf[i];
    }
    result[j] = '\0';
    format_append_str(state, result);
}

/* English number words (for ~R) */
static const char *ones[] = {"", "one", "two", "three", "four",
    "five", "six", "seven", "eight", "nine",
    "ten", "eleven", "twelve", "thirteen", "fourteen",
    "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
static const char *tens[] = {"", "", "twenty", "thirty", "forty",
    "fifty", "sixty", "seventy", "eighty", "ninety"};

static void format_number_english(FormatState *state, long long n);

static void format_hundreds(FormatState *state, long long n, int first) {
    if (n >= 100) {
        if (!first) format_append_char(state, ' ');
        format_append_str(state, ones[n / 100]);
        format_append_str(state, " hundred");
        n %= 100;
        if (n > 0) format_append_char(state, ' ');
    }
    if (n >= 20) {
        format_append_str(state, tens[n / 10]);
        if (n % 10) {
            format_append_char(state, '-');
            format_append_str(state, ones[n % 10]);
        }
    } else if (n > 0)
        format_append_str(state, ones[n]);

}

static void format_number_english(FormatState *state, long long n) {
    if (n == 0) {
        format_append_str(state, "zero");
        return;
    }
    if (n < 0) {
        format_append_str(state, "negative ");
        n = -n;
    }

    int first = 1;

    if (n >= 1000000000000LL) {
        format_hundreds(state, n / 1000000000000LL, first);
        format_append_str(state, " trillion");
        n %= 1000000000000LL;
        first = 0;
    }
    if (n >= 1000000000LL) {
        if (!first) format_append_char(state, ' ');
        format_hundreds(state, n / 1000000000LL, first);
        format_append_str(state, " billion");
        n %= 1000000000LL;
        first = 0;
    }
    if (n >= 1000000) {
        if (!first) format_append_char(state, ' ');
        format_hundreds(state, n / 1000000, first);
        format_append_str(state, " million");
        n %= 1000000;
        first = 0;
    }
    if (n >= 1000) {
        if (!first) format_append_char(state, ' ');
        format_hundreds(state, n / 1000, first);
        format_append_str(state, " thousand");
        n %= 1000;
        first = 0;
    }
    if (n > 0) {
        if (!first) format_append_char(state, ' ');
        format_hundreds(state, n, first);
    }
}

/* Ordinal words */
static const char *ordinal_ones[] = {"", "first", "second", "third", "fourth",
    "fifth", "sixth", "seventh", "eighth", "ninth",
    "tenth", "eleventh", "twelfth", "thirteenth", "fourteenth",
    "fifteenth", "sixteenth", "seventeenth", "eighteenth", "nineteenth"};
static const char *ordinal_tens[] = {"", "", "twentieth", "thirtieth", "fortieth",
    "fiftieth", "sixtieth", "seventieth", "eightieth", "ninetieth"};

static void format_number_ordinal(FormatState *state, long long n) {
    if (n == 0) {
        format_append_str(state, "zeroth");
        return;
    }
    if (n < 0) {
        format_append_str(state, "negative ");
        n = -n;
    }

    /* Handle the non-ordinal parts first */
    if (n >= 100) {
        format_number_english(state, (n / 100) * 100);
        n %= 100;
        if (n > 0)
            format_append_char(state, ' ');
        else {
            /* Just append "th" to the last word */
            format_append_str(state, "th");
            return;
        }
    }

    if (n >= 20) {
        int t = n / 10;
        int o = n % 10;
        if (o == 0)
            format_append_str(state, ordinal_tens[t]);
        else {
            format_append_str(state, tens[t]);
            format_append_char(state, '-');
            format_append_str(state, ordinal_ones[o]);
        }
    } else if (n > 0)
        format_append_str(state, ordinal_ones[n]);

}

/* Roman numerals */
static void format_number_roman(FormatState *state, long long n) {
    if (n <= 0 || n > 3999) {
        format_append_fmt(state, "%lld", n);
        return;
    }

    static const struct { int value; const char *numeral; } roman[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
        {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"},
        {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
    };

    for (int i = 0; i < 13; i++) {
        while (n >= roman[i].value) {
            format_append_str(state, roman[i].numeral);
            n -= roman[i].value;
        }
    }
}

/* Format integer in given base */
static void format_int_base(FormatState *state, long long value, int base, bool uppercase) {
    char buf[65];
    int idx = 64;
    buf[idx] = '\0';

    bool neg = value < 0;
    unsigned long long uval = neg ? -value : value;

    const char *digits = uppercase ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" : "0123456789abcdefghijklmnopqrstuvwxyz";

    if (uval == 0)
        buf[--idx] = '0';
    else {
        while (uval > 0) {
            buf[--idx] = digits[uval % base];
            uval /= base;
        }
    }

    if (neg)
        buf[--idx] = '-';

    format_append_str(state, buf + idx);
}

/* Parse directive parameters */
typedef struct {
    int mincol;           /* Minimum column width */
    int colinc;           /* Column increment */
    int minpad;           /* Minimum padding */
    char padchar;         /* Padding character */
    char separator;       /* Separator for ~:D (default ',') */
    int precision;        /* For floats */
    int radix;            /* Radix for ~nR */
    bool colon;           /* : modifier */
    bool at;              /* @ modifier */
    bool has_mincol;
    bool has_precision;
    bool has_radix;
    bool mincol_from_arg;     /* 'v' - get mincol from next arg */
    bool precision_from_arg;  /* 'v' - get precision from next arg */
} DirectiveParams;

static const char *parse_directive_params(const char *p, DirectiveParams *params, FormatState *state) {
    memset(params, 0, sizeof(*params));
    params->padchar = ' ';
    params->separator = ',';
    params->precision = -1;

    /* Track comma-separated parameter position */
    int param_index = 0;

    while (*p) {
        if (*p == ':') {
            params->colon = true;
            p++;
        } else if (*p == '@') {
            params->at = true;
            p++;
        } else if (*p == 'v' || *p == 'V') {
            /* Get parameter from next argument */
            p++;
            if (param_index == 0) {
                params->mincol_from_arg = true;
                if (state && state->arg_index < state->arg_count) {
                    params->mincol = (int)format_arg_to_int(&state->args[state->arg_index++]);
                    params->has_mincol = true;
                }
            } else {
                params->precision_from_arg = true;
                if (state && state->arg_index < state->arg_count) {
                    params->precision = (int)format_arg_to_int(&state->args[state->arg_index++]);
                    params->has_precision = true;
                }
            }
        } else if (*p >= '0' && *p <= '9') {
            int val = 0;
            while (*p >= '0' && *p <= '9') {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (param_index == 0) {
                params->mincol = val;
                params->has_mincol = true;
            } else {
                params->precision = val;
                params->has_precision = true;
            }
        } else if (*p == ',') {
            p++;
            param_index++;
        } else if (*p == '\'') {
            p++;
            if (*p) {
                /* Can be padchar or separator depending on context */
                if (param_index == 0)
                    params->padchar = *p++;
                else
                    params->separator = *p++;
            }
        } else if (*p == '#') {
            /* # means number of remaining args */
            p++;
            if (state) {
                int remaining = (int)(state->arg_count - state->arg_index);
                if (param_index == 0) {
                    params->mincol = remaining;
                    params->has_mincol = true;
                } else {
                    params->precision = remaining;
                    params->has_precision = true;
                }
            }
        } else
            break;

    }

    /* For ~R, check if mincol is a radix (2-36) */
    if (params->has_mincol && params->mincol >= 2 && params->mincol <= 36) {
        params->radix = params->mincol;
        params->has_radix = true;
    }

    return p;
}

/* Count UTF-8 codepoints (display width approximation) */
static size_t utf8_strlen(const char *str) {
    size_t count = 0;
    const unsigned char *s = (const unsigned char *)str;
    while (*s) {
        /* Count only leading bytes (not continuation bytes 10xxxxxx) */
        if ((*s & 0xC0) != 0x80)
            count++;
        s++;
    }
    return count;
}

/* Apply padding to output (UTF-8 aware) */
static void format_with_padding(FormatState *state, const char *str, DirectiveParams *params) {
    size_t display_len = utf8_strlen(str);  /* Count codepoints, not bytes */
    int mincol = params->mincol > 0 ? params->mincol : 0;

    if ((int)display_len >= mincol) {
        format_append_str(state, str);
        return;
    }

    int padding = mincol - (int)display_len;

    if (params->at) {
        /* Left pad (right align) */
        for (int i = 0; i < padding; i++)
            format_append_char(state, params->padchar);

        format_append_str(state, str);
    } else {
        /* Right pad (left align) */
        format_append_str(state, str);
        for (int i = 0; i < padding; i++)
            format_append_char(state, params->padchar);
    }
}

/* Format ~A (aesthetic) */
static void format_aesthetic(FormatState *state, FormatArg *arg, DirectiveParams *params) {
    char buf[256];

    switch (arg->type) {
        case FORMAT_TYPE_NIL:
            snprintf(buf, sizeof(buf), "nil");
            break;
        case FORMAT_TYPE_INT:
            snprintf(buf, sizeof(buf), "%d", arg->value.as_int);
            break;
        case FORMAT_TYPE_UINT:
            snprintf(buf, sizeof(buf), "%u", arg->value.as_uint);
            break;
        case FORMAT_TYPE_LONG:
            snprintf(buf, sizeof(buf), "%ld", arg->value.as_long);
            break;
        case FORMAT_TYPE_ULONG:
            snprintf(buf, sizeof(buf), "%lu", arg->value.as_ulong);
            break;
        case FORMAT_TYPE_LLONG:
            snprintf(buf, sizeof(buf), "%lld", arg->value.as_llong);
            break;
        case FORMAT_TYPE_ULLONG:
            snprintf(buf, sizeof(buf), "%llu", arg->value.as_ullong);
            break;
        case FORMAT_TYPE_DOUBLE:
            if (params->has_precision)
                snprintf(buf, sizeof(buf), "%.*g", params->precision, arg->value.as_double);
            else
                snprintf(buf, sizeof(buf), "%g", arg->value.as_double);

            break;
        case FORMAT_TYPE_CHAR:
            buf[0] = arg->value.as_char;
            buf[1] = '\0';
            break;
        case FORMAT_TYPE_STRING:
            if (arg->value.as_string) {
                format_with_padding(state, arg->value.as_string, params);
                return;
            }
            snprintf(buf, sizeof(buf), "nil");
            break;
        case FORMAT_TYPE_POINTER:
            snprintf(buf, sizeof(buf), "%p", arg->value.as_pointer);
            break;
        case FORMAT_TYPE_BOOL:
            snprintf(buf, sizeof(buf), "%s", arg->value.as_bool ? "true" : "false");
            break;
        case FORMAT_TYPE_ARRAY:
            snprintf(buf, sizeof(buf), "[array:%zu]", arg->value.as_array.count);
            break;
        case FORMAT_TYPE_CUSTOM:
            if (arg->value.as_custom.func) {
                arg->value.as_custom.func(state, arg->value.as_custom.data);
                return;
            }
            snprintf(buf, sizeof(buf), "[custom]");
            break;
    }

    format_with_padding(state, buf, params);
}

/* Format ~S (standard - with escapes for strings) */
static void format_standard(FormatState *state, FormatArg *arg, DirectiveParams *params) {
    if (arg->type == FORMAT_TYPE_STRING) {
        char buf[256];
        const char *s = arg->value.as_string;
        if (!s) {
            format_with_padding(state, "nil", params);
            return;
        }
        int j = 0;
        buf[j++] = '"';
        while (*s && j < 250) {
            if (*s == '"' || *s == '\\')
                buf[j++] = '\\';

            buf[j++] = *s++;
        }
        buf[j++] = '"';
        buf[j] = '\0';
        format_with_padding(state, buf, params);
    } else {
        /* For non-strings, same as ~A */
        format_aesthetic(state, arg, params);
    }
}

/* Find matching closing bracket */
static const char *find_closing(const char *p, char open, char close) {
    int depth = 1;
    while (*p && depth > 0) {
        if (*p == '~' && *(p+1) == open) {
            depth++;
            p += 2;
        } else if (*p == '~' && *(p+1) == close) {
            depth--;
            if (depth == 0) return p;
            p += 2;
        } else
            p++;

    }
    return p;
}

/* Find next separator at current depth */
static const char *find_separator(const char *start, const char *end) {
    int depth = 0;
    const char *p = start;
    while (p < end) {
        if (*p == '~') {
            if (*(p+1) == '[' || *(p+1) == '{' || *(p+1) == '(') {
                depth++;
                p += 2;
            } else if (*(p+1) == ']' || *(p+1) == '}' || *(p+1) == ')') {
                depth--;
                p += 2;
            } else if (*(p+1) == ';' && depth == 0)
                return p;
            else
                p++;

        } else
            p++;

    }
    return NULL;
}

/* Forward declaration for recursive formatting */
static void format_process(FormatState *state, const char *fmt, const char *end);

/* Format justification ~<...~> */
/* ~mincol< content ~> with ~:< right, ~@< center, ~:@< left justify */
static const char *format_justification(FormatState *state, const char *p, DirectiveParams *params) {
    const char *end = find_closing(p, '<', '>');
    if (!end) return p;

    int mincol = params->has_mincol ? params->mincol : 0;
    char padchar = params->padchar;

    /* First, format the content to a temp buffer to get its length */
    char temp[1024];
    FormatState temp_state = {
        .output = temp,
        .capacity = sizeof(temp),
        .pos = 0,
        .args = state->args,
        .arg_count = state->arg_count,
        .arg_index = state->arg_index,
        .error = FORMAT_OK,
        .error_pos = NULL
    };

    /* Process segments separated by ~; */
    const char *seg_start = p;
    size_t seg_count = 0;
    size_t seg_lens[16];
    const char *seg_starts[16];

    while (seg_start < end && seg_count < 16) {
        const char *sep = find_separator(seg_start, end);
        const char *seg_end = sep ? sep : end;

        seg_starts[seg_count] = seg_start;

        /* Format this segment */
        size_t start_pos = temp_state.pos;
        format_process(&temp_state, seg_start, seg_end);
        seg_lens[seg_count] = temp_state.pos - start_pos;

        seg_count++;

        if (!sep) break;
        seg_start = sep + 2; /* Skip ~; */
    }

    state->arg_index = temp_state.arg_index; /* Update consumed args */

    size_t total_len = temp_state.pos;

    if ((int)total_len >= mincol) {
        /* Just output as-is */
        temp[temp_state.pos] = '\0';
        format_append_str(state, temp);
    } else {
        int padding = mincol - (int)total_len;

        if (params->colon && params->at) {
            /* ~:@< left justify (padding on right) */
            format_append_str(state, temp);
            for (int i = 0; i < padding; i++)
                format_append_char(state, padchar);
        } else if (params->colon) {
            /* ~:< right justify (padding on left) */
            for (int i = 0; i < padding; i++)
                format_append_char(state, padchar);
            format_append_str(state, temp);
        } else if (params->at) {
            /* ~@< center */
            int left_pad = padding / 2;
            int right_pad = padding - left_pad;
            for (int i = 0; i < left_pad; i++)
                format_append_char(state, padchar);
            format_append_str(state, temp);
            for (int i = 0; i < right_pad; i++)
                format_append_char(state, padchar);
        } else {
            /* ~< left justify (default) */
            format_append_str(state, temp);
            for (int i = 0; i < padding; i++)
                format_append_char(state, padchar);
        }
    }

    return end + 2; /* Skip ~> */
}

/* Format conditional ~[...~;...~] */
static const char *format_conditional(FormatState *state, const char *p, DirectiveParams *params) {
    /* Find the end of the conditional */
    const char *end = find_closing(p, '[', ']');

    if (params->colon && params->at) {
        /* ~:@[...~] - Include if non-nil, keep arg available for inner directives */
        if (state->arg_index < state->arg_count) {
            FormatArg *arg = &state->args[state->arg_index];
            if (format_arg_is_true(arg)) {
                /* Don't consume - let inner directives use it */
                format_process(state, p, end);
            }
            /* If nil, don't consume - arg stays for next directive */
        }
    } else if (params->colon) {
        /* ~:[false~;true~] - Boolean conditional */
        if (state->arg_index < state->arg_count) {
            FormatArg *arg = &state->args[state->arg_index++];
            bool truthy = format_arg_is_true(arg);

            const char *sep = find_separator(p, end);
            if (sep) {
                if (truthy)
                    format_process(state, sep + 2, end);
                else
                    format_process(state, p, sep);
            }
        }
    } else if (params->at) {
        /* ~@[...~] - Include if non-nil */
        if (state->arg_index < state->arg_count) {
            FormatArg *arg = &state->args[state->arg_index];
            if (format_arg_is_true(arg))
                format_process(state, p, end);
            else
                state->arg_index++; /* Consume the nil argument */
        }
    } else {
        /* ~[case0~;case1~;...~] - Numeric selector */
        FormatArg *arg = &state->args[state->arg_index++];
        int selector = (int)format_arg_to_int(arg);

        /* Find the right branch */
        int branch = 0;
        const char *branch_start = p;
        const char *branch_end = NULL;
        const char *default_start = NULL;

        while (branch_start < end) {
            const char *sep = find_separator(branch_start, end);
            branch_end = sep ? sep : end;

            /* Check for default case ~:; */
            if (sep && *(sep+1) == ':' && *(sep+2) == ';') {
                default_start = sep + 3;
                break;
            }

            if (branch == selector) {
                format_process(state, branch_start, branch_end);
                goto done;
            }

            branch++;
            if (sep)
                branch_start = sep + 2;
            else
                break;

        }

        /* Use default if available */
        if (default_start)
            format_process(state, default_start, end);

    }

done:
    return end + 2; /* Skip ~] */
}

/* Format iteration ~{...~} */
static const char *format_iteration(FormatState *state, const char *p, DirectiveParams *params) {
    (void)params;
    const char *end = find_closing(p, '{', '}');

    FormatArg *arr_arg = &state->args[state->arg_index++];

    if (arr_arg->type != FORMAT_TYPE_ARRAY) {
        /* Not an array - skip */
        return end + 2;
    }

    FormatArg *items = arr_arg->value.as_array.items;
    size_t count = arr_arg->value.as_array.count;

    /* Check for escape ~^ */
    const char *escape_pos = NULL;
    for (const char *scan = p; scan < end - 1; scan++) {
        if (*scan == '~' && *(scan+1) == '^') {
            escape_pos = scan;
            break;
        }
    }

    /* Save state and iterate */
    FormatArg *saved_args = state->args;
    size_t saved_count = state->arg_count;

    for (size_t i = 0; i < count; i++) {
        /* Set up args for this iteration */
        state->args = &items[i];
        state->arg_count = 1;
        state->arg_index = 0;

        if (escape_pos) {
            /* Format up to escape */
            format_process(state, p, escape_pos);
            /* Check if we should escape (not last element) */
            if (i < count - 1)
                format_process(state, escape_pos + 2, end);

        } else
            format_process(state, p, end);
    }

    /* Restore state */
    state->args = saved_args;
    state->arg_count = saved_count;

    return end + 2;
}

/* Format case conversion ~(...~) */
static const char *format_case(FormatState *state, const char *p, DirectiveParams *params) {
    const char *end = find_closing(p, '(', ')');

    /* Capture output */
    size_t start_pos = state->pos;
    format_process(state, p, end);

    /* Apply case conversion */
    char *str = state->output + start_pos;
    size_t len = state->pos - start_pos;

    if (params->at && params->colon) {
        /* ~:@() - Capitalize all words */
        bool new_word = true;
        for (size_t i = 0; i < len; i++) {
            if (isspace((unsigned char)str[i]))
                new_word = true;
            else if (new_word) {
                str[i] = toupper((unsigned char)str[i]);
                new_word = false;
            }
        }
    } else if (params->at) {
        /* ~@() - Uppercase all */
        for (size_t i = 0; i < len; i++)
            str[i] = toupper((unsigned char)str[i]);
    } else if (params->colon) {
        /* ~:() - Capitalize first letter of each word */
        bool new_word = true;
        for (size_t i = 0; i < len; i++) {
            if (isspace((unsigned char)str[i]))
                new_word = true;
            else if (new_word) {
                str[i] = toupper((unsigned char)str[i]);
                new_word = false;
            } else
                str[i] = tolower((unsigned char)str[i]);
        }
    } else {
        /* ~() - Lowercase all */
        for (size_t i = 0; i < len; i++)
            str[i] = tolower((unsigned char)str[i]);
    }

    return end + 2;
}

/* Process a single directive */
static const char *format_directive(FormatState *state, const char *p) {
    DirectiveParams params;
    p = parse_directive_params(p, &params, state);

    char cmd = *p++;

    switch (toupper((unsigned char)cmd)) {
        case 'A': { /* Aesthetic */
            if (state->arg_index < state->arg_count)
                format_aesthetic(state, &state->args[state->arg_index++], &params);
            break;
        }

        case 'S': { /* Standard */
            if (state->arg_index < state->arg_count)
                format_standard(state, &state->args[state->arg_index++], &params);

            break;
        }

        case 'W': { /* Write - like ~A but specifically for custom types */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                if (arg->type == FORMAT_TYPE_CUSTOM && arg->value.as_custom.func) {
                    /* Call the custom write function */
                    arg->value.as_custom.func(state, arg->value.as_custom.data);
                } else {
                    /* Fall back to aesthetic for non-custom types */
                    state->arg_index--; /* Unconsume */
                    format_aesthetic(state, &state->args[state->arg_index++], &params);
                }
            }
            break;
        }

        case 'D': { /* Decimal */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                long long val = format_arg_to_int(arg);
                if (params.colon)
                    format_int_with_separator(state, val, params.separator);
                else {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%lld", val);
                    format_with_padding(state, buf, &params);
                }
            }
            break;
        }

        case 'B': { /* Binary */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                format_int_base(state, format_arg_to_int(arg), 2, false);
            }
            break;
        }

        case 'O': { /* Octal */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                format_int_base(state, format_arg_to_int(arg), 8, false);
            }
            break;
        }

        case 'X': { /* Hexadecimal */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                format_int_base(state, format_arg_to_int(arg), 16, cmd == 'X');
            }
            break;
        }

        case 'R': { /* Radix */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                long long val = format_arg_to_int(arg);
                if (params.has_radix) {
                    /* ~nR - Print in base n */
                    format_int_base(state, val, params.radix, false);
                } else if (params.at)
                    format_number_roman(state, val);
                else if (params.colon)
                    format_number_ordinal(state, val);
                else
                    format_number_english(state, val);
            }
            break;
        }

        case 'F': { /* Fixed float */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                double val = format_arg_to_double(arg);
                char buf[64];
                int prec = params.has_precision ? params.precision : 6;
                snprintf(buf, sizeof(buf), "%.*f", prec, val);
                format_with_padding(state, buf, &params);
            }
            break;
        }

        case 'E': { /* Exponential */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                double val = format_arg_to_double(arg);
                char buf[64];
                int prec = params.has_precision ? params.precision : 6;
                snprintf(buf, sizeof(buf), "%.*e", prec, val);
                format_with_padding(state, buf, &params);
            }
            break;
        }

        case 'G': { /* General float */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                double val = format_arg_to_double(arg);
                char buf[64];
                int prec = params.has_precision ? params.precision : 6;
                snprintf(buf, sizeof(buf), "%.*g", prec, val);
                format_with_padding(state, buf, &params);
            }
            break;
        }

        case '$': { /* Monetary */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                double val = format_arg_to_double(arg);
                char buf[64];
                snprintf(buf, sizeof(buf), "%.2f", val);
                format_with_padding(state, buf, &params);
            }
            break;
        }

        case 'C': { /* Character */
            if (state->arg_index < state->arg_count) {
                FormatArg *arg = &state->args[state->arg_index++];
                if (arg->type == FORMAT_TYPE_CHAR) {
                    if (params.at) {
                        /* Print character name */
                        char c = arg->value.as_char;
                        if (c == ' ') format_append_str(state, "Space");
                        else if (c == '\n') format_append_str(state, "Newline");
                        else if (c == '\t') format_append_str(state, "Tab");
                        else if (c == '\r') format_append_str(state, "Return");
                        else format_append_char(state, c);
                    } else
                        format_append_char(state, arg->value.as_char);

                } else
                    format_aesthetic(state, arg, &params);
            }
            break;
        }

        case 'P': { /* Plural */
            long long val;
            if (params.colon && state->arg_index > 0) /* ~:P - use previous arg */
                val = format_arg_to_int(&state->args[state->arg_index - 1]);
            else if (state->arg_index < state->arg_count)
                val = format_arg_to_int(&state->args[state->arg_index++]);
            else
                val = 0;

            if (params.at) {
                /* ~@P or ~:@P - y/ies */
                format_append_str(state, val == 1 ? "y" : "ies");
            } else
                format_append_str(state, val == 1 ? "" : "s");

            break;
        }

        case '%': { /* Newline */
            int count = params.has_mincol ? params.mincol : 1;
            for (int i = 0; i < count; i++)
                format_append_char(state, '\n');

            break;
        }

        case '&': { /* Fresh line */
            if (state->pos > 0 && state->output[state->pos - 1] != '\n')
                format_append_char(state, '\n');

            break;
        }

        case 'T': { /* Tab */
            int col = params.has_mincol ? params.mincol : 1;
            if (params.at) {
                /* Relative tab */
                for (int i = 0; i < col; i++)
                    format_append_char(state, ' ');

            } else {
                /* Absolute tab */
                while ((int)state->pos < col)
                    format_append_char(state, ' ');

            }
            break;
        }

        case '|': { /* Page separator - ~n| outputs n page breaks */
            int count = params.has_mincol ? params.mincol : 1;
            for (int i = 0; i < count; i++)
                format_append_char(state, '\f');

            break;
        }

        case '~': { /* Literal tilde */
            format_append_char(state, '~');
            break;
        }

        case '*': { /* Argument navigation */
            if (params.colon) {
                /* ~:* - Go back */
                if (state->arg_index > 0) state->arg_index--;
            } else if (params.at) {
                /* ~@* - Go to absolute position */
                int pos = params.has_mincol ? params.mincol : 0;
                if (pos >= 0 && (size_t)pos < state->arg_count)
                    state->arg_index = pos;

            } else {
                /* ~* - Skip forward */
                int skip = params.has_mincol ? params.mincol : 1;
                state->arg_index += skip;
                if (state->arg_index > state->arg_count)
                    state->arg_index = state->arg_count;
            }
            break;
        }

        case '[': { /* Conditional */
            return format_conditional(state, p, &params);
        }

        case '{': { /* Iteration */
            return format_iteration(state, p, &params);
        }

        case '(': { /* Case conversion */
            return format_case(state, p, &params);
        }

        case '<': { /* Justification */
            return format_justification(state, p, &params);
        }

        case '?': { /* Indirect formatting */
            if (state->arg_index < state->arg_count) {
                FormatArg *fmt_arg = &state->args[state->arg_index++];
                if (fmt_arg->type == FORMAT_TYPE_STRING && fmt_arg->value.as_string) {
                    const char *sub_fmt = fmt_arg->value.as_string;
                    if (params.at) {
                        /* ~@? - Use remaining arguments */
                        FormatArg *remaining_args = &state->args[state->arg_index];
                        size_t remaining_count = state->arg_count - state->arg_index;
                        state->arg_index = state->arg_count; /* Consume all remaining */

                        FormatState sub_state = {
                            .output = state->output,
                            .capacity = state->capacity,
                            .pos = state->pos,
                            .args = remaining_args,
                            .arg_count = remaining_count,
                            .arg_index = 0,
                            .error = FORMAT_OK,
                            .error_pos = NULL
                        };
                        format_process(&sub_state, sub_fmt, NULL);
                        state->pos = sub_state.pos;
                    } else {
                        /* ~? - Next arg should be array of args */
                        if (state->arg_index < state->arg_count) {
                            FormatArg *args_arg = &state->args[state->arg_index++];
                            if (args_arg->type == FORMAT_TYPE_ARRAY) {
                                FormatState sub_state = {
                                    .output = state->output,
                                    .capacity = state->capacity,
                                    .pos = state->pos,
                                    .args = args_arg->value.as_array.items,
                                    .arg_count = args_arg->value.as_array.count,
                                    .arg_index = 0,
                                    .error = FORMAT_OK,
                                    .error_pos = NULL
                                };
                                format_process(&sub_state, sub_fmt, NULL);
                                state->pos = sub_state.pos;
                            }
                        }
                    }
                }
            }
            break;
        }

        case '/': { /* Call registered function ~/func/ */
            const char *name_start = p;
            while (*p && *p != '/') p++;
            if (*p == '/') {
                size_t name_len = p - name_start;
                FormatFunc func = format_lookup_func(name_start, name_len);
                if (func && state->arg_index < state->arg_count)
                    func(state, &state->args[state->arg_index++]);

                p++; /* Skip closing / */
            }
            return p;
        }

        case '^': { /* Escape - handled by iteration */
            break;
        }

        case '\n': { /* Newline in format string */
            /* Skip whitespace after newline */
            while (*p && (*p == ' ' || *p == '\t')) p++;
            break;
        }

default:
            /* Unknown directive - output as-is */
            format_append_char(state, '~');
            format_append_char(state, cmd);
            break;
    }

    return p;
}

/* Process format string (with optional end pointer) */
static void format_process(FormatState *state, const char *fmt, const char *end) {
    const char *p = fmt;

    while (*p && (!end || p < end)) {
        if (*p == '~') {
            p++;
            if (!*p || (end && p >= end)) break;
            p = format_directive(state, p);
        } else
            format_append_char(state, *p++);

    }
}

/* Main implementation function */
int format_impl(char *buf, size_t bufsize, const char *fmt_string,
                FormatArg *args, size_t arg_count) {
    return format_impl_ex(buf, bufsize, fmt_string, args, arg_count, NULL);
}

/* Extended implementation with error output */
int format_impl_ex(char *buf, size_t bufsize, const char *fmt_string,
                   FormatArg *args, size_t arg_count, FormatError *err_out) {
    format_last_error = FORMAT_OK;

    if (!buf) {
        format_last_error = FORMAT_ERR_NULL_BUFFER;
        if (err_out) *err_out = FORMAT_ERR_NULL_BUFFER;
        return -1;
    }
    if (bufsize == 0) {
        format_last_error = FORMAT_ERR_ZERO_SIZE;
        if (err_out) *err_out = FORMAT_ERR_ZERO_SIZE;
        return -1;
    }
    if (!fmt_string) {
        format_last_error = FORMAT_ERR_NULL_FORMAT;
        if (err_out) *err_out = FORMAT_ERR_NULL_FORMAT;
        buf[0] = '\0';
        return -1;
    }

    FormatState state = {
        .output = buf,
        .capacity = bufsize,
        .pos = 0,
        .args = args,
        .arg_count = arg_count,
        .arg_index = 0,
        .error = FORMAT_OK,
        .error_pos = NULL
    };

    buf[0] = '\0';
    format_process(&state, fmt_string, NULL);

    if (err_out) *err_out = state.error;
    if (state.error != FORMAT_OK)
        format_last_error = state.error;

    return (int)state.pos;
}

/* Allocate and return formatted string */
char *format_alloc(const char *fmt_string, FormatArg *args, size_t arg_count) {
    /* First pass: calculate size */
    char small_buf[1024];
    int len = format_impl(small_buf, sizeof(small_buf), fmt_string, args, arg_count);

    if (len < (int)sizeof(small_buf) - 1) {
        /* Fits in small buffer */
        char *result = malloc(len + 1);
        if (result)
            memcpy(result, small_buf, len + 1);

        return result;
    }

    /* Need larger buffer */
    size_t size = len + 256;
    char *result = malloc(size);
    if (result)
        format_impl(result, size, fmt_string, args, arg_count);

    return result;
}

/* Print to stdout */
int format_print(const char *fmt_string, FormatArg *args, size_t arg_count) {
    char buf[4096];
    int len = format_impl(buf, sizeof(buf), fmt_string, args, arg_count);
    if (len >= (int)sizeof(buf) - 1) {
        /* Need larger buffer */
        char *large = format_alloc(fmt_string, args, arg_count);
        if (large) {
            len = (int)strlen(large);
            fputs(large, stdout);
            free(large);
        }
    } else
        fputs(buf, stdout);

    return len;
}

/* Print to file */
int format_fprint(FILE *fp, const char *fmt_string, FormatArg *args, size_t arg_count) {
    char buf[4096];
    int len = format_impl(buf, sizeof(buf), fmt_string, args, arg_count);
    if (len >= (int)sizeof(buf) - 1) {
        char *large = format_alloc(fmt_string, args, arg_count);
        if (large) {
            len = (int)strlen(large);
            fputs(large, fp);
            free(large);
        }
    } else
        fputs(buf, fp);

    return len;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif /* FORMAT_IMPLEMENTATION */
#endif /* FORMAT_H */
