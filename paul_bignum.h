/* paul/paul_bignum.h -- https://github.com/takeiteasy/paul

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
 @header paul_bignum.h
 @copyright George Watson GPLv3
 @updated 2025-07-19
 @brief Arbitrary precision arithmetic library for C/C++.
 @discussion
    Implementation is included when PAUL_BIGNUMBERS_IMPLEMENTATION or PAUL_IMPLEMENTATION is defined.
*/

#ifndef PAUL_BIGNUM_H
#define PAUL_BIGNUM_H
#ifdef __cpluscplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

/*!
 @define BIGNUM_BASE
 @brief The base used for big integer digit representation.
 @discussion Defines the base (10^9) for storing big integer digits, which fits in a uint32_t.
 */
#ifndef BIGNUM_BASE
#define BIGNUM_BASE 1000000000UL // 10^9, fits in uint32_t
#endif

/*!
 @define BIGNUM_DIGIT_BITS
 @brief The number of bits in each digit.
 @discussion Defines the number of bits (log2(BIGNUM_BASE)) for efficient operations.
 */
#ifndef BIGNUM_DIGIT_BITS
#define BIGNUM_DIGIT_BITS 30 // log2(BIGNUM_BASE), for efficient operations
#endif

/*!
 @define BIGNUM_DEFAULT_PRECISION
 @brief The default decimal precision for big floats.
 @discussion Defines the default number of significant decimal digits for big float operations.
 */
#ifndef BIGNUM_DEFAULT_PRECISION
#define BIGNUM_DEFAULT_PRECISION 64 // Default decimal precision for big floats
#endif

/*!
 @define BIGNUM_DEFAULT_EPSILON
 @brief The default epsilon for float comparisons.
 @discussion Defines the default epsilon value used for comparing big floats.
 */
#ifndef BIGNUM_DEFAULT_EPSILON
#define BIGNUM_DEFAULT_EPSILON 1e-30 // Default epsilon for float comparisons
#endif

// Parameter validation macros
#define BIGNUM_VALIDATE_PARAMS_3(a, b, result) \
    if (!a || !b || !result)                   \
        return BIGNUM_ERROR_INVALID_ARGS;

#define BIGNUM_VALIDATE_PARAMS_2(a, result) \
    if (!a || !result)                      \
        return BIGNUM_ERROR_INVALID_ARGS;

#define BIGNUM_VALIDATE_PARAMS_1(a) \
    if (!a)                         \
        return BIGNUM_ERROR_INVALID_ARGS;

#define BIGNUM_VALIDATE_PARAMS_BITWISE(a, b, result)       \
    if (!a || !b || !result || a->negative || b->negative) \
        return BIGNUM_ERROR_INVALID_ARGS;

/*!
 @typedef bignum_err_t
 @brief Enumeration of error codes for bignum operations.
 @discussion Defines the possible error conditions that can occur during arbitrary precision arithmetic operations.
 @field BIGNUM_OK Operation completed successfully.
 @field BIGNUM_ERROR_MEMORY Memory allocation failed.
 @field BIGNUM_ERROR_DIVIDE_BY_ZERO Division by zero attempted.
 @field BIGNUM_ERROR_INVALID_ARGS Invalid arguments provided to function.
 @field BIGNUM_ERROR_OVERFLOW Arithmetic overflow occurred.
 @field BIGNUM_ERROR_UNDERFLOW Arithmetic underflow occurred.
 */
typedef enum bignum_err {
    BIGNUM_OK = 0,
    BIGNUM_ERROR_MEMORY,
    BIGNUM_ERROR_DIVIDE_BY_ZERO,
    BIGNUM_ERROR_INVALID_ARGS,
    BIGNUM_ERROR_OVERFLOW,
    BIGNUM_ERROR_UNDERFLOW
} bignum_err_t;

/*!
 @typedef INT
 @brief Represents an arbitrary precision integer.
 @discussion Uses a base-10^9 representation for efficient operations. The digits array stores the number in little-endian order (least significant digit first).
 @field digits Array of base-10^9 digits (least significant first).
 @field length Number of used digits in the array.
 @field capacity Allocated capacity of the digits array.
 @field negative Sign flag (true for negative numbers).
 */
typedef struct INT {
    uint32_t *digits; // Array of base-10^9 digits (least significant first)
    size_t length;    // Number of used digits
    size_t capacity;  // Allocated capacity
    bool negative;    // Sign flag
} INT;

/*!
 @typedef REAL
 @brief Represents an arbitrary precision floating-point number.
 @discussion Uses a significand (mantissa) and exponent representation. The mantissa is stored as a big integer, and the exponent is a power of 10.
 @field mantissa The significand as a big integer.
 @field exponent Power of 10 exponent.
 @field precision Number of significant decimal digits.
 @field negative Sign flag (true for negative numbers).
 */
typedef struct REAL {
    INT mantissa;     // The significand as a big integer
    int32_t exponent; // Power of 10 exponent
    size_t precision; // Number of significant decimal digits
    bool negative;    // Sign flag
} REAL;

/*!
 @typedef COMPLEX
 @brief Represents an arbitrary precision complex number.
 @discussion Consists of real and imaginary parts, each stored as a REAL.
 @field real Real part of the complex number.
 @field imag Imaginary part of the complex number.
 */
typedef struct COMPLEX {
    REAL real; // Real part
    REAL imag; // Imaginary part
} COMPLEX;

/*!
 @function bigint_new
 @brief Creates a new big integer initialized to zero.
 @discussion Allocates memory for a new INT structure and initializes it to represent the value 0.
 @return A pointer to the new big integer, or NULL on allocation failure.
 */
INT *bigint_new(void);

/*!
 @function bigint_new_from_string
 @brief Creates a new big integer from a string representation.
 @discussion Parses the input string to create a big integer. Supports decimal notation with optional leading sign.
 @param str The string representation of the number.
 @return A pointer to the new big integer, or NULL on failure.
 */
INT *bigint_new_from_string(const char *str);

/*!
 @function bigint_new_from_int64
 @brief Creates a new big integer from a 64-bit integer value.
 @discussion Allocates and initializes a big integer with the given int64_t value.
 @param value The 64-bit integer value to convert.
 @return A pointer to the new big integer, or NULL on failure.
 */
INT *bigint_new_from_int64(int64_t value);

/*!
 @function bigint_new_copy
 @brief Creates a copy of an existing big integer.
 @discussion Allocates a new big integer and copies all data from the source.
 @param src The big integer to copy.
 @return A pointer to the new copy, or NULL on failure.
 */
INT *bigint_new_copy(const INT *src);

/*!
 @function bigint_free
 @brief Frees the memory associated with a big integer.
 @discussion Releases all resources allocated for the big integer. The pointer becomes invalid after this call.
 @param bigint The big integer to free.
 */
void bigint_free(INT *bigint);

/*!
 @function bigint_add
 @brief Adds two big integers.
 @discussion Computes a + b and stores the result in the provided result buffer.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the sum.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_add(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_subtract
 @brief Subtracts two big integers.
 @discussion Computes a - b and stores the result in the provided result buffer.
 @param a The minuend.
 @param b The subtrahend.
 @param result Pointer to store the difference.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_subtract(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_multiply
 @brief Multiplies two big integers.
 @discussion Computes a * b and stores the result in the provided result buffer.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the product.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_multiply(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_divide
 @brief Divides two big integers.
 @discussion Computes quotient and remainder of a / b. Stores quotient in quotient and remainder in remainder if provided.
 @param a The dividend.
 @param b The divisor.
 @param quotient Pointer to store the quotient.
 @param remainder Pointer to store the remainder (can be NULL).
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_divide(const INT *a, const INT *b, INT *quotient, INT *remainder);

/*!
 @function bigint_modulo
 @brief Computes the modulo of two big integers.
 @discussion Computes a % b and stores the result in the provided result buffer.
 @param a The dividend.
 @param b The divisor.
 @param result Pointer to store the remainder.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_modulo(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_power
 @brief Raises a big integer to a power.
 @discussion Computes base^exponent and stores the result in the provided result buffer. Only non-negative exponents are supported.
 @param base The base.
 @param exponent The exponent (must be non-negative).
 @param result Pointer to store the power.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_power(const INT *base, const INT *exponent, INT *result);

/*!
 @function bigint_and
 @brief Performs bitwise AND on two big integers.
 @discussion Computes a & b and stores the result in the provided result buffer. Operands must be non-negative.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_and(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_or
 @brief Performs bitwise OR on two big integers.
 @discussion Computes a | b and stores the result in the provided result buffer. Operands must be non-negative.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_or(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_xor
 @brief Performs bitwise XOR on two big integers.
 @discussion Computes a ^ b and stores the result in the provided result buffer. Operands must be non-negative.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_xor(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_not
 @brief Performs bitwise NOT on a big integer.
 @discussion Computes ~a and stores the result in the provided result buffer. The operand must be non-negative.
 @param a The operand.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_not(const INT *a, INT *result);

/*!
 @function bigint_shift_left
 @brief Shifts a big integer left by a specified number of bits.
 @discussion Computes a << bits and stores the result in the provided result buffer.
 @param a The operand.
 @param bits The number of bits to shift.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_shift_left(const INT *a, size_t bits, INT *result);

/*!
 @function bigint_shift_right
 @brief Shifts a big integer right by a specified number of bits.
 @discussion Computes a >> bits and stores the result in the provided result buffer.
 @param a The operand.
 @param bits The number of bits to shift.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_shift_right(const INT *a, size_t bits, INT *result);

/*!
 @function bigint_compare
 @brief Compares two big integers.
 @discussion Returns -1 if a < b, 0 if a == b, 1 if a > b.
 @param a The first big integer.
 @param b The second big integer.
 @return The comparison result.
*/
int bigint_compare(const INT *a, const INT *b);
/*!
 @function bigint_equals
 @brief Checks if two big integers are equal.
 @discussion Returns true if a == b, false otherwise.
 @param a The first big integer.
 @param b The second big integer.
 @return True if equal, false otherwise.
 */
bool bigint_equals(const INT *a, const INT *b);
/*!
 @function bigint_less_than
 @brief Checks if the first big integer is less than the second.
 @discussion Returns true if a < b, false otherwise.
 @param a The first big integer.
 @param b The second big integer.
 @return True if a < b, false otherwise.
 */
bool bigint_less_than(const INT *a, const INT *b);
/*!
 @function bigint_greater_than
 @brief Checks if the first big integer is greater than the second.
 @discussion Returns true if a > b, false otherwise.
 @param a The first big integer.
 @param b The second big integer.
 @return True if a > b, false otherwise.
 */
bool bigint_greater_than(const INT *a, const INT *b);
/*!
 @function bigint_is_zero
 @brief Checks if a big integer is zero.
 @discussion Returns true if the big integer represents 0, false otherwise.
 @param a The big integer to check.
 @return True if zero, false otherwise.
 */
bool bigint_is_zero(const INT *a);
/*!
 @function bigint_is_negative
 @brief Checks if a big integer is negative.
 @discussion Returns true if the big integer is negative, false otherwise.
 @param a The big integer to check.
 @return True if negative, false otherwise.
 */
bool bigint_is_negative(const INT *a);

/*!
 @function bigint_compare
 @brief Compares two big integers.
 @discussion Returns -1 if a < b, 0 if a == b, 1 if a > b.
 @param a The first big integer.
 @param b The second big integer.
 @return The comparison result.
 */
int bigint_compare(const INT *a, const INT *b);

/*!
 @function bigint_absolute
 @brief Computes the absolute value of a big integer.
 @discussion Takes the absolute value of the input big integer and stores it in the result buffer.
 @param a The input big integer.
 @param result Pointer to store the absolute value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_absolute(const INT *a, INT *result);

/*!
 @function bigint_negate
 @brief Negates a big integer.
 @discussion Flips the sign of the big integer. Zero remains zero.
 @param a The big integer to negate.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_negate(INT *a);

/*!
 @function bigint_ensure_capacity
 @brief Ensures a big integer has sufficient capacity.
 @discussion Resizes the digit array if necessary to accommodate at least the required number of digits.
 @param bigint The big integer to resize.
 @param required_capacity The minimum required capacity.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_ensure_capacity(INT *bigint, size_t required_capacity);

/*!
 @function bigint_set_from_string
 @brief Sets a big integer from a string representation.
 @discussion Parses the input string and sets the big integer to the corresponding value.
 @param bigint The big integer to set.
 @param str The string representation of the number.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_set_from_string(INT *bigint, const char *str);

/*!
 @function bigint_set_from_int64
 @brief Sets a big integer from a 64-bit integer.
 @discussion Converts the int64_t value to a big integer representation.
 @param bigint The big integer to set.
 @param value The 64-bit integer value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_set_from_int64(INT *bigint, int64_t value);

/*!
 @function bigint_to_string
 @brief Converts a big integer to a string representation.
 @discussion Returns a dynamically allocated string representing the big integer in decimal.
 @param bigint The big integer to convert.
 @return A pointer to the string, or NULL on failure. The caller must free the string.
 */
char *bigint_to_string(const INT *bigint);

/*!
 @function bigint_to_string_base
 @brief Converts a big integer to a string in a specified base.
 @discussion Returns a string representation of the big integer in the given base (2-36).
 @param bigint The big integer to convert.
 @param base The base for the conversion (2-36).
 @return A pointer to the string, or NULL on failure. The caller must free the string.
 */
