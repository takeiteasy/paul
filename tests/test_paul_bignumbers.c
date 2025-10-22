#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_common.h"

#define PAUL_IMPLEMENTATION
#include "../paul_bignumbers.h"

int main(void) {
    /* number of tests */
    printf("1..8\n");

    /* Test 1: Create a new bigint */
    INT *a = bigint_new();
    expect_int("bigint_new not null", a != NULL, 1);

    /* Test 2: Set from int64 */
    bignum_err_t err = bigint_set_from_int64(a, 42);
    expect_int("set_from_int64 success", err, BIGNUM_OK);

    /* Test 3: Convert to string */
    char *str = bigint_to_string(a);
    expect_str("to_string 42", str, "42");
    free(str);

    /* Test 4: Create another bigint */
    INT *b = bigint_new_from_int64(58);
    expect_int("bigint_new_from_int64 not null", b != NULL, 1);

    /* Test 5: Add two bigints */
    INT *result = bigint_new();
    err = bigint_add(a, b, result);
    expect_int("add success", err, BIGNUM_OK);

    /* Test 6: Check sum */
    str = bigint_to_string(result);
    expect_str("add result 100", str, "100");
    free(str);

    /* Test 7: Compare */
    int cmp = bigint_compare(a, b);
    expect_int("compare 42 < 58", cmp, -1);

    /* Test 8: Free */
    bigint_free(a);
    bigint_free(b);
    bigint_free(result);
    expect_int("free completed", 1, 1); // Always pass

    if (test_failures() == 0) {
        fprintf(stderr, "# all bignumbers tests passed\n");
        return 0;
    }
    fprintf(stderr, "# %d bignumbers test(s) failed\n", test_failures());
    return 1;
}