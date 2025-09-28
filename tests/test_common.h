/* Lightweight test helpers for tests/ */
#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int TEST_FAILURES = 0;

static inline void expect_int_impl(const char *name, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s: got %d want %d\n", name, got, want);
        TEST_FAILURES++;
    } else {
        printf("ok %s\n", name);
    }
}

static inline void expect_str_impl(const char *name, const char *got, const char *want) {
    if (!got || !want || strcmp(got, want) != 0) {
        fprintf(stderr, "FAIL %s: got '%s' want '%s'\n", name, got ? got : "(null)", want ? want : "(null)");
        TEST_FAILURES++;
    } else {
        printf("ok %s\n", name);
    }
}

/* convenience aliases matching previous names */
#define expect_int(name, got, want) expect_int_impl((name),(got),(want))
#define expect_str(name, got, want) expect_str_impl((name),(got),(want))
#define test_failures() (TEST_FAILURES)
#define test_comment(fmt, ...) fprintf(stderr, "# " fmt "\n", ##__VA_ARGS__)

#endif /* TEST_COMMON_H */