char *bigint_to_string_base(const INT *bigint, int base);

/*!
 @function bigint_to_int64
 @brief Converts a big integer to a 64-bit integer.
 @discussion Attempts to convert the big integer to int64_t, may overflow.
 @param bigint The big integer to convert.
 @return The int64_t value, or INT64_MIN/INT64_MAX on overflow.
 */
int64_t bigint_to_int64(const INT *bigint);

/*!
 @function bigint_create_power
 @brief Creates a big integer representing base raised to an exponent.
 @discussion Computes base^exponent and returns a new big integer with that value.
 @param base The base (int64_t).
 @param exponent The exponent (size_t).
 @return A pointer to the new big integer, or NULL on failure.
 */
INT *bigint_create_power(int64_t base, size_t exponent);

/*!
 @function bigfloat_new
 @brief Creates a new big float initialized to zero.
 @discussion Allocates memory for a new REAL structure and initializes it to represent 0.0 with the specified precision.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big float, or NULL on allocation failure.
*/
REAL *bigfloat_new(size_t precision);

/*!
 @function bigfloat_new_from_string
 @brief Creates a new big float from a string representation.
 @discussion Parses the input string and creates a big float with the specified precision.
 @param str The string representation of the number.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big float, or NULL on failure.
 */
REAL *bigfloat_new_from_string(const char *str, size_t precision);

/*!
 @function bigfloat_new_from_double
 @brief Creates a new big float from a double value.
 @discussion Converts the double to a big float with the specified precision.
 @param value The double value to convert.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big float, or NULL on failure.
 */
REAL *bigfloat_new_from_double(double value, size_t precision);

/*!
 @function bigfloat_new_from_bigint
 @brief Creates a new big float from a big integer.
 @discussion Converts the big integer to a big float with the specified precision.
 @param bigint The big integer to convert.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big float, or NULL on failure.
 */
REAL *bigfloat_new_from_bigint(const INT *bigint, size_t precision);

/*!
 @function bigfloat_new_copy
 @brief Creates a copy of an existing big float.
 @discussion Allocates a new big float and copies all data from the source.
 @param src The big float to copy.
 @return A pointer to the new copy, or NULL on failure.
 */
REAL *bigfloat_new_copy(const REAL *src);

/*!
 @function bigfloat_free
 @brief Frees the memory associated with a big float.
 @discussion Releases all resources allocated for the big float. The pointer becomes invalid after this call.
 @param bigfloat The big float to free.
 */
void bigfloat_free(REAL *bigfloat);

/*!
 @function bigfloat_new
 @brief Creates a new big float initialized to zero.
 @discussion Allocates memory for a new REAL structure and initializes it to represent 0.0 with the specified precision.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big float, or NULL on allocation failure.
 */
REAL *bigfloat_new(size_t precision);

/*!
 @function bigfloat_add
 @brief Adds two big floats.
 @discussion Computes a + b and stores the result in the provided result buffer.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the sum.
 @return BIGNUM_OK on success, or an error code.
*/
bignum_err_t bigfloat_add(const REAL *a, const REAL *b, REAL *result);

/*!
 @function bigfloat_subtract
 @brief Subtracts two big floats.
 @discussion Computes a - b and stores the result in the provided result buffer.
 @param a The minuend.
 @param b The subtrahend.
 @param result Pointer to store the difference.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_subtract(const REAL *a, const REAL *b, REAL *result);

/*!
 @function bigfloat_multiply
 @brief Multiplies two big floats.
 @discussion Computes a * b and stores the result in the provided result buffer.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the product.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_multiply(const REAL *a, const REAL *b, REAL *result);

/*!
 @function bigfloat_divide
 @brief Divides two big floats.
 @discussion Computes a / b and stores the result in the provided result buffer.
 @param a The dividend.
 @param b The divisor.
 @param result Pointer to store the quotient.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_divide(const REAL *a, const REAL *b, REAL *result);

/*!
 @function bigfloat_power
 @brief Raises a big float to a power.
 @discussion Computes base^exponent and stores the result in the provided result buffer.
 @param base The base.
 @param exponent The exponent.
 @param result Pointer to store the power.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_power(const REAL *base, const REAL *exponent, REAL *result);

/*!
 @function bigfloat_add
 @brief Adds two big floats.
 @discussion Computes a + b and stores the result in the provided result buffer.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the sum.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_add(const REAL *a, const REAL *b, REAL *result);

/*!
 @function bigfloat_compare
 @brief Compares two big floats.
 @discussion Returns -1 if a < b, 0 if a == b, 1 if a > b.
 @param a The first big float.
 @param b The second big float.
 @return The comparison result.
 */
int bigfloat_compare(const REAL *a, const REAL *b);

/*!
 @function bigfloat_equals
 @brief Checks if two big floats are equal.
 @discussion Returns true if a == b, false otherwise.
 @param a The first big float.
 @param b The second big float.
 @return True if equal, false otherwise.
 */
bool bigfloat_equals(const REAL *a, const REAL *b);

/*!
 @function bigfloat_less_than
 @brief Checks if the first big float is less than the second.
 @discussion Returns true if a < b, false otherwise.
 @param a The first big float.
 @param b The second big float.
 @return True if a < b, false otherwise.
 */
bool bigfloat_less_than(const REAL *a, const REAL *b);

/*!
 @function bigfloat_greater_than
 @brief Checks if the first big float is greater than the second.
 @discussion Returns true if a > b, false otherwise.
 @param a The first big float.
 @param b The second big float.
 @return True if a > b, false otherwise.
 */
bool bigfloat_greater_than(const REAL *a, const REAL *b);

/*!
 @function bigfloat_is_zero
 @brief Checks if a big float is zero.
 @discussion Returns true if the big float represents 0.0, false otherwise.
 @param a The big float to check.
 @return True if zero, false otherwise.
 */
bool bigfloat_is_zero(const REAL *a);

/*!
 @function bigfloat_is_negative
 @brief Checks if a big float is negative.
 @discussion Returns true if the big float is negative, false otherwise.
 @param a The big float to check.
 @return True if negative, false otherwise.
 */
bool bigfloat_is_negative(const REAL *a);

/*!
 @function bigfloat_absolute
 @brief Computes the absolute value of a big float.
 @discussion Takes the absolute value of the input big float and stores it in the result buffer.
 @param a The input big float.
 @param result Pointer to store the absolute value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_absolute(const REAL *a, REAL *result);

/*!
 @function bigfloat_negate
 @brief Negates a big float.
 @discussion Flips the sign of the big float. Zero remains zero.
 @param a The big float to negate.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_negate(REAL *a);

/*!
 @function bigfloat_set_precision
 @brief Sets the precision of a big float.
 @discussion Updates the precision (number of significant decimal digits) for the big float.
 @param bigfloat The big float to update.
 @param precision The new precision.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_set_precision(REAL *bigfloat, size_t precision);

/*!
 @function bigfloat_set_from_string
 @brief Sets a big float from a string representation.
 @discussion Parses the input string and sets the big float to the corresponding value.
 @param bigfloat The big float to set.
 @param str The string representation of the number.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_set_from_string(REAL *bigfloat, const char *str);

/*!
 @function bigfloat_set_from_double
 @brief Sets a big float from a double value.
 @discussion Converts the double to a big float representation.
 @param bigfloat The big float to set.
 @param value The double value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_set_from_double(REAL *bigfloat, double value);

/*!
 @function bigfloat_set_from_bigint
 @brief Sets a big float from a big integer.
 @discussion Converts the big integer to a big float.
 @param bigfloat The big float to set.
 @param bigint The big integer to convert.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_set_from_bigint(REAL *bigfloat, const INT *bigint);

/*!
 @function bigfloat_copy
 @brief Copies one big float to another.
 @discussion Copies all data from the source big float to the destination.
 @param src The source big float.
 @param dest The destination big float.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_copy(const REAL *src, REAL *dest);

/*!
 @function bigfloat_to_string
 @brief Converts a big float to a string representation.
 @discussion Returns a dynamically allocated string representing the big float.
 @param bigfloat The big float to convert.
 @return A pointer to the string, or NULL on failure. The caller must free the string.
 */
char *bigfloat_to_string(const REAL *bigfloat);

/*!
 @function bigfloat_to_double
 @brief Converts a big float to a double.
 @discussion Attempts to convert the big float to a double value.
 @param bigfloat The big float to convert.
 @return The double value.
 */
double bigfloat_to_double(const REAL *bigfloat);

/*!
 @function bigfloat_to_bigint
 @brief Converts a big float to a big integer.
 @discussion Truncates the big float to an integer value.
 @param bigfloat The big float to convert.
 @return A pointer to the new big integer, or NULL on failure.
 */
INT *bigfloat_to_bigint(const REAL *bigfloat);

/*!
 @function bigint_to_bigfloat
 @brief Converts a big integer to a big float.
 @discussion Creates a big float from a big integer value.
 @param bigint The big integer to convert.
 @return A pointer to the new big float, or NULL on failure.
 */
REAL *bigint_to_bigfloat(const INT *bigint);

/*!
 @function bigfloat_sqrt
 @brief Computes the square root of a big float.
 @discussion Calculates sqrt(a) and stores the result in the provided buffer.
 @param a The input big float (must be non-negative).
 @param result Pointer to store the square root.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_sqrt(const REAL *a, REAL *result);

/*!
 @function bigfloat_floor
 @brief Computes the floor of a big float.
 @discussion Rounds down to the nearest integer and stores the result.
 @param a The input big float.
 @param result Pointer to store the floor value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_floor(const REAL *a, REAL *result);

/*!
 @function bigfloat_ceil
 @brief Computes the ceiling of a big float.
 @discussion Rounds up to the nearest integer and stores the result.
 @param a The input big float.
 @param result Pointer to store the ceiling value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_ceil(const REAL *a, REAL *result);

/*!
 @function bigfloat_round
 @brief Rounds a big float to the nearest integer.
 @discussion Rounds to the nearest integer and stores the result.
 @param a The input big float.
 @param result Pointer to store the rounded value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_round(const REAL *a, REAL *result);

/*!
 @function bigfloat_trunc
 @brief Truncates a big float to an integer.
 @discussion Removes the fractional part and stores the integer part.
 @param a The input big float.
 @param result Pointer to store the truncated value.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_trunc(const REAL *a, REAL *result);

// Mathematical functions

/*!
 @function bigfloat_exp
 @brief Computes the exponential function of a big float.
 @discussion Calculates e^a and stores the result.
 @param a The exponent.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_exp(const REAL *a, REAL *result);

/*!
 @function bigfloat_log
 @brief Computes the natural logarithm of a big float.
 @discussion Calculates ln(a) and stores the result.
 @param a The input (must be positive).
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_log(const REAL *a, REAL *result);

/*!
 @function bigfloat_log10
 @brief Computes the base-10 logarithm of a big float.
 @discussion Calculates log10(a) and stores the result.
 @param a The input (must be positive).
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_log10(const REAL *a, REAL *result);

/*!
 @function bigfloat_log2
 @brief Computes the base-2 logarithm of a big float.
 @discussion Calculates log2(a) and stores the result.
 @param a The input (must be positive).
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_log2(const REAL *a, REAL *result);

/*!
 @function bigfloat_pow
 @brief Computes the power of a big float raised to another.
 @discussion Calculates base^exp and stores the result.
 @param base The base.
 @param exp The exponent.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_pow(const REAL *base, const REAL *exp, REAL *result);

// Trigonometric functions

/*!
 @function bigfloat_sin
 @brief Computes the sine of a big float.
 @discussion Calculates sin(a) and stores the result.
 @param a The angle in radians.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_sin(const REAL *a, REAL *result);

/*!
 @function bigfloat_cos
 @brief Computes the cosine of a big float.
 @discussion Calculates cos(a) and stores the result.
 @param a The angle in radians.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_cos(const REAL *a, REAL *result);

/*!
 @function bigfloat_tan
 @brief Computes the tangent of a big float.
 @discussion Calculates tan(a) and stores the result.
 @param a The angle in radians.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_tan(const REAL *a, REAL *result);

/*!
 @function bigfloat_asin
 @brief Computes the arcsine of a big float.
 @discussion Calculates asin(a) and stores the result.
 @param a The value (must be in [-1, 1]).
 @param result Pointer to store the result in radians.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_asin(const REAL *a, REAL *result);

/*!
 @function bigfloat_acos
 @brief Computes the arccosine of a big float.
 @discussion Calculates acos(a) and stores the result.
 @param a The value (must be in [-1, 1]).
 @param result Pointer to store the result in radians.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_acos(const REAL *a, REAL *result);

/*!
 @function bigfloat_atan
 @brief Computes the arctangent of a big float.
 @discussion Calculates atan(a) and stores the result.
 @param a The value.
 @param result Pointer to store the result in radians.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_atan(const REAL *a, REAL *result);

/*!
 @function bigfloat_atan2
 @brief Computes the arctangent of y/x.
 @discussion Calculates atan2(y, x) and stores the result.
 @param y The y-coordinate.
 @param x The x-coordinate.
 @param result Pointer to store the result in radians.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_atan2(const REAL *y, const REAL *x, REAL *result);

// Hyperbolic functions

/*!
 @function bigfloat_sinh
 @brief Computes the hyperbolic sine of a big float.
 @discussion Calculates sinh(a) and stores the result.
 @param a The input.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_sinh(const REAL *a, REAL *result);

/*!
 @function bigfloat_cosh
 @brief Computes the hyperbolic cosine of a big float.
 @discussion Calculates cosh(a) and stores the result.
 @param a The input.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_cosh(const REAL *a, REAL *result);

/*!
 @function bigfloat_tanh
 @brief Computes the hyperbolic tangent of a big float.
 @discussion Calculates tanh(a) and stores the result.
 @param a The input.
 @param result Pointer to store the result.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_tanh(const REAL *a, REAL *result);

// Special functions

/*!
 @function bigfloat_sign
 @brief Returns the sign of a big float.
 @discussion Returns -1 for negative, 0 for zero, 1 for positive.
 @param a The input.
 @param result Pointer to store the sign.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_sign(const REAL *a, REAL *result);

/*!
 @function bigfloat_min
 @brief Returns the minimum of two big floats.
 @discussion Compares a and b and stores the smaller one in result.
 @param a The first value.
 @param b The second value.
 @param result Pointer to store the minimum.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_min(const REAL *a, const REAL *b, REAL *result);

/*!
 @function bigfloat_max
 @brief Returns the maximum of two big floats.
 @discussion Compares a and b and stores the larger one in result.
 @param a The first value.
 @param b The second value.
 @param result Pointer to store the maximum.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigfloat_max(const REAL *a, const REAL *b, REAL *result);

// Complex number functions

/*!
 @function bigcomplex_new
 @brief Creates a new big complex number initialized to zero.
 @discussion Allocates memory for a new COMPLEX structure and initializes it to 0+0i.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big complex, or NULL on failure.
 */
