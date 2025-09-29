#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "test_common.h"

/* Ensure GNU-style typeof is available when compiling under strict -std=c17 */
#ifndef typeof
#define typeof __typeof__
#endif

#include "../paul_list.h"

int main(void) {
    /* number of tests */
    printf("1..14\n");

    int *lst = NULL;

    /* Test 1: append and count */
    list_append(lst, 1);
    list_append(lst, 2);
    list_append(lst, 3);
    expect_int("append_count", list_count(lst), 3);

    /* Test 2: first element (car) */
    expect_int("car_value", *((int *)list_car(lst)), 1);

    /* Test 3: last element */
    expect_int("last_value", *((int *)list_last(lst)), 3);

    /* Test 4 & 5: insert at index 1 (value and count)
       after: [1, 42, 2, 3] */
    list_insert(lst, 1, 42);
    expect_int("insert_value", lst[1], 42);
    expect_int("insert_count", list_count(lst), 4);

    /* Test 6 & 7: push to front and check first and count
       after: [99, 1, 42, 2, 3] */
    list_push(lst, 99);
    expect_int("push_first", lst[0], 99);
    expect_int("push_count", list_count(lst), 5);

    /* Test 8 & 9: pop reduces count and last element updates
       after pop: [99,1,42,2] */
    list_pop(lst);
    expect_int("pop_count", list_count(lst), 4);
    expect_int("pop_last", *((int *)list_last(lst)), 2);

    /* Test 10 & 11: reverse list and verify first/last
       before reverse: [99,1,42,2]
       after reverse:  [2,42,1,99] */
    list_reverse(lst);
    expect_int("reverse_first", lst[0], 2);
    expect_int("reverse_last", *((int *)list_last(lst)), 99);

    /* Test 12: shuffle preserves multiset of elements
       Seed rand for reproducibility. */
    int n = (int)list_count(lst);
    int orig[16] = {0};
    for (int i = 0; i < n; i++)
        orig[i] = lst[i];
    srand(2025);
    list_shuffle(lst);
    /* verify same multiset by counting occurrences */
    int counts[256] = {0};
    int ok = 1;
    for (int i = 0; i < n; i++)
    {
        if ((unsigned)orig[i] >= (unsigned)256)
        {
            ok = 0;
            break;
        }
        counts[(unsigned)orig[i]]++;
    }
    for (int i = 0; i < n; i++)
    {
        if ((unsigned)lst[i] >= (unsigned)256)
        {
            ok = 0;
            break;
        }
        counts[(unsigned)lst[i]]--;
    }
    for (int v = 0; v < 256; v++)
        if (counts[v] != 0)
            ok = 0;
    expect_int("shuffle_preserve", ok, 1);

    /* Test 13: clear makes count zero */
    list_clear(lst);
    expect_int("clear_count_zero", list_count(lst), 0);

    /* Test 14: free and set to NULL; count should be zero when NULL */
    list_free(lst);
    lst = NULL;
    expect_int("free_null_count", list_count(lst), 0);

    if (test_failures() == 0)
    {
        fprintf(stderr, "# all list tests passed\n");
        return 0;
    }
    fprintf(stderr, "# %d list test(s) failed\n", test_failures());
    return 1;
}
