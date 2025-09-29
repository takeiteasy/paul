#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test_common.h"

#define PAUL_OS_IMPLEMENTATION
#include "../paul_os.h"

int main(void) {
    printf("1..7\n"); /* simple TAP header (expected tests) */

    /* environment variable */
    setenv("PAUL_TEST_VAR", "hello", 1);
    char *val = environment_variable("PAUL_TEST_VAR");
    expect_str("environment_variable", val, "hello");
    if (val)
        free(val);

    /* temp file / file operations */
    char tmpl[] = "tests/tmpfileXXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        perror("mkstemp");
        return 2;
    }
    close(fd);

    expect_int("file_exists_after_create", file_exists(tmpl) ? 1 : 0, 1);

    file_t f = io_invalid;
    if (!io_open(&f, tmpl, FILE_WRITE)) {
        fprintf(stderr, "FAIL io_open FILE_WRITE\n");
        TEST_FAILURES++;
    } else {
        io_write_string(f, "paul_test\n");
        io_close(&f);
    }

    int sz = file_size(tmpl);
    expect_int("file_size_positive", sz > 0 ? 1 : 0, 1);

    size_t readsz = 0;
    const char *content = file_read(tmpl, &readsz);
    if (content) {
        expect_int("file_read_size", (int)readsz, sz);
        free((void *)content);
    } else {
        fprintf(stderr, "FAIL file_read returned NULL\n");
        TEST_FAILURES++;
    }

    /* Debugging: show the temp filename and deletion result */
    printf("# tmpfile='%s'\n", tmpl);
    int del = file_delete(tmpl) ? 1 : 0;
    if (!del)
        perror("file_delete");
    expect_int("file_delete", del, 1);
    expect_int("file_exists_after_delete", file_exists(tmpl) ? 1 : 0, 0);

    /* path helpers */
    const char *joined = path_join("a", "b");
    expect_str("path_join", joined, "a/b");
    if (joined)
        free((void *)joined);

    const char *ext = path_get_file_extension("foo.txt");
    expect_str("path_get_file_extension", ext, "txt");

    const char *fname = path_get_file_name("/tmp/foo.txt");
    expect_str("path_get_file_name", fname, "foo.txt");

    char *noext = (char *)path_get_file_name_no_extension("/tmp/foo.txt");
    expect_str("path_get_file_name_no_extension", noext, "foo");
    if (noext)
        free(noext);

    /* shell execution */
    shell_io io = {0};
    int rc = shell("echo hello", &io);
    expect_int("shell_exitcode", rc, 0);
    if (io.out) {
        if (strstr(io.out, "hello") != NULL)
            printf("ok shell_stdout_contains_hello\n");
        else {
            fprintf(stderr, "FAIL shell_stdout_contains_hello: got '%s'\n", io.out);
            TEST_FAILURES++;
        }
        free(io.out);
    } else {
        fprintf(stderr, "FAIL shell no stdout captured\n");
        TEST_FAILURES++;
    }
    if (io.err)
        free(io.err);

    if (test_failures() == 0) {
        fprintf(stderr, "# all tests passed\n");
        /* continue to additional tests (glob/resolve) below */
    } else {
        fprintf(stderr, "# %d test(s) failed\n", test_failures());
        return 1;
    }

    /* --- Additional tests: glob and path_resolve --- */

    /* Setup a small directory tree under tests/tmp_glob */
    const char *base_dir = "tests/tmp_glob";
    directory_create(base_dir, true);
    const char *f1 = "tests/tmp_glob/a.txt";
    const char *f2 = "tests/tmp_glob/b.log";
    const char *subdir = "tests/tmp_glob/sub";
    directory_create(subdir, true);
    const char *f3 = "tests/tmp_glob/sub/c.txt";

    file_t tf = io_invalid;
    if (io_open(&tf, f1, FILE_WRITE)) {
        io_write_string(tf, "1\n");
        io_close(&tf);
    }
    if (io_open(&tf, f2, FILE_WRITE)) {
        io_write_string(tf, "2\n");
        io_close(&tf);
    }
    if (io_open(&tf, f3, FILE_WRITE)) {
        io_write_string(tf, "3\n");
        io_close(&tf);
    }

    /* directory_glob: list *.txt under tests/tmp_glob (non-recursive) */
    int count = 0;
    const char **matches = directory_glob("tests/tmp_glob/*.txt", &count);
    if (matches) {
        if (count >= 1)
            printf("ok directory_glob_count\n");
        else {
            fprintf(stderr, "FAIL directory_glob_count: got %d want >=1\n", count);
            TEST_FAILURES++;
        }
        for (int i = 0; i < count; i++)
            free((void *)matches[i]);
        free((void *)matches);
    } else {
        /* directory_glob returned NULL -> no matches */
        printf("ok directory_glob_count (no matches)\n");
    }

    /* path_resolve: test relative and .. components. Use path_get_working_directory to create a known cwd */
    const char *cwd = path_get_working_directory();
    if (cwd) {
        printf("# calling path_resolve test, cwd=%s\n", cwd);
        const char *joined = path_join(cwd, "tests/tmp_glob/../tmp_glob/sub/../a.txt");
        char *resolved = (char *)path_resolve(joined);
        if (resolved) {
            const char *basename = path_get_file_name(resolved);
            expect_str("path_resolve_basename", basename, "a.txt");
            free((void *)resolved);
        } else {
            fprintf(stderr, "FAIL path_resolve returned NULL\n");
            TEST_FAILURES++;
        }
        if (joined)
            free((void *)joined);
        printf("# path_resolve done\n");
    } else {
        fprintf(stderr, "FAIL path_get_working_directory returned NULL\n");
        TEST_FAILURES++;
    }

    /* Cleanup created files and directories */
    file_delete(f1);
    file_delete(f2);
    file_delete(f3);
    directory_delete(subdir, false, false);
    directory_delete(base_dir, true, false);

    if (test_failures() == 0) {
        fprintf(stderr, "# all tests including glob/resolve passed\n");
        return 0;
    }
    fprintf(stderr, "# %d test(s) failed (including glob/resolve)\n", test_failures());
    return 1;
}