COMPLEX *bigcomplex_new(size_t precision);

/*!
 @function bigcomplex_new_from_floats
 @brief Creates a new big complex from real and imaginary big floats.
 @discussion Allocates a new COMPLEX with the given real and imaginary parts.
 @param real The real part.
 @param imag The imaginary part.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big complex, or NULL on failure.
 */
COMPLEX *bigcomplex_new_from_floats(const REAL *real, const REAL *imag, size_t precision);

/*!
 @function bigcomplex_new_from_doubles
 @brief Creates a new big complex from double values.
 @discussion Allocates a new COMPLEX with real and imaginary parts from doubles.
 @param real The real part as double.
 @param imag The imaginary part as double.
 @param precision The number of significant decimal digits.
 @return A pointer to the new big complex, or NULL on failure.
 */
COMPLEX *bigcomplex_new_from_doubles(double real, double imag, size_t precision);

/*!
 @function bigcomplex_new_from_string
 @brief Creates a new big complex from a string representation.
 @discussion Parses the input string and creates a big complex.
 @param str The string representation (e.g., "1+2i").
 @param precision The number of significant decimal digits.
 @return A pointer to the new big complex, or NULL on failure.
 */
COMPLEX *bigcomplex_new_from_string(const char *str, size_t precision);

/*!
 @function bigcomplex_new_copy
 @brief Creates a copy of an existing big complex.
 @discussion Allocates a new COMPLEX and copies all data from the source.
 @param src The big complex to copy.
 @return A pointer to the new copy, or NULL on failure.
 */
COMPLEX *bigcomplex_new_copy(const COMPLEX *src);

/*!
 @function bigcomplex_free
 @brief Frees the memory associated with a big complex.
 @discussion Releases all resources allocated for the big complex. The pointer becomes invalid after this call.
 @param complex The big complex to free.
 */
void bigcomplex_free(COMPLEX *complex);

/*!
 @function bigcomplex_add
 @brief Adds two big complex numbers.
 @discussion Computes a + b and stores the result in the provided buffer.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the sum.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigcomplex_add(const COMPLEX *a, const COMPLEX *b, COMPLEX *result);

/*!
 @function bigcomplex_subtract
 @brief Subtracts two big complex numbers.
 @discussion Computes a - b and stores the result in the provided buffer.
 @param a The minuend.
 @param b The subtrahend.
 @param result Pointer to store the difference.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigcomplex_subtract(const COMPLEX *a, const COMPLEX *b, COMPLEX *result);

/*!
 @function bigcomplex_multiply
 @brief Multiplies two big complex numbers.
 @discussion Computes a * b and stores the result in the provided buffer.
 @param a The first operand.
 @param b The second operand.
 @param result Pointer to store the product.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigcomplex_multiply(const COMPLEX *a, const COMPLEX *b, COMPLEX *result);

/*!
 @function bigcomplex_divide
 @brief Divides two big complex numbers.
 @discussion Computes a / b and stores the result in the provided buffer.
 @param a The dividend.
 @param b The divisor.
 @param result Pointer to store the quotient.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigcomplex_divide(const COMPLEX *a, const COMPLEX *b, COMPLEX *result);

/*!
 @function bigcomplex_conjugate
 @brief Computes the conjugate of a big complex number.
 @discussion Flips the sign of the imaginary part.
 @param a The input complex number.
 @param result Pointer to store the conjugate.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigcomplex_conjugate(const COMPLEX *a, COMPLEX *result);

/*!
 @function bigcomplex_absolute
 @brief Computes the absolute value (magnitude) of a big complex number.
 @discussion Calculates |a| and stores the result as a big float.
 @param a The input complex number.
 @param result Pointer to store the magnitude.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigcomplex_absolute(const COMPLEX *a, REAL *result);

/*!
 @function bigcomplex_equals
 @brief Checks if two big complex numbers are equal.
 @discussion Returns true if a == b, false otherwise.
 @param a The first complex number.
 @param b The second complex number.
 @return True if equal, false otherwise.
 */
bool bigcomplex_equals(const COMPLEX *a, const COMPLEX *b);

/*!
 @function bigcomplex_is_zero
 @brief Checks if a big complex number is zero.
 @discussion Returns true if both real and imaginary parts are zero.
 @param a The complex number to check.
 @return True if zero, false otherwise.
 */
bool bigcomplex_is_zero(const COMPLEX *a);

/*!
 @function bigcomplex_to_string
 @brief Converts a big complex number to a string representation.
 @discussion Returns a dynamically allocated string representing the complex number.
 @param complex The big complex to convert.
 @return A pointer to the string, or NULL on failure. The caller must free the string.
 */
char *bigcomplex_to_string(const COMPLEX *complex);

// Number theory functions

/*!
 @function bigint_gcd
 @brief Computes the greatest common divisor of two big integers.
 @discussion Uses the Euclidean algorithm to find GCD(a, b).
 @param a The first big integer.
 @param b The second big integer.
 @param result Pointer to store the GCD.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_gcd(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_lcm
 @brief Computes the least common multiple of two big integers.
 @discussion Calculates LCM(a, b) = |a*b| / GCD(a, b).
 @param a The first big integer.
 @param b The second big integer.
 @param result Pointer to store the LCM.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_lcm(const INT *a, const INT *b, INT *result);

/*!
 @function bigint_factorial
 @brief Computes the factorial of a big integer.
 @discussion Calculates a! for non-negative a.
 @param a The input (must be non-negative).
 @param result Pointer to store the factorial.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t bigint_factorial(const INT *a, INT *result);

/*!
 @function bigint_isprime
 @brief Checks if a big integer is prime.
 @discussion Returns true if a is a prime number, false otherwise.
 @param a The big integer to check (must be positive).
 @return True if prime, false otherwise.
 */
bool bigint_isprime(const INT *a);

/*!
 @function _bigint_to_bigfloat
 @brief Internal function to convert big integer to big float.
 @discussion Converts a big integer to a big float representation.
 @param bigint The big integer to convert.
 @param result Pointer to store the big float.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t _bigint_to_bigfloat(const INT *bigint, REAL *result);

/*!
 @function _bigfloat_to_bigint
 @brief Internal function to convert big float to big integer.
 @discussion Truncates a big float to a big integer.
 @param bigfloat The big float to convert.
 @param result Pointer to store the big integer.
 @return BIGNUM_OK on success, or an error code.
 */
bignum_err_t _bigfloat_to_bigint(const REAL *bigfloat, INT *result);

/*!
 @function bignum_error_string
 @brief Returns a string description of a bignum error code.
 @discussion Provides a human-readable string for the given error code.
 @param error The error code.
 @return A string describing the error.
 */
const char *bignum_error_string(bignum_err_t error);

#ifdef __cpluscplus
}
#endif
#endif // PAUL_BIGNUM_H

#if defined(PAUL_IMPLEMENTATION) || defined(PAUL_BIGNUM_IMPLEMENTATION)
// Parameter validation macros
#define BIGNUM_VALIDATE_PARAMS_3(a, b, result) \
    if (!a || !b || !result)                   \
        return BIGNUM_ERROR_INVALID_ARGS;

#define BIGNUM_VALIDATE_PARAMS_2(a, result) \
    if (!a || !result)                      \
        return BIGNUM_ERROR_INVALID_ARGS;

#define BIGNUM_VALIDATE_PARAMS_1(a) \
    if (!a)                         \
        return BIGNUM_ERROR_INVALID_ARGS;

#define BIGNUM_VALIDATE_PARAMS_BITWISE(a, b, result)       \
    if (!a || !b || !result || a->negative || b->negative) \
        return BIGNUM_ERROR_INVALID_ARGS;

bignum_err_t bigint_ensure_capacity(INT *bigint, size_t required_capacity) {
    if (bigint->capacity >= required_capacity)
        return BIGNUM_OK;

    size_t new_capacity = bigint->capacity == 0 ? 8 : bigint->capacity;
    while (new_capacity < required_capacity)
        new_capacity *= 2;

    uint32_t *new_digits = realloc(bigint->digits, new_capacity * sizeof(uint32_t));
    if (!new_digits)
        return BIGNUM_ERROR_MEMORY;

    bigint->digits = new_digits;
    bigint->capacity = new_capacity;
    return BIGNUM_OK;
}

static void bigint_normalize(INT *bigint) {
    while (bigint->length > 1 && bigint->digits[bigint->length - 1] == 0)
        bigint->length--;

    if (bigint->length == 1 && bigint->digits[0] == 0)
        bigint->negative = false;
}

INT *bigint_create_power(int64_t base, size_t exponent) {
    INT *result = bigint_new_from_int64(1);
    if (!result)
        return NULL;

    INT *base_int = bigint_new_from_int64(base);
    if (!base_int) {
        bigint_free(result);
        return NULL;
    }

    for (size_t i = 0; i < exponent; i++) {
        bignum_err_t err = bigint_multiply(result, base_int, result);
        if (err != BIGNUM_OK) {
            bigint_free(result);
            bigint_free(base_int);
            return NULL;
        }
    }

    bigint_free(base_int);
    return result;
}

static int bigint_compare_magnitude(const INT *a, const INT *b) {
    if (a->length != b->length)
        return a->length > b->length ? 1 : -1;

    for (size_t i = a->length; i > 0; i--)
        if (a->digits[i - 1] != b->digits[i - 1])
            return a->digits[i - 1] > b->digits[i - 1] ? 1 : -1;

    return 0;
}

static bignum_err_t bigint_add_magnitude(const INT *a, const INT *b, INT *result) {
    size_t max_len = a->length > b->length ? a->length : b->length;
    bignum_err_t err = bigint_ensure_capacity(result, max_len + 1);
    if (err != BIGNUM_OK)
        return err;

    uint64_t carry = 0;
    size_t i;

    for (i = 0; i < max_len; i++) {
        uint64_t sum = carry;
        if (i < a->length)
            sum += a->digits[i];
        if (i < b->length)
            sum += b->digits[i];

        result->digits[i] = sum % BIGNUM_BASE;
        carry = sum / BIGNUM_BASE;
    }

    if (carry > 0)
        result->digits[i++] = carry;

    result->length = i;
    return BIGNUM_OK;
}

static bignum_err_t bigint_subtract_magnitude(const INT *a, const INT *b, INT *result) {
    // Assumes |a| >= |b|
    bignum_err_t err = bigint_ensure_capacity(result, a->length);
    if (err != BIGNUM_OK)
        return err;

    int64_t borrow = 0;

    for (size_t i = 0; i < a->length; i++) {
        int64_t diff = (int64_t)a->digits[i] - borrow;
        if (i < b->length)
            diff -= b->digits[i];

        if (diff < 0) {
            diff += BIGNUM_BASE;
            borrow = 1;
        }
        else
            borrow = 0;

        result->digits[i] = diff;
    }

    result->length = a->length;
    bigint_normalize(result);
    return BIGNUM_OK;
}

INT *bigint_new(void) {
    INT *bigint = malloc(sizeof(INT));
    if (!bigint)
        return NULL;

    bigint->digits = NULL;
    bigint->length = 0;
    bigint->capacity = 0;
    bigint->negative = false;

    // Initialize with zero
    if (bigint_ensure_capacity(bigint, 1) != BIGNUM_OK) {
        free(bigint);
        return NULL;
    }

    bigint->digits[0] = 0;
    bigint->length = 1;
    /* Sanity check: ensure representation is valid */
    if (!bigint->digits || bigint->length == 0) {
        fprintf(stderr, "ASSERT: bigint_new produced invalid bigint (digits=%p length=%zu)\n", (void *)bigint->digits, bigint->length);
        abort();
    }
    return bigint;
}

