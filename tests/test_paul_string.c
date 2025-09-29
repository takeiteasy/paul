#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "test_common.h"

#include "../paul_string.h"

/* Expanded tests for paul_string: ascii/wide creation, conversion and many utilities */
static void expect_wstr(const char *name, const wchar_t *got, const wchar_t *want) {
    if (!got || !want || wcscmp(got, want) != 0) {
        fprintf(stderr, "FAIL %s: got '", name);
        if (got) {
            /* print wide as narrow for diagnostics (best-effort) */
            size_t len = wcslen(got);
            for (size_t i = 0; i < len; i++) fputc((char)got[i], stderr);
        } else fprintf(stderr, "(null)");
        fprintf(stderr, "' want '");
        if (want) {
            size_t len = wcslen(want);
            for (size_t i = 0; i < len; i++) fputc((char)want[i], stderr);
        } else fprintf(stderr, "(null)");
        fprintf(stderr, "'\n");
        TEST_FAILURES++;
    } else {
        printf("ok %s\n", name);
    }
}

int main(void) {
    /* total tests below: update header if you change count */
    printf("1..32\n");

    /* creation and raw access */
    str_t a = str_from_cstr("Hello");
    expect_str("from_cstr", str_raw_cstr(a), "Hello");

    str_t a_dup = str_dup(a);
    expect_str("dup", str_raw_cstr(a_dup), "Hello");

    /* UTF-16 conversion and checks */
    str_t aw = str_make_utf16(a);
    expect_wstr("make_utf16", str_raw_wcstr(aw), L"Hello");
    expect_int("is_utf16", str_is_utf16(aw) ? 1 : 0, 1);
    /* convert back to ASCII */
    str_t a2 = str_make_ascii(aw);
    expect_str("utf16_to_ascii", str_raw_cstr(a2), "Hello");

    /* conversion failure: non-ASCII wide char */
    str_t non_ascii = str_from_wcstr(L"\u00E9"); /* 'é' */
    expect_int("make_ascii_fail", str_make_ascii(non_ascii) == NULL ? 1 : 0, 1);

    /* resizing */
    str_t r = str_from_cstr("abcdef");
    str_resize(&r, 3);
    expect_str("resize_shrink", str_raw_cstr(r), "abc");

    /* concat */
    str_t c = str_from_cstr("foo");
    str_t tail = str_from_cstr("bar");
    str_concat(&c, tail);
    expect_str("concat", str_raw_cstr(c), "foobar");

    /* append char */
    str_t ac = str_from_cstr("x");
    str_append_char(&ac, 'y');
    expect_str("append_char", str_raw_cstr(ac), "xy");

    /* insert / erase */
    str_t ins = str_from_cstr("ace");
    str_t one = str_from_cstr("b");
    str_insert(&ins, 1, one); /* a b c e */
    expect_str("insert", str_raw_cstr(ins), "abce");
    str_erase(&ins, 2, 1); /* remove 'c' */
    expect_str("erase", str_raw_cstr(ins), "abe");

    /* replace (first occurrence) */
    str_t rep = str_from_cstr("the cat sat");
    str_t old = str_from_cstr("cat");
    str_t nw = str_from_cstr("dog");
    str_replace(&rep, old, nw);
    expect_str("replace", str_raw_cstr(rep), "the dog sat");

    /* trim */
    str_t tr = str_from_cstr("  hi\n");
    str_trim(&tr);
    expect_str("trim", str_raw_cstr(tr), "hi");

    /* to upper / lower */
    str_t ul = str_from_cstr("AbC");
    str_to_upper(&ul);
    expect_str("to_upper", str_raw_cstr(ul), "ABC");
    str_to_lower(&ul);
    expect_str("to_lower", str_raw_cstr(ul), "abc");

    /* cmp / equal */
    str_t ca = str_from_cstr("a");
    str_t ca2 = str_from_cstr("a");
    expect_int("equal_true", str_equal(ca, ca2) ? 1 : 0, 1);
    expect_int("cmp_zero", str_cmp(ca, ca2) == 0 ? 1 : 0, 1);
    str_t cb = str_from_cstr("b");
    expect_int("cmp_neg", str_cmp(ca, cb) < 0 ? 1 : 0, 1);

    /* find / find_char */
    str_t f = str_from_cstr("abracadabra");
    str_t sub = str_from_cstr("cada");
    expect_int("find_sub", (int)str_find(f, sub), 4);
    expect_int("find_char", (int)str_find_char(f, 'r'), 2);

    /* starts/ends with */
    expect_int("starts_with", str_starts_with(f, str_from_cstr("abra")) ? 1 : 0, 1);
    expect_int("ends_with", str_ends_with(f, str_from_cstr("bra")) ? 1 : 0, 1);

    /* wildcard extensive checks (ASCII) */
    str_t wf = str_from_cstr("hello.c");
    expect_int("wc_exact", str_wildcard_ascii(wf, "hello.c") ? 1 : 0, 1);
    expect_int("wc_star", str_wildcard_ascii(wf, "*.c") ? 1 : 0, 1);
    expect_int("wc_qmark", str_wildcard_ascii(wf, "h?llo.c") ? 1 : 0, 1);
    expect_int("wc_class", str_wildcard_ascii(wf, "h[ae]llo.c") ? 1 : 0, 1);
    expect_int("wc_range", str_wildcard_ascii(wf, "h[a-z]llo.c") ? 1 : 0, 1);
    expect_int("wc_neg_class", str_wildcard_ascii(wf, "h[^e]llo.c") ? 1 : 0, 0);

    /* wildcard wide */
    str_t wwf = str_from_wcstr(L"Test123");
    expect_int("wc_wide_star", str_wildcard_wide(wwf, L"Test*") ? 1 : 0, 1);
    expect_int("wc_wide_qmark", str_wildcard_wide(wwf, L"T?st[0-9][0-9][0-9]") ? 1 : 0, 1);

    if (test_failures() == 0) {
        fprintf(stderr, "# all string tests passed\n");
        return 0;
    }
    fprintf(stderr, "# %d string test(s) failed\n", test_failures());
    return 1;
}
