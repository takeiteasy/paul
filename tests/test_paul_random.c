#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "test_common.h"

#define PAUL_RANDOM_IMPLEMENTATION
#include "../paul_random.h"

/* swap callback for rnd_shuffle_cb */
static void int_array_swap(size_t a, size_t b, void *user_data) {
    int *arr = (int *)user_data;
    int tmp = arr[a]; arr[a] = arr[b]; arr[b] = tmp;
}

int main(void) {
    printf("1..6\n");

    /* Test 1: repeatability with same seed */
    rng_t r1, r2;
    rnd_seed(&r1, 123456789ULL);
    rnd_seed(&r2, 123456789ULL);
    uint64_t v1 = rnd_randi(&r1);
    uint64_t v2 = rnd_randi(&r2);
    expect_int("rng_repeatable_first", v1 == v2 ? 1 : 0, 1);

    /* Test 2: rnd_randf_signed in [-1,1) */
    rnd_seed(&r1, 424242ULL);
    float fs = rnd_randf_signed(&r1);
    expect_int("rnd_randf_signed_range", (fs >= -1.0f && fs < 1.0f) ? 1 : 0, 1);

    /* Test 3: rnd_exponential > 0 */
    rnd_seed(&r1, 654321ULL);
    float ex = rnd_exponential(&r1, 1.5f);
    expect_int("rnd_exponential_positive", (ex > 0.0f) ? 1 : 0, 1);

    /* Test 4: weighted choice from contiguous array with single non-zero weight */
    float weights[4] = {0.0f, 0.0f, 1.0f, 0.0f};
    rnd_seed(&r1, 1ULL);
    int wi = rnd_weighted_choice_array(&r1, weights, 4);
    expect_int("rnd_weighted_choice_array_single", wi, 2);

    /* Test 5: rnd_permutation yields unique values 0..n-1 */
    const size_t N = 10;
    uint32_t perm[N];
    rnd_seed(&r1, 99ULL);
    rnd_permutation(&r1, perm, N);
    int seen[N]; memset(seen, 0, sizeof(seen));
    int unique = 1;
    for (size_t i = 0; i < N; i++) {
        if (perm[i] >= N) { unique = 0; break; }
        if (seen[perm[i]]) { unique = 0; break; }
        seen[perm[i]] = 1;
    }
    expect_int("rnd_permutation_unique", unique, 1);

    /* Test 6: rnd_shuffle_cb preserves elements and values 0..N-1 */
    int arr[N];
    for (int i = 0; i < (int)N; i++) arr[i] = i;
    rnd_seed(&r1, 2025ULL);
    rnd_shuffle_cb(&r1, N, int_array_swap, arr);
    /* Verify contents are a permutation of 0..N-1 */
    int seen2[N]; memset(seen2, 0, sizeof(seen2));
    int ok = 1;
    for (size_t i = 0; i < N; i++) {
        if (arr[i] < 0 || arr[i] >= (int)N) { ok = 0; break; }
        if (seen2[arr[i]]) { ok = 0; break; }
        seen2[arr[i]] = 1;
    }
    expect_int("rnd_shuffle_cb_preserve", ok, 1);

    if (test_failures() == 0) {
        fprintf(stderr, "# all random tests passed\n");
        return 0;
    }
    fprintf(stderr, "# %d random test(s) failed\n", test_failures());
    return 1;
}