INT *bigint_new_from_string(const char *str) {
    INT *bigint = bigint_new();
    if (!bigint)
        return NULL;

    if (bigint_set_from_string(bigint, str) != BIGNUM_OK) {
        bigint_free(bigint);
        return NULL;
    }

    return bigint;
}

INT *bigint_new_from_int64(int64_t value) {
    INT *bigint = bigint_new();
    if (!bigint)
        return NULL;

    if (bigint_set_from_int64(bigint, value) != BIGNUM_OK) {
        bigint_free(bigint);
        return NULL;
    }

    return bigint;
}

INT *bigint_new_copy(const INT *src) {
    if (!src)
        return NULL;

    INT *bigint = malloc(sizeof(INT));
    if (!bigint)
        return NULL;
    // Defensive: ensure at least capacity 1
    size_t cap = src->capacity == 0 ? 1 : src->capacity;
    bigint->digits = malloc(cap * sizeof(uint32_t));
    if (!bigint->digits) {
        free(bigint);
        return NULL;
    }

    // If source length is zero, normalize to represent zero
    if (src->length == 0) {
        bigint->digits[0] = 0;
        bigint->length = 1;
    } else {
        memcpy(bigint->digits, src->digits, src->length * sizeof(uint32_t));
        bigint->length = src->length;
    }

    bigint->capacity = cap;
    bigint->negative = src->negative;

    /* Sanity check: ensure copy has valid representation */
    if (!bigint->digits || bigint->length == 0) {
        fprintf(stderr, "ASSERT: bigint_new_copy produced invalid bigint (src=%p digits=%p length=%zu)\n", (void *)src, (void *)bigint->digits, bigint->length);
        abort();
    }

    return bigint;
}

void bigint_free(INT *bigint) {
    if (!bigint)
        return;

    free(bigint->digits);
    free(bigint);
}

bignum_err_t bigint_set_from_string(INT *bigint, const char *str) {
    if (!str || !bigint)
        return BIGNUM_ERROR_INVALID_ARGS;

    while (*str == ' ' || *str == '\t')
        str++;

    if (*str == '\0')
        return BIGNUM_ERROR_INVALID_ARGS;

    bool negative = false;
    if (*str == '-') {
        negative = true;
        str++;
    }
    else if (*str == '+')
        str++;

    while (*str == '0')
        str++;

    if (*str == '\0') {
        // Number is zero
        bigint->length = 1;
        bigint->digits[0] = 0;
        bigint->negative = false;
        return BIGNUM_OK;
    }

    size_t digit_count = 0;
    for (const char *p = str; *p >= '0' && *p <= '9'; p++)
        digit_count++;

    if (digit_count == 0)
        return BIGNUM_ERROR_INVALID_ARGS;

    // Calculate required capacity (9 decimal digits per base digit)
    size_t required_capacity = (digit_count + 8) / 9;
    bignum_err_t err = bigint_ensure_capacity(bigint, required_capacity);
    if (err != BIGNUM_OK)
        return err;

    bigint->length = 0;
    bigint->negative = negative;

    // Process digits in groups of 9 from right to left
    const char *end = str + digit_count;
    while (end > str) {
        const char *start = end - 9;
        if (start < str)
            start = str;

        uint32_t digit = 0;
        uint32_t multiplier = 1;

        for (const char *p = end - 1; p >= start; p--) {
            if (*p < '0' || *p > '9')
                return BIGNUM_ERROR_INVALID_ARGS;

            digit += (*p - '0') * multiplier;
            multiplier *= 10;
        }

        bigint->digits[bigint->length++] = digit;
        end = start;
    }

    bigint_normalize(bigint);
    return BIGNUM_OK;
}

bignum_err_t bigint_set_from_int64(INT *bigint, int64_t value) {
    if (!bigint)
        return BIGNUM_ERROR_INVALID_ARGS;

    bigint->negative = value < 0;
    uint64_t abs_value = bigint->negative ? -(uint64_t)value : (uint64_t)value;

    if (abs_value == 0) {
        bigint->length = 1;
        bigint->digits[0] = 0;
        bigint->negative = false;
        /* Sanity check: ensure representation is valid after set */
        if (!bigint->digits || bigint->length == 0) {
            fprintf(stderr, "ASSERT: bigint_set_from_int64 produced invalid bigint (digits=%p length=%zu)\n", (void *)bigint->digits, bigint->length);
            abort();
        }
        return BIGNUM_OK;
    }

    size_t required_capacity = 0;
    uint64_t temp = abs_value;
    while (temp > 0) {
        temp /= BIGNUM_BASE;
        required_capacity++;
    }

    bignum_err_t err = bigint_ensure_capacity(bigint, required_capacity);
    if (err != BIGNUM_OK)
        return err;

    bigint->length = 0;
    while (abs_value > 0) {
        bigint->digits[bigint->length++] = abs_value % BIGNUM_BASE;
        abs_value /= BIGNUM_BASE;
    }

    return BIGNUM_OK;
}

char *bigint_to_string(const INT *bigint) {
    if (!bigint)
        return NULL;

    // Defensive: if length is zero treat as zero rather than returning NULL
    if (bigint->length == 0) {
        char *result = malloc(2);
        if (!result)
            return NULL;
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    if (bigint_is_zero(bigint)) {
        char *result = malloc(2 * sizeof(char));
        if (result) {
            result[0] = '0';
            result[1] = '\0';
        }
        return result;
    }

    // Calculate maximum string length
    size_t max_len = bigint->length * 9 + 2; // +1 for sign, +1 for null terminator
    char *result = malloc(max_len * sizeof(char));
    if (!result)
        return NULL;

    char *p = result;
    if (bigint->negative)
        *p++ = '-';

    // Convert most significant digit
    p += sprintf(p, "%u", bigint->digits[bigint->length - 1]);

    // Convert remaining digits with leading zeros
    for (size_t i = bigint->length - 1; i > 0; i--)
        p += sprintf(p, "%09u", bigint->digits[i - 1]);

    return result;
}

char *bigint_to_string_base(const INT *bigint, int base) {
    if (!bigint || bigint->length == 0 || base < 2 || base > 36)
        return NULL;

    if (bigint_is_zero(bigint)) {
        char *result = malloc(2 * sizeof(char));
        if (result) {
            result[0] = '0';
            result[1] = '\0';
        }
        return result;
    }

    if (base == 10)
        return bigint_to_string(bigint);

    INT *temp = bigint_new_copy(bigint);
    if (!temp)
        return NULL;

    bool was_negative = temp->negative;
    temp->negative = false;

    // Estimate maximum string length (base 2 is worst case)
    size_t max_len = bigint->length * 32 + 2; // Conservative estimate
    char *result = malloc(max_len * sizeof(char));
    if (!result) {
        bigint_free(temp);
        return NULL;
    }

    char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *p = result + max_len - 1;
    *p = '\0';

    // Convert by repeatedly dividing by base
    INT *base_int = bigint_new();
    INT *quotient = bigint_new();
    INT *remainder = bigint_new();

    if (!base_int || !quotient || !remainder) {
        bigint_free(temp);
        bigint_free(base_int);
        bigint_free(quotient);
        bigint_free(remainder);
        free(result);
        return NULL;
    }

    bigint_set_from_int64(base_int, base);

    while (!bigint_is_zero(temp)) {
        if (bigint_divide(temp, base_int, quotient, remainder) != BIGNUM_OK)
            break;

        int64_t rem_val = bigint_to_int64(remainder);
        *(--p) = digits[rem_val];

        // Copy quotient back to temp for next iteration
        bigint_free(temp);
        if (!(temp = bigint_new_copy(quotient)))
            break;
    }

    if (was_negative)
        *(--p) = '-';

    size_t len = strlen(p);
    memmove(result, p, len + 1);

    bigint_free(temp);
    bigint_free(base_int);
    bigint_free(quotient);
    bigint_free(remainder);

    return result;
}

int64_t bigint_to_int64(const INT *bigint) {
    if (!bigint || bigint_is_zero(bigint))
        return 0;

    uint64_t result = 0;
    uint64_t multiplier = 1;

    for (size_t i = 0; i < bigint->length && i < 3; i++) { // Limit to avoid overflow
        result += (uint64_t)bigint->digits[i] * multiplier;
        multiplier *= BIGNUM_BASE;

        // Check for overflow
        if (result > INT64_MAX)
            return bigint->negative ? INT64_MIN : INT64_MAX;
    }

    return bigint->negative ? -(int64_t)result : (int64_t)result;
}

int bigint_compare(const INT *a, const INT *b) {
    if (!a || !b)
        return 0;

    if (a->negative != b->negative)
        return a->negative ? -1 : 1;

    int mag_cmp = bigint_compare_magnitude(a, b);
    return a->negative ? -mag_cmp : mag_cmp;
}

bool bigint_equals(const INT *a, const INT *b) {
    return bigint_compare(a, b) == 0;
}

bool bigint_less_than(const INT *a, const INT *b) {
    return bigint_compare(a, b) < 0;
}

bool bigint_greater_than(const INT *a, const INT *b) {
    return bigint_compare(a, b) > 0;
}

bool bigint_is_zero(const INT *a) {
    return a && a->length == 1 && a->digits[0] == 0;
}

bool bigint_is_negative(const INT *a) {
    return a && a->negative && !bigint_is_zero(a);
}

bignum_err_t bigint_add(const INT *a, const INT *b, INT *result) {
    BIGNUM_VALIDATE_PARAMS_3(a, b, result);

    if (a->negative == b->negative) {
        // Same sign: add magnitudes
        bignum_err_t err = bigint_add_magnitude(a, b, result);
        if (err != BIGNUM_OK)
            return err;
        result->negative = a->negative;
    } else {
        // Different signs: subtract magnitudes
        int mag_cmp = bigint_compare_magnitude(a, b);
        if (mag_cmp == 0) {
            // Result is zero
            result->length = 1;
            result->digits[0] = 0;
            result->negative = false;
        }
        else if (mag_cmp > 0) {
            // |a| > |b|
            bignum_err_t err = bigint_subtract_magnitude(a, b, result);
            if (err != BIGNUM_OK)
                return err;
            result->negative = a->negative;
        } else {
            // |a| < |b|
            bignum_err_t err = bigint_subtract_magnitude(b, a, result);
            if (err != BIGNUM_OK)
                return err;
            result->negative = b->negative;
        }
    }

    bigint_normalize(result);
    return BIGNUM_OK;
}

bignum_err_t bigint_subtract(const INT *a, const INT *b, INT *result) {
    BIGNUM_VALIDATE_PARAMS_3(a, b, result);

    if (a->negative != b->negative) {
        // Different signs: add magnitudes
        bignum_err_t err = bigint_add_magnitude(a, b, result);
        if (err != BIGNUM_OK)
            return err;
        result->negative = a->negative;
    } else {
        // Same sign: subtract magnitudes
        int mag_cmp = bigint_compare_magnitude(a, b);
        if (mag_cmp == 0) {
            // Result is zero
            result->length = 1;
            result->digits[0] = 0;
            result->negative = false;
        }
        else if (mag_cmp > 0) {
            // |a| > |b|
            bignum_err_t err = bigint_subtract_magnitude(a, b, result);
            if (err != BIGNUM_OK)
                return err;
            result->negative = a->negative;
        } else {
            // |a| < |b|
            bignum_err_t err = bigint_subtract_magnitude(b, a, result);
            if (err != BIGNUM_OK)
                return err;
            result->negative = !a->negative;
        }
    }

    bigint_normalize(result);
    return BIGNUM_OK;
}

bignum_err_t bigint_multiply(const INT *a, const INT *b, INT *result) {
    BIGNUM_VALIDATE_PARAMS_3(a, b, result);

    if (bigint_is_zero(a) || bigint_is_zero(b)) {
        result->length = 1;
        result->digits[0] = 0;
        result->negative = false;
        return BIGNUM_OK;
    }

    size_t result_len = a->length + b->length;
    bignum_err_t err = bigint_ensure_capacity(result, result_len);
    if (err != BIGNUM_OK)
        return err;

    for (size_t i = 0; i < result_len; i++)
        result->digits[i] = 0;

    for (size_t i2 = 0; i2 < a->length; i2++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->length; j++) {
            uint64_t product = (uint64_t)a->digits[i2] * b->digits[j] + result->digits[i2 + j] + carry;
            result->digits[i2 + j] = product % BIGNUM_BASE;
            carry = product / BIGNUM_BASE;
        }
        if (carry > 0)
            result->digits[i2 + b->length] = carry;
    }

    result->length = result_len;
    result->negative = a->negative != b->negative;
    bigint_normalize(result);
    return BIGNUM_OK;
}

bignum_err_t bigint_divide(const INT *a, const INT *b, INT *quotient, INT *remainder) {
    if (!a || !b || !quotient)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (bigint_is_zero(b))
        return BIGNUM_ERROR_DIVIDE_BY_ZERO;

    if (bigint_is_zero(a)) {
        quotient->length = 1;
        quotient->digits[0] = 0;
        quotient->negative = false;
        if (remainder) {
            remainder->length = 1;
            remainder->digits[0] = 0;
            remainder->negative = false;
        }
        return BIGNUM_OK;
    }

    INT *abs_a = bigint_new_copy(a);
    INT *abs_b = bigint_new_copy(b);
    if (!abs_a || !abs_b) {
        bigint_free(abs_a);
        bigint_free(abs_b);
        return BIGNUM_ERROR_MEMORY;
    }

    abs_a->negative = false;
    abs_b->negative = false;

    if (bigint_compare_magnitude(abs_a, abs_b) < 0) {
        // |a| < |b|, so quotient is 0 and remainder is a
        quotient->length = 1;
        quotient->digits[0] = 0;
        quotient->negative = false;

        if (remainder) {
            bignum_err_t err = bigint_ensure_capacity(remainder, a->length);
            if (err != BIGNUM_OK) {
                bigint_free(abs_a);
                bigint_free(abs_b);
                return err;
            }
            memcpy(remainder->digits, a->digits, a->length * sizeof(uint32_t));
            remainder->length = a->length;
            remainder->negative = a->negative;
        }

        bigint_free(abs_a);
        bigint_free(abs_b);
        return BIGNUM_OK;
    }

    INT *temp_quotient = bigint_new();
    INT *temp_remainder = bigint_new_copy(abs_a);
    if (!temp_quotient || !temp_remainder) {
        bigint_free(abs_a);
        bigint_free(abs_b);
        bigint_free(temp_quotient);
        bigint_free(temp_remainder);
        return BIGNUM_ERROR_MEMORY;
    }

    while (bigint_compare_magnitude(temp_remainder, abs_b) >= 0) {
        bigint_subtract_magnitude(temp_remainder, abs_b, temp_remainder);

        // Increment quotient
        INT *one = bigint_new_from_int64(1);
        if (!one) {
            bigint_free(abs_a);
            bigint_free(abs_b);
            bigint_free(temp_quotient);
            bigint_free(temp_remainder);
            return BIGNUM_ERROR_MEMORY;
        }
        bigint_add_magnitude(temp_quotient, one, temp_quotient);
        bigint_free(one);
    }

    temp_quotient->negative = a->negative != b->negative;
    temp_remainder->negative = a->negative;

    bignum_err_t err = bigint_ensure_capacity(quotient, temp_quotient->length);
    if (err == BIGNUM_OK) {
        memcpy(quotient->digits, temp_quotient->digits, temp_quotient->length * sizeof(uint32_t));
        quotient->length = temp_quotient->length;
        quotient->negative = temp_quotient->negative;

        if (remainder)
            if ((err = bigint_ensure_capacity(remainder, temp_remainder->length)) == BIGNUM_OK) {
                memcpy(remainder->digits, temp_remainder->digits, temp_remainder->length * sizeof(uint32_t));
                remainder->length = temp_remainder->length;
                remainder->negative = temp_remainder->negative;
            }
    }

    bigint_free(abs_a);
    bigint_free(abs_b);
    bigint_free(temp_quotient);
    bigint_free(temp_remainder);

    bigint_normalize(quotient);
    if (remainder)
        bigint_normalize(remainder);

    return err;
}

bignum_err_t bigint_modulo(const INT *a, const INT *b, INT *result) {
    INT *quotient = bigint_new();
    if (!quotient)
        return BIGNUM_ERROR_MEMORY;

    bignum_err_t error = bigint_divide(a, b, quotient, result);
    bigint_free(quotient);
    return error;
}

bignum_err_t bigint_power(const INT *base, const INT *exponent, INT *result) {
    BIGNUM_VALIDATE_PARAMS_3(base, exponent, result);

    if (bigint_is_negative(exponent))
        return BIGNUM_ERROR_INVALID_ARGS; // Only support non-negative exponents

    // base^0 = 1
    if (bigint_is_zero(exponent))
        return bigint_set_from_int64(result, 1);

    // 0^n = 0 (for n > 0)
    if (bigint_is_zero(base))
        return bigint_set_from_int64(result, 0);

    // For small exponents, use simple multiplication (more reliable)
    int64_t exp_value = bigint_to_int64(exponent);

    if (exp_value >= 0 && exp_value <= 1000) { // Reasonable limit for iterative approach
        bignum_err_t err = bigint_set_from_int64(result, 1);
        if (err != BIGNUM_OK)
            return err;

        INT *temp_base = bigint_new_copy(base);
        if (!temp_base)
            return BIGNUM_ERROR_MEMORY;

        for (int64_t i = 0; i < exp_value; i++) {
            INT *temp_result = bigint_new();
            if (!temp_result) {
                bigint_free(temp_base);
                return BIGNUM_ERROR_MEMORY;
            }

            err = bigint_multiply(result, temp_base, temp_result);
            if (err != BIGNUM_OK) {
                bigint_free(temp_result);
                bigint_free(temp_base);
                return err;
            }

            err = bigint_ensure_capacity(result, temp_result->length);
            if (err != BIGNUM_OK) {
                bigint_free(temp_result);
                bigint_free(temp_base);
                return err;
            }

            memcpy(result->digits, temp_result->digits, temp_result->length * sizeof(uint32_t));
            result->length = temp_result->length;
            result->negative = temp_result->negative;

            bigint_free(temp_result);
        }

        bigint_free(temp_base);
        return BIGNUM_OK;
    }

    // For larger exponents, use exponentiation by squaring
    INT *temp_base = bigint_new_copy(base);
    INT *temp_exp = bigint_new_copy(exponent);
    INT *temp_result = bigint_new_from_int64(1);

    if (!temp_base || !temp_exp || !temp_result) {
        bigint_free(temp_base);
        bigint_free(temp_exp);
        bigint_free(temp_result);
        return BIGNUM_ERROR_MEMORY;
    }

    bignum_err_t err = BIGNUM_OK;
    INT *two = bigint_new_from_int64(2);
    if (!two) {
        bigint_free(temp_base);
        bigint_free(temp_exp);
        bigint_free(temp_result);
        return BIGNUM_ERROR_MEMORY;
    }

    while (!bigint_is_zero(temp_exp) && err == BIGNUM_OK) {
        // Check if exponent is odd
        INT *quotient = bigint_new();
        INT *remainder = bigint_new();

        if (!quotient || !remainder) {
            bigint_free(quotient);
            bigint_free(remainder);
            err = BIGNUM_ERROR_MEMORY;
            break;
        }

        if ((err = bigint_divide(temp_exp, two, quotient, remainder)) != BIGNUM_OK) {
            bigint_free(quotient);
            bigint_free(remainder);
            break;
        }

        bool is_odd = !bigint_is_zero(remainder);
        if (is_odd)
            if ((err = bigint_multiply(temp_result, temp_base, temp_result)) != BIGNUM_OK) {
                bigint_free(quotient);
                bigint_free(remainder);
                break;
            }

        if ((err = bigint_multiply(temp_base, temp_base, temp_base)) != BIGNUM_OK) {
            bigint_free(quotient);
            bigint_free(remainder);
            break;
        }

        // Set temp_exp = quotient (dividing by 2)
        bigint_free(temp_exp);
        temp_exp = quotient;
        bigint_free(remainder);
    }

    bigint_free(two);

    if (err == BIGNUM_OK)
        if ((err = bigint_ensure_capacity(result, temp_result->length)) == BIGNUM_OK) {
            memcpy(result->digits, temp_result->digits, temp_result->length * sizeof(uint32_t));
            result->length = temp_result->length;
            result->negative = temp_result->negative;
        }

    bigint_free(temp_base);
    bigint_free(temp_exp);
    bigint_free(temp_result);

    return err;
}

bignum_err_t bigint_and(const INT *a, const INT *b, INT *result) {
    BIGNUM_VALIDATE_PARAMS_BITWISE(a, b, result);

    size_t min_len = a->length < b->length ? a->length : b->length;
    bignum_err_t err = bigint_ensure_capacity(result, min_len);
    if (err != BIGNUM_OK)
        return err;

    for (size_t i = 0; i < min_len; i++)
        result->digits[i] = a->digits[i] & b->digits[i];

    result->length = min_len;
    result->negative = false;
    bigint_normalize(result);
    return BIGNUM_OK;
}

bignum_err_t bigint_or(const INT *a, const INT *b, INT *result) {
    BIGNUM_VALIDATE_PARAMS_BITWISE(a, b, result);

    size_t max_len = a->length > b->length ? a->length : b->length;
    bignum_err_t err = bigint_ensure_capacity(result, max_len);
    if (err != BIGNUM_OK)
        return err;

    for (size_t i = 0; i < max_len; i++) {
        uint32_t a_digit = i < a->length ? a->digits[i] : 0;
        uint32_t b_digit = i < b->length ? b->digits[i] : 0;
        result->digits[i] = a_digit | b_digit;
    }

    result->length = max_len;
    result->negative = false;
    bigint_normalize(result);
    return BIGNUM_OK;
}

bignum_err_t bigint_xor(const INT *a, const INT *b, INT *result) {
    BIGNUM_VALIDATE_PARAMS_BITWISE(a, b, result);

    size_t max_len = a->length > b->length ? a->length : b->length;
    bignum_err_t err = bigint_ensure_capacity(result, max_len);
    if (err != BIGNUM_OK)
        return err;

    for (size_t i = 0; i < max_len; i++) {
        uint32_t a_digit = i < a->length ? a->digits[i] : 0;
        uint32_t b_digit = i < b->length ? b->digits[i] : 0;
        result->digits[i] = a_digit ^ b_digit;
    }

    result->length = max_len;
    result->negative = false;
    bigint_normalize(result);
    return BIGNUM_OK;
}

bignum_err_t bigint_not(const INT *a, INT *result) {
    if (!a || !result || a->negative)
        return BIGNUM_ERROR_INVALID_ARGS;

    bignum_err_t err = bigint_ensure_capacity(result, a->length);
    if (err != BIGNUM_OK)
        return err;

    for (size_t i = 0; i < a->length; i++)
        result->digits[i] = (~a->digits[i]) & (BIGNUM_BASE - 1);

    result->length = a->length;
    result->negative = false;
    bigint_normalize(result);
    return BIGNUM_OK;
}

bignum_err_t bigint_shift_left(const INT *a, size_t bits, INT *result) {
    BIGNUM_VALIDATE_PARAMS_2(a, result);

    if (bits == 0 || bigint_is_zero(a)) {
        if (result != a) {
            bignum_err_t err = bigint_ensure_capacity(result, a->length);
            if (err != BIGNUM_OK)
                return err;
            memcpy(result->digits, a->digits, a->length * sizeof(uint32_t));
            result->length = a->length;
            result->negative = a->negative;
        }
        return BIGNUM_OK;
    }

    // Multiply by 2^bits
    INT *power_of_two = bigint_create_power(2, bits);
    if (!power_of_two)
        return BIGNUM_ERROR_MEMORY;

    bignum_err_t err = bigint_multiply(a, power_of_two, result);
    bigint_free(power_of_two);
    return err;
}

bignum_err_t bigint_shift_right(const INT *a, size_t bits, INT *result) {
    BIGNUM_VALIDATE_PARAMS_2(a, result);

    if (bits == 0 || bigint_is_zero(a)) {
        if (result != a) {
            bignum_err_t err = bigint_ensure_capacity(result, a->length);
            if (err != BIGNUM_OK)
                return err;
            memcpy(result->digits, a->digits, a->length * sizeof(uint32_t));
            result->length = a->length;
            result->negative = a->negative;
        }
        return BIGNUM_OK;
    }

    // Divide by 2^bits
    INT *power_of_two = bigint_create_power(2, bits);
    if (!power_of_two)
        return BIGNUM_ERROR_MEMORY;

    bignum_err_t err = bigint_divide(a, power_of_two, result, NULL);
    bigint_free(power_of_two);
    return err;
}

bignum_err_t bigint_absolute(const INT *a, INT *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (result != a) {
        bignum_err_t err = bigint_ensure_capacity(result, a->length);
        if (err != BIGNUM_OK)
            return err;

        memcpy(result->digits, a->digits, a->length * sizeof(uint32_t));
        result->length = a->length;
    }

    result->negative = false;
    return BIGNUM_OK;
}

bignum_err_t bigint_negate(INT *a) {
    if (!a)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (!bigint_is_zero(a))
        a->negative = !a->negative;

    return BIGNUM_OK;
}

bignum_err_t bigint_gcd(const INT *a, const INT *b, INT *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    INT *temp_a = bigint_new_copy(a);
    INT *temp_b = bigint_new_copy(b);
    if (!temp_a || !temp_b) {
        if (temp_a)
            bigint_free(temp_a);
        if (temp_b)
            bigint_free(temp_b);
        return BIGNUM_ERROR_MEMORY;
    }

    temp_a->negative = false;
    temp_b->negative = false;

    // Euclidean algorithm
    while (!bigint_is_zero(temp_b)) {
        INT *remainder = bigint_new();
        if (!remainder) {
            bigint_free(temp_a);
            bigint_free(temp_b);
            return BIGNUM_ERROR_MEMORY;
        }

        INT *quotient = bigint_new();
        if (!quotient) {
            bigint_free(temp_a);
            bigint_free(temp_b);
            bigint_free(remainder);
            return BIGNUM_ERROR_MEMORY;
        }

        bignum_err_t err = bigint_divide(temp_a, temp_b, quotient, remainder);
        if (err != BIGNUM_OK) {
            bigint_free(temp_a);
            bigint_free(temp_b);
            bigint_free(remainder);
            bigint_free(quotient);
            return err;
        }

        bigint_free(quotient);
        bigint_free(temp_a);
        temp_a = temp_b;
        temp_b = remainder;
    }

    bignum_err_t err = bigint_ensure_capacity(result, temp_a->length);
    if (err == BIGNUM_OK) {
        memcpy(result->digits, temp_a->digits, temp_a->length * sizeof(uint32_t));
        result->length = temp_a->length;
        result->negative = false;
    }

    bigint_free(temp_a);
    bigint_free(temp_b);
    return err;
}

bignum_err_t bigint_lcm(const INT *a, const INT *b, INT *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    // LCM(a,b) = |a*b| / GCD(a,b)
    INT *gcd = bigint_new();
    INT *product = bigint_new();
    if (!gcd || !product) {
        if (gcd)
            bigint_free(gcd);
        if (product)
            bigint_free(product);
        return BIGNUM_ERROR_MEMORY;
    }

    bignum_err_t err = bigint_gcd(a, b, gcd);
    if (err != BIGNUM_OK) {
        bigint_free(gcd);
        bigint_free(product);
        return err;
    }

    err = bigint_multiply(a, b, product);
    if (err != BIGNUM_OK) {
        bigint_free(gcd);
        bigint_free(product);
        return err;
    }

    INT *remainder = bigint_new();
    if (!remainder) {
        bigint_free(gcd);
        bigint_free(product);
        return BIGNUM_ERROR_MEMORY;
    }

    err = bigint_divide(product, gcd, result, remainder);

    bigint_free(gcd);
    bigint_free(product);
    bigint_free(remainder);

    if (err == BIGNUM_OK)
        result->negative = false;

    return err;
}

bignum_err_t bigint_factorial(const INT *a, INT *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (bigint_is_negative(a))
        return BIGNUM_ERROR_INVALID_ARGS;

    int64_t n = bigint_to_int64(a);
    if (n < 0 || n > 100000)
        return BIGNUM_ERROR_OVERFLOW; // Prevent huge factorials

    bignum_err_t err = bigint_set_from_int64(result, 1);
    if (err != BIGNUM_OK)
        return err;

    for (int64_t i = 2; i <= n; i++) {
        INT *temp = bigint_new_from_int64(i);
        if (!temp)
            return BIGNUM_ERROR_MEMORY;

        err = bigint_multiply(result, temp, result);
        bigint_free(temp);

        if (err != BIGNUM_OK)
            return err;
    }

    return BIGNUM_OK;
}

bool bigint_isprime(const INT *a) {
    if (!a)
        return false;

    if (bigint_is_negative(a) || bigint_is_zero(a))
        return false;

    int64_t n = bigint_to_int64(a);
    if (n == 1)
        return false;
    if (n == 2)
        return true;
    if (n % 2 == 0)
        return false;

    // Simple trial division for now
    for (int64_t i = 3; i * i <= n; i += 2)
        if (n % i == 0)
            return false;

    return true;
}

REAL *bigfloat_new(size_t precision) {
    REAL *bigfloat = malloc(sizeof(REAL));
    if (!bigfloat)
        return NULL;

    INT *zero = bigint_new();
    if (!zero) {
        free(bigfloat);
        return NULL;
    }

    bigfloat->mantissa = *zero;
    free(zero);

    bigfloat->exponent = 0;
    bigfloat->precision = precision > 0 ? precision : BIGNUM_DEFAULT_PRECISION;
    bigfloat->negative = false;

    return bigfloat;
}

REAL *bigfloat_new_from_string(const char *str, size_t precision) {
    if (!str)
        return NULL;

    REAL *bigfloat = bigfloat_new(precision);
    if (!bigfloat)
        return NULL;

    if (bigfloat_set_from_string(bigfloat, str) != BIGNUM_OK) {
        bigfloat_free(bigfloat);
        return NULL;
    }

    return bigfloat;
}

REAL *bigfloat_new_from_double(double value, size_t precision) {
    REAL *bigfloat = bigfloat_new(precision);
    if (!bigfloat)
        return NULL;

    if (bigfloat_set_from_double(bigfloat, value) != BIGNUM_OK) {
        bigfloat_free(bigfloat);
        return NULL;
    }

    return bigfloat;
}

REAL *bigfloat_new_from_bigint(const INT *bigint, size_t precision) {
    if (!bigint)
        return NULL;

    REAL *bigfloat = bigfloat_new(precision);
    if (!bigfloat)
        return NULL;

    if (bigfloat_set_from_bigint(bigfloat, bigint) != BIGNUM_OK) {
        bigfloat_free(bigfloat);
        return NULL;
    }

    return bigfloat;
}

REAL *bigfloat_new_copy(const REAL *src) {
    if (!src)
        return NULL;

    REAL *bigfloat = malloc(sizeof(REAL));
    if (!bigfloat)
        return NULL;

    INT *mantissa_copy = bigint_new_copy(&src->mantissa);
    if (!mantissa_copy) {
        free(bigfloat);
        return NULL;
    }

    bigfloat->mantissa = *mantissa_copy;
    free(mantissa_copy);

    bigfloat->exponent = src->exponent;
    bigfloat->precision = src->precision;
    bigfloat->negative = src->negative;

    return bigfloat;
}

void bigfloat_free(REAL *bigfloat) {
    if (!bigfloat)
        return;

    free(bigfloat->mantissa.digits);
    free(bigfloat);
}

bignum_err_t bigfloat_set_from_string(REAL *bigfloat, const char *str) {
    if (!bigfloat || !str)
        return BIGNUM_ERROR_INVALID_ARGS;

    while (*str == ' ' || *str == '\t')
        str++;

    if (*str == '\0')
        return BIGNUM_ERROR_INVALID_ARGS;

    bool negative = false;
    if (*str == '-') {
        negative = true;
        str++;
    }
    else if (*str == '+')
        str++;

    const char *decimal_point = strchr(str, '.');
    const char *exponent_char = strchr(str, 'e');
    if (!exponent_char)
        exponent_char = strchr(str, 'E');

    char mantissa_str[1024] = {0};
    int32_t decimal_places = 0;

    if (decimal_point) {
        size_t int_len = decimal_point - str;
        if (int_len >= sizeof(mantissa_str) - 1)
            return BIGNUM_ERROR_INVALID_ARGS;
        strncpy(mantissa_str, str, int_len);

        const char *frac_start = decimal_point + 1;
        const char *frac_end = exponent_char ? exponent_char : str + strlen(str);
        size_t frac_len = frac_end - frac_start;

        if (int_len + frac_len >= sizeof(mantissa_str) - 1)
            return BIGNUM_ERROR_INVALID_ARGS;

        strncat(mantissa_str, frac_start, frac_len);
        decimal_places = frac_len;
    } else {
        const char *end = exponent_char ? exponent_char : str + strlen(str);
        size_t len = end - str;
        if (len >= sizeof(mantissa_str) - 1)
            return BIGNUM_ERROR_INVALID_ARGS;
        strncpy(mantissa_str, str, len);
    }

    bignum_err_t err = bigint_set_from_string(&bigfloat->mantissa, mantissa_str);
    if (err != BIGNUM_OK)
        return err;

    int32_t exponent = -decimal_places;
    if (exponent_char)
        exponent += atoi(exponent_char + 1);

    bigfloat->exponent = exponent;
    bigfloat->negative = negative;

    return BIGNUM_OK;
}

bignum_err_t bigfloat_set_from_double(REAL *bigfloat, double value) {
    if (!bigfloat)
        return BIGNUM_ERROR_INVALID_ARGS;

    char str[64];
    snprintf(str, sizeof(str), "%.15g", value);
    return bigfloat_set_from_string(bigfloat, str);
}

bignum_err_t bigfloat_set_from_bigint(REAL *bigfloat, const INT *bigint) {
    if (!bigfloat || !bigint)
        return BIGNUM_ERROR_INVALID_ARGS;

    bignum_err_t err = bigint_ensure_capacity(&bigfloat->mantissa, bigint->length);
    if (err != BIGNUM_OK)
        return err;

    memcpy(bigfloat->mantissa.digits, bigint->digits, bigint->length * sizeof(uint32_t));
    bigfloat->mantissa.length = bigint->length;
    bigfloat->mantissa.negative = false; // Sign is stored in big float

    bigfloat->exponent = 0;
    bigfloat->negative = bigint->negative;

    return BIGNUM_OK;
}

bignum_err_t bigfloat_copy(const REAL *src, REAL *dest) {
    if (!src || !dest)
        return BIGNUM_ERROR_INVALID_ARGS;

    bignum_err_t err = bigint_ensure_capacity(&dest->mantissa, src->mantissa.length);
    if (err != BIGNUM_OK)
        return err;

    memcpy(dest->mantissa.digits, src->mantissa.digits, src->mantissa.length * sizeof(uint32_t));
    dest->mantissa.length = src->mantissa.length;
    dest->mantissa.negative = src->mantissa.negative;

    dest->exponent = src->exponent;
    dest->precision = src->precision;
    dest->negative = src->negative;

    return BIGNUM_OK;
}

char *bigfloat_to_string(const REAL *bigfloat) {
    if (!bigfloat)
        return NULL;

    if (bigfloat_is_zero(bigfloat)) {
        char *result = malloc(4 * sizeof(char));
        if (result)
            strcpy(result, "0.0");
        return result;
    }

    char *mantissa_str = bigint_to_string(&bigfloat->mantissa);
    if (!mantissa_str)
        return NULL;

    size_t mantissa_len = strlen(mantissa_str);
    size_t result_len = mantissa_len + 32; // Extra space for formatting
    char *result = malloc(result_len * sizeof(char));
    if (!result) {
        free(mantissa_str);
        return NULL;
    }

    if (bigfloat->exponent == 0)
        snprintf(result, result_len, "%s%s.0",
                 bigfloat->negative ? "-" : "", mantissa_str);
    else if (bigfloat->exponent > 0)
        snprintf(result, result_len, "%s%se%d",
                 bigfloat->negative ? "-" : "", mantissa_str, bigfloat->exponent);
    else
        snprintf(result, result_len, "%s%se%d",
                 bigfloat->negative ? "-" : "", mantissa_str, bigfloat->exponent);

    free(mantissa_str);
    return result;
}

double bigfloat_to_double(const REAL *bigfloat) {
    if (!bigfloat || bigfloat_is_zero(bigfloat))
        return 0.0;

    char *str = bigfloat_to_string(bigfloat);
    if (!str)
        return 0.0;

    double result = strtod(str, NULL);
    free(str);

    return result;
}

INT *bigfloat_to_bigint(const REAL *bigfloat) {
    if (!bigfloat)
        return NULL;

    INT *result = bigint_new_copy(&bigfloat->mantissa);
    if (!result)
        return NULL;

    // Apply exponent by multiplying or dividing by powers of 10
    if (bigfloat->exponent > 0) {
        INT *multiplier = bigint_create_power(10, bigfloat->exponent);
        if (!multiplier) {
            bigint_free(result);
            return NULL;
        }

        bigint_multiply(result, multiplier, result);
        bigint_free(multiplier);
    }
    else if (bigfloat->exponent < 0) {
        INT *divisor = bigint_create_power(10, -bigfloat->exponent);
        if (!divisor) {
            bigint_free(result);
            return NULL;
        }

        bigint_divide(result, divisor, result, NULL);
        bigint_free(divisor);
    }

    result->negative = bigfloat->negative;
    return result;
}

bignum_err_t bigfloat_add(const REAL *a, const REAL *b, REAL *result) {
    BIGNUM_VALIDATE_PARAMS_3(a, b, result);

    // Align exponents
    int32_t min_exp = a->exponent < b->exponent ? a->exponent : b->exponent;

    // Create aligned mantissas
    INT *a_aligned = bigint_new_copy(&a->mantissa);
    INT *b_aligned = bigint_new_copy(&b->mantissa);

    if (!a_aligned || !b_aligned) {
        bigint_free(a_aligned);
        bigint_free(b_aligned);
        return BIGNUM_ERROR_MEMORY;
    }

    // Scale mantissas to align decimal points
    if (a->exponent - min_exp > 0) {
        INT *ten_power = bigint_create_power(10, a->exponent - min_exp);
        if (!ten_power) {
            bigint_free(a_aligned);
            bigint_free(b_aligned);
            return BIGNUM_ERROR_MEMORY;
        }
        bigint_multiply(a_aligned, ten_power, a_aligned);
        bigint_free(ten_power);
    }

    if (b->exponent - min_exp > 0) {
        INT *ten_power = bigint_create_power(10, b->exponent - min_exp);
        if (!ten_power) {
            bigint_free(a_aligned);
            bigint_free(b_aligned);
            return BIGNUM_ERROR_MEMORY;
        }
        bigint_multiply(b_aligned, ten_power, b_aligned);
        bigint_free(ten_power);
    }

    // Apply signs
    a_aligned->negative = a->negative;
    b_aligned->negative = b->negative;

    // Add the aligned mantissas
    INT *sum = bigint_new();
    if (!sum) {
        bigint_free(a_aligned);
        bigint_free(b_aligned);
        return BIGNUM_ERROR_MEMORY;
    }

    bignum_err_t err = bigint_add(a_aligned, b_aligned, sum);
    if (err != BIGNUM_OK) {
        bigint_free(a_aligned);
        bigint_free(b_aligned);
        bigint_free(sum);
        return err;
    }

    result->mantissa = *sum;
    free(sum);
    result->exponent = min_exp;
    result->negative = sum->negative;
    result->precision = a->precision > b->precision ? a->precision : b->precision;

    bigint_free(a_aligned);
    bigint_free(b_aligned);

    return BIGNUM_OK;
}

bignum_err_t bigfloat_subtract(const REAL *a, const REAL *b, REAL *result) {
    BIGNUM_VALIDATE_PARAMS_3(a, b, result);

    // Create a copy of b with negated sign
    REAL *neg_b = bigfloat_new_copy(b);
    if (!neg_b)
        return BIGNUM_ERROR_MEMORY;

    neg_b->negative = !neg_b->negative;

    bignum_err_t err = bigfloat_add(a, neg_b, result);
    bigfloat_free(neg_b);

    return err;
}

bignum_err_t bigfloat_multiply(const REAL *a, const REAL *b, REAL *result) {
    BIGNUM_VALIDATE_PARAMS_3(a, b, result);

    INT *product = bigint_new();
    if (!product)
        return BIGNUM_ERROR_MEMORY;

    bignum_err_t err = bigint_multiply(&a->mantissa, &b->mantissa, product);
    if (err != BIGNUM_OK) {
        bigint_free(product);
        return err;
    }

    result->mantissa = *product;
    free(product);
    result->exponent = a->exponent + b->exponent;
    result->negative = a->negative != b->negative;
    result->precision = a->precision > b->precision ? a->precision : b->precision;

    return BIGNUM_OK;
}

bignum_err_t bigfloat_divide(const REAL *a, const REAL *b, REAL *result) {
    BIGNUM_VALIDATE_PARAMS_3(a, b, result);

    if (bigfloat_is_zero(b))
        return BIGNUM_ERROR_DIVIDE_BY_ZERO;

    // Use double precision for division for now
    double a_d = bigfloat_to_double(a);
    double b_d = bigfloat_to_double(b);
    double result_d = a_d / b_d;

    return bigfloat_set_from_double(result, result_d);
}

int bigfloat_compare(const REAL *a, const REAL *b) {
    if (!a || !b)
        return 0;

    if (bigfloat_is_zero(a) && bigfloat_is_zero(b))
        return 0;

    if (a->negative != b->negative)
        return a->negative ? -1 : 1;

    if (a->exponent != b->exponent) {
        if (a->exponent > b->exponent)
            return a->negative ? -1 : 1;
        else
            return a->negative ? 1 : -1;
    }

    int cmp = bigint_compare(&a->mantissa, &b->mantissa);
    return a->negative ? -cmp : cmp;
}

bool bigfloat_equals(const REAL *a, const REAL *b) {
    return bigfloat_compare(a, b) == 0;
}

bool bigfloat_less_than(const REAL *a, const REAL *b) {
    return bigfloat_compare(a, b) < 0;
}

bool bigfloat_greater_than(const REAL *a, const REAL *b) {
    return bigfloat_compare(a, b) > 0;
}

bool bigfloat_is_zero(const REAL *a) {
    return a && bigint_is_zero(&a->mantissa);
}

bool bigfloat_is_negative(const REAL *a) {
    return a && a->negative && !bigfloat_is_zero(a);
}

bignum_err_t bigfloat_absolute(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (result != a) {
        REAL *copy = bigfloat_new_copy(a);
        if (!copy)
            return BIGNUM_ERROR_MEMORY;

        bigfloat_free(result);
        *result = *copy;
        free(copy);
    }

    result->negative = false;
    return BIGNUM_OK;
}

bignum_err_t bigfloat_negate(REAL *a) {
    if (!a)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (!bigfloat_is_zero(a))
        a->negative = !a->negative;

    return BIGNUM_OK;
}

bignum_err_t bigfloat_set_precision(REAL *bigfloat, size_t precision) {
    if (!bigfloat)
        return BIGNUM_ERROR_INVALID_ARGS;

    bigfloat->precision = precision > 0 ? precision : BIGNUM_DEFAULT_PRECISION;
    return BIGNUM_OK;
}

bignum_err_t bigfloat_sqrt(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (bigfloat_is_negative(a))
        return BIGNUM_ERROR_INVALID_ARGS; // No complex numbers support

    if (bigfloat_is_zero(a))
        return bigfloat_set_from_double(result, 0.0);

    // Simplified Newton-Raphson method using double precision
    double approx = sqrt(bigfloat_to_double(a));
    return bigfloat_set_from_double(result, approx);
}

bignum_err_t bigfloat_floor(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    INT *truncated = bigfloat_to_bigint(a);
    if (!truncated)
        return BIGNUM_ERROR_MEMORY;

    // If negative and has fractional part, subtract 1
    if (a->negative && a->exponent < 0) {
        INT *one = bigint_new_from_int64(1);
        if (!one) {
            bigint_free(truncated);
            return BIGNUM_ERROR_MEMORY;
        }
        bigint_subtract(truncated, one, truncated);
        bigint_free(one);
    }

    bignum_err_t err = bigfloat_set_from_bigint(result, truncated);
    bigint_free(truncated);
    return err;
}

bignum_err_t bigfloat_ceil(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    INT *truncated = bigfloat_to_bigint(a);
    if (!truncated)
        return BIGNUM_ERROR_MEMORY;

    // If positive and has fractional part, add 1
    if (!a->negative && a->exponent < 0) {
        INT *one = bigint_new_from_int64(1);
        if (!one) {
            bigint_free(truncated);
            return BIGNUM_ERROR_MEMORY;
        }
        bigint_add(truncated, one, truncated);
        bigint_free(one);
    }

    bignum_err_t err = bigfloat_set_from_bigint(result, truncated);
    bigint_free(truncated);
    return err;
}

bignum_err_t bigfloat_round(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    // Simplified rounding: add 0.5 and floor
    REAL *half = bigfloat_new_from_double(0.5, a->precision);
    if (!half)
        return BIGNUM_ERROR_MEMORY;

    REAL *temp = bigfloat_new(a->precision);
    if (!temp) {
        bigfloat_free(half);
        return BIGNUM_ERROR_MEMORY;
    }

    bignum_err_t err = bigfloat_add(a, half, temp);
    if (err == BIGNUM_OK)
        err = bigfloat_floor(temp, result);

    bigfloat_free(half);
    bigfloat_free(temp);
    return err;
}

bignum_err_t bigfloat_power(const REAL *base, const REAL *exponent, REAL *result) {
    if (!base || !exponent || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    // Simplified implementation using double precision
    double base_d = bigfloat_to_double(base);
    double exp_d = bigfloat_to_double(exponent);
    double result_d = pow(base_d, exp_d);

    return bigfloat_set_from_double(result, result_d);
}

REAL* bigint_to_bigfloat(const INT *bigint) {
    if (!bigint)
        return NULL;

    REAL *result = malloc(sizeof(REAL));
    if (!result)
        return NULL;
    bignum_err_t err = bigfloat_set_from_bigint(result, bigint);
    if (err != BIGNUM_OK) {
        free(result);
        return NULL;
    }
    return result;
}

bignum_err_t _bigint_to_bigfloat(const INT *bigint, REAL *result) {
    if (!bigint || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    return bigfloat_set_from_bigint(result, bigint);
}

bignum_err_t _bigfloat_to_bigint(const REAL *bigfloat, INT *result) {
    if (!bigfloat || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    INT *temp = bigfloat_to_bigint(bigfloat);
    if (!temp)
        return BIGNUM_ERROR_MEMORY;

    bignum_err_t err = bigint_ensure_capacity(result, temp->length);
    if (err == BIGNUM_OK) {
        memcpy(result->digits, temp->digits, temp->length * sizeof(uint32_t));
        result->length = temp->length;
        result->negative = temp->negative;
    }

    bigint_free(temp);
    return err;
}

bignum_err_t bigfloat_trunc(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, trunc(d));
}

bignum_err_t bigfloat_exp(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, exp(d));
}

bignum_err_t bigfloat_log(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    if (d <= 0)
        return BIGNUM_ERROR_INVALID_ARGS;
    return bigfloat_set_from_double(result, log(d));
}

bignum_err_t bigfloat_log10(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    if (d <= 0)
        return BIGNUM_ERROR_INVALID_ARGS;
    return bigfloat_set_from_double(result, log10(d));
}

bignum_err_t bigfloat_log2(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    if (d <= 0)
        return BIGNUM_ERROR_INVALID_ARGS;
    return bigfloat_set_from_double(result, log2(d));
}

bignum_err_t bigfloat_pow(const REAL *base, const REAL *exp, REAL *result) {
    if (!base || !exp || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double b = bigfloat_to_double(base);
    double e = bigfloat_to_double(exp);
    return bigfloat_set_from_double(result, pow(b, e));
}

bignum_err_t bigfloat_sin(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, sin(d));
}

bignum_err_t bigfloat_cos(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, cos(d));
}

bignum_err_t bigfloat_tan(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, tan(d));
}

bignum_err_t bigfloat_asin(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    if (d < -1.0 || d > 1.0)
        return BIGNUM_ERROR_INVALID_ARGS;
    return bigfloat_set_from_double(result, asin(d));
}

bignum_err_t bigfloat_acos(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    if (d < -1.0 || d > 1.0)
        return BIGNUM_ERROR_INVALID_ARGS;
    return bigfloat_set_from_double(result, acos(d));
}

bignum_err_t bigfloat_atan(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, atan(d));
}

bignum_err_t bigfloat_atan2(const REAL *y, const REAL *x, REAL *result) {
    if (!y || !x || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double y_d = bigfloat_to_double(y);
    double x_d = bigfloat_to_double(x);
    return bigfloat_set_from_double(result, atan2(y_d, x_d));
}

bignum_err_t bigfloat_sinh(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, sinh(d));
}

bignum_err_t bigfloat_cosh(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, cosh(d));
}

bignum_err_t bigfloat_tanh(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    return bigfloat_set_from_double(result, tanh(d));
}

bignum_err_t bigfloat_sign(const REAL *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double d = bigfloat_to_double(a);
    double sign = (d > 0) ? 1.0 : (d < 0) ? -1.0
                                          : 0.0;
    return bigfloat_set_from_double(result, sign);
}

bignum_err_t bigfloat_min(const REAL *a, const REAL *b, REAL *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double a_d = bigfloat_to_double(a);
    double b_d = bigfloat_to_double(b);
    return bigfloat_set_from_double(result, (a_d < b_d) ? a_d : b_d);
}

bignum_err_t bigfloat_max(const REAL *a, const REAL *b, REAL *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;
    double a_d = bigfloat_to_double(a);
    double b_d = bigfloat_to_double(b);
    return bigfloat_set_from_double(result, (a_d > b_d) ? a_d : b_d);
}

COMPLEX *bigcomplex_new(size_t precision) {
    COMPLEX *complex = malloc(sizeof(COMPLEX));
    if (!complex)
        return NULL;

    INT *real_zero = bigint_new();
    if (!real_zero) {
        free(complex);
        return NULL;
    }
    complex->real.mantissa = *real_zero;
    free(real_zero);
    complex->real.exponent = 0;
    complex->real.precision = precision > 0 ? precision : BIGNUM_DEFAULT_PRECISION;
    complex->real.negative = false;

    INT *imag_zero = bigint_new();
    if (!imag_zero) {
        free(complex->real.mantissa.digits);
        free(complex);
        return NULL;
    }
    complex->imag.mantissa = *imag_zero;
    free(imag_zero);
    complex->imag.exponent = 0;
    complex->imag.precision = precision > 0 ? precision : BIGNUM_DEFAULT_PRECISION;
    complex->imag.negative = false;

    return complex;
}

COMPLEX *bigcomplex_new_from_floats(const REAL *real, const REAL *imag, size_t precision) {
    if (!real || !imag)
        return NULL;

    COMPLEX *complex = bigcomplex_new(precision);
    if (!complex)
        return NULL;

    bignum_err_t err = bigint_ensure_capacity(&complex->real.mantissa, real->mantissa.length);
    if (err != BIGNUM_OK) {
        bigcomplex_free(complex);
        return NULL;
    }
    memcpy(complex->real.mantissa.digits, real->mantissa.digits, real->mantissa.length * sizeof(uint32_t));
    complex->real.mantissa.length = real->mantissa.length;
    complex->real.mantissa.negative = real->mantissa.negative;
    complex->real.exponent = real->exponent;
    complex->real.precision = precision;
    complex->real.negative = real->negative;

    err = bigint_ensure_capacity(&complex->imag.mantissa, imag->mantissa.length);
    if (err != BIGNUM_OK) {
        bigcomplex_free(complex);
        return NULL;
    }
    memcpy(complex->imag.mantissa.digits, imag->mantissa.digits, imag->mantissa.length * sizeof(uint32_t));
    complex->imag.mantissa.length = imag->mantissa.length;
    complex->imag.mantissa.negative = imag->mantissa.negative;
    complex->imag.exponent = imag->exponent;
    complex->imag.precision = precision;
    complex->imag.negative = imag->negative;

    return complex;
}

COMPLEX *bigcomplex_new_from_doubles(double real, double imag, size_t precision) {
    REAL *real_float = bigfloat_new_from_double(real, precision);
    REAL *imag_float = bigfloat_new_from_double(imag, precision);

    if (!real_float || !imag_float) {
        if (real_float)
            bigfloat_free(real_float);
        if (imag_float)
            bigfloat_free(imag_float);
        return NULL;
    }

    COMPLEX *complex = bigcomplex_new_from_floats(real_float, imag_float, precision);

    bigfloat_free(real_float);
    bigfloat_free(imag_float);

    return complex;
}

COMPLEX *bigcomplex_new_copy(const COMPLEX *src) {
    if (!src)
        return NULL;

    return bigcomplex_new_from_floats(&src->real, &src->imag, src->real.precision);
}

void bigcomplex_free(COMPLEX *complex) {
    if (!complex)
        return;

    free(complex->real.mantissa.digits);
    free(complex->imag.mantissa.digits);
    free(complex);
}

bignum_err_t bigcomplex_add(const COMPLEX *a, const COMPLEX *b, COMPLEX *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    bignum_err_t err;

    // real + real
    if ((err = bigfloat_add(&a->real, &b->real, &result->real)) != BIGNUM_OK)
        return err;

    // imag + imag
    if ((err = bigfloat_add(&a->imag, &b->imag, &result->imag)) != BIGNUM_OK)
        return err;

    return BIGNUM_OK;
}

bignum_err_t bigcomplex_subtract(const COMPLEX *a, const COMPLEX *b, COMPLEX *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    bignum_err_t err;

    // real - real
    if ((err = bigfloat_subtract(&a->real, &b->real, &result->real)) != BIGNUM_OK)
        return err;

    // imag - imag
    if ((err = bigfloat_subtract(&a->imag, &b->imag, &result->imag)) != BIGNUM_OK)
        return err;

    return BIGNUM_OK;
}

bignum_err_t bigcomplex_multiply(const COMPLEX *a, const COMPLEX *b, COMPLEX *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    bignum_err_t err;

    // (a.real * b.real) - (a.imag * b.imag) + i(a.real * b.imag + a.imag * b.real)

    REAL *temp1 = bigfloat_new(a->real.precision);
    REAL *temp2 = bigfloat_new(a->real.precision);
    REAL *temp3 = bigfloat_new(a->real.precision);
    REAL *temp4 = bigfloat_new(a->real.precision);

    if (!temp1 || !temp2 || !temp3 || !temp4) {
        if (temp1)
            bigfloat_free(temp1);
        if (temp2)
            bigfloat_free(temp2);
        if (temp3)
            bigfloat_free(temp3);
        if (temp4)
            bigfloat_free(temp4);
        return BIGNUM_ERROR_MEMORY;
    }

    // temp1 = a.real * b.real
    if ((err = bigfloat_multiply(&a->real, &b->real, temp1)) != BIGNUM_OK)
        goto cleanup;

    // temp2 = a.imag * b.imag
    if ((err = bigfloat_multiply(&a->imag, &b->imag, temp2)) != BIGNUM_OK)
        goto cleanup;

    // temp3 = a.real * b.imag
    if ((err = bigfloat_multiply(&a->real, &b->imag, temp3)) != BIGNUM_OK)
        goto cleanup;

    // temp4 = a.imag * b.real
    if ((err = bigfloat_multiply(&a->imag, &b->real, temp4)) != BIGNUM_OK)
        goto cleanup;

    // result.real = temp1 - temp2
    if ((err = bigfloat_subtract(temp1, temp2, &result->real)) != BIGNUM_OK)
        goto cleanup;

    // result.imag = temp3 + temp4
    if ((err = bigfloat_add(temp3, temp4, &result->imag)) != BIGNUM_OK)
        goto cleanup;

cleanup:
    bigfloat_free(temp1);
    bigfloat_free(temp2);
    bigfloat_free(temp3);
    bigfloat_free(temp4);

    return err;
}

bignum_err_t bigcomplex_divide(const COMPLEX *a, const COMPLEX *b, COMPLEX *result) {
    if (!a || !b || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    if (bigfloat_is_zero(&b->real) && bigfloat_is_zero(&b->imag))
        return BIGNUM_ERROR_DIVIDE_BY_ZERO;

    bignum_err_t err;

    // For complex division: (a * conj(b)) / |b|^2

    COMPLEX *conj_b = bigcomplex_new(a->real.precision);
    if (!conj_b)
        return BIGNUM_ERROR_MEMORY;

    // Get conjugate of b
    err = bigcomplex_conjugate(b, conj_b);
    if (err != BIGNUM_OK) {
        bigcomplex_free(conj_b);
        return err;
    }

    // Multiply a by conjugate of b
    COMPLEX *numerator = bigcomplex_new(a->real.precision);
    if (!numerator) {
        bigcomplex_free(conj_b);
        return BIGNUM_ERROR_MEMORY;
    }

    err = bigcomplex_multiply(a, conj_b, numerator);
    if (err != BIGNUM_OK) {
        bigcomplex_free(conj_b);
        bigcomplex_free(numerator);
        return err;
    }

    // Calculate |b|^2 = b.real^2 + b.imag^2
    REAL *b_real_sq = bigfloat_new(a->real.precision);
    REAL *b_imag_sq = bigfloat_new(a->real.precision);
    REAL *denominator = bigfloat_new(a->real.precision);

    if (!b_real_sq || !b_imag_sq || !denominator) {
        bigcomplex_free(conj_b);
        bigcomplex_free(numerator);
        if (b_real_sq)
            bigfloat_free(b_real_sq);
        if (b_imag_sq)
            bigfloat_free(b_imag_sq);
        if (denominator)
            bigfloat_free(denominator);
        return BIGNUM_ERROR_MEMORY;
    }

    err = bigfloat_multiply(&b->real, &b->real, b_real_sq);
    if (err != BIGNUM_OK)
        goto cleanup_div;

    err = bigfloat_multiply(&b->imag, &b->imag, b_imag_sq);
    if (err != BIGNUM_OK)
        goto cleanup_div;

    err = bigfloat_add(b_real_sq, b_imag_sq, denominator);
    if (err != BIGNUM_OK)
        goto cleanup_div;

    // Divide real and imaginary parts by denominator
    err = bigfloat_divide(&numerator->real, denominator, &result->real);
    if (err != BIGNUM_OK)
        goto cleanup_div;

    err = bigfloat_divide(&numerator->imag, denominator, &result->imag);
    if (err != BIGNUM_OK)
        goto cleanup_div;

cleanup_div:
    bigcomplex_free(conj_b);
    bigcomplex_free(numerator);
    bigfloat_free(b_real_sq);
    bigfloat_free(b_imag_sq);
    bigfloat_free(denominator);

    return err;
}

bignum_err_t bigcomplex_conjugate(const COMPLEX *a, COMPLEX *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    bignum_err_t err = bigfloat_copy(&a->real, &result->real);
    if (err != BIGNUM_OK)
        return err;

    err = bigfloat_copy(&a->imag, &result->imag);
    if (err != BIGNUM_OK)
        return err;

    result->imag.negative = !a->imag.negative;

    return BIGNUM_OK;
}

bignum_err_t bigcomplex_absolute(const COMPLEX *a, REAL *result) {
    if (!a || !result)
        return BIGNUM_ERROR_INVALID_ARGS;

    // |z| = sqrt(real^2 + imag^2)

    REAL *real_sq = bigfloat_new(a->real.precision);
    REAL *imag_sq = bigfloat_new(a->real.precision);
    REAL *sum = bigfloat_new(a->real.precision);

    if (!real_sq || !imag_sq || !sum) {
        if (real_sq)
            bigfloat_free(real_sq);
        if (imag_sq)
            bigfloat_free(imag_sq);
        if (sum)
            bigfloat_free(sum);
        return BIGNUM_ERROR_MEMORY;
    }

    bignum_err_t err;

    err = bigfloat_multiply(&a->real, &a->real, real_sq);
    if (err != BIGNUM_OK)
        goto cleanup_abs;

    err = bigfloat_multiply(&a->imag, &a->imag, imag_sq);
    if (err != BIGNUM_OK)
        goto cleanup_abs;

    err = bigfloat_add(real_sq, imag_sq, sum);
    if (err != BIGNUM_OK)
        goto cleanup_abs;

    err = bigfloat_sqrt(sum, result);
    if (err != BIGNUM_OK)
        goto cleanup_abs;

cleanup_abs:
    bigfloat_free(real_sq);
    bigfloat_free(imag_sq);
    bigfloat_free(sum);

    return err;
}

char *bigcomplex_to_string(const COMPLEX *complex) {
    if (!complex)
        return NULL;

    char *real_str = bigfloat_to_string(&complex->real);
    char *imag_str = bigfloat_to_string(&complex->imag);

    if (!real_str || !imag_str) {
        if (real_str)
            free(real_str);
        if (imag_str)
            free(imag_str);
        return NULL;
    }

    // Format as "real + imag i" or similar
    size_t len = strlen(real_str) + strlen(imag_str) + 8; // +8 for " +  i" and null
    char *result = malloc(len * sizeof(char));
    if (!result) {
        free(real_str);
        free(imag_str);
        return NULL;
    }

    // Check if imaginary part is zero
    if (bigfloat_is_zero(&complex->imag))
        strcpy(result, real_str);
    else if (bigfloat_is_zero(&complex->real)) {
        if (bigfloat_compare(&complex->imag, bigfloat_new_from_double(1.0, complex->real.precision)) == 0)
            strcpy(result, "i");
        else if (bigfloat_compare(&complex->imag, bigfloat_new_from_double(-1.0, complex->real.precision)) == 0)
            strcpy(result, "-i");
        else
            snprintf(result, len, "%si", imag_str);
    } else {
        if (bigfloat_compare(&complex->imag, bigfloat_new_from_double(1.0, complex->real.precision)) == 0)
            snprintf(result, len, "%s + i", real_str);
        else if (bigfloat_compare(&complex->imag, bigfloat_new_from_double(-1.0, complex->real.precision)) == 0)
            snprintf(result, len, "%s - i", real_str);
        else if (complex->imag.negative)
            snprintf(result, len, "%s - %si", real_str, imag_str + 1); // Skip the minus sign
        else
            snprintf(result, len, "%s + %si", real_str, imag_str);
    }

    free(real_str);
    free(imag_str);

    return result;
}

bool bigcomplex_equals(const COMPLEX *a, const COMPLEX *b) {
    if (!a || !b)
        return false;

    return bigfloat_equals(&a->real, &b->real) && bigfloat_equals(&a->imag, &b->imag);
}

bool bigcomplex_is_zero(const COMPLEX *a) {
    return a && bigfloat_is_zero(&a->real) && bigfloat_is_zero(&a->imag);
}

COMPLEX *bigcomplex_new_from_string(const char *str, size_t precision) {
    if (!str)
        return NULL;

    while (*str && isspace(*str))
        str++;

    if (*str == '\0')
        return NULL;

    // Simple parser for complex numbers
    // Supports formats like: "1+2i", "1-2i", "2i", "-3i", "5", "1.5", "-i", "i"

    const char *i_pos = strchr(str, 'i');
    if (!i_pos) {
        // No 'i' found, treat as real number
        REAL *real = bigfloat_new_from_string(str, precision);
        if (!real)
            return NULL;

        REAL *imag_zero = bigfloat_new_from_double(0.0, precision);
        if (!imag_zero) {
            bigfloat_free(real);
            return NULL;
        }

        COMPLEX *result = bigcomplex_new_from_floats(real, imag_zero, precision);
        bigfloat_free(real);
        bigfloat_free(imag_zero);
        return result;
    }

    // Has 'i', parse as complex
    double real_part = 0.0;
    double imag_part = 0.0;

    if (i_pos == str) {
        // Starts with 'i', like "i" or "-i"
        if (str[1] == '\0')
            imag_part = 1.0;
        else if (str[1] == '\0' && str[0] == '-')
            imag_part = -1.0;
        else
            return NULL; // Invalid format
    } else {
        // Parse the string before 'i'
        char *temp_str = strdup(str);
        if (!temp_str)
            return NULL;

        // Find the position of 'i'
        char *i_in_temp = strchr(temp_str, 'i');
        if (!i_in_temp) {
            free(temp_str);
            return NULL;
        }

        // Temporarily null-terminate at 'i'
        *i_in_temp = '\0';

        // Parse the coefficient before 'i'
        if (*temp_str == '\0') {
            // Just "i"
            imag_part = 1.0;
        } else {
            imag_part = strtod(temp_str, NULL);
        }

        free(temp_str);
    }

    // Check if there's a real part before the imaginary part
    const char *plus_pos = strrchr(str, '+');
    const char *minus_pos = strrchr(str, '-');

    // Find the last operator before 'i'
    const char *operator_pos = NULL;
    if (plus_pos && plus_pos < i_pos) {
        operator_pos = plus_pos;
        real_part = strtod(str, NULL);
    }
    else if (minus_pos && minus_pos < i_pos && minus_pos != str) { // Not at the beginning
        operator_pos = minus_pos;
        // For subtraction, we need to parse differently
        char *temp = strndup(str, operator_pos - str);
        if (temp) {
            real_part = strtod(temp, NULL);
            free(temp);
        }
        imag_part = -fabs(imag_part); // Make imaginary negative
    }

    COMPLEX *result = bigcomplex_new_from_doubles(real_part, imag_part, precision);
    return result;
}
#endif
