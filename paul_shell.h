/* shellell

 https://github.com/takeiteasy/shellell

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
 @header paul_shell.h
 @copyright George Watson GPLv3
 @updated 2025-09-29
 @brief Embeddable bourne-like shell (posix+windows) 
 @discussion
    Implementation is included when PAUL_SHELL_IMPLEMENTATION or PAUL_IMPLEMENTATION is defined.
*/

#ifndef PAUL_SHELL_H
#define PAUL_SHELL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <processthreadsapi.h>
#include <synchapi.h>
#else
/* POSIX headers */
#include <unistd.h>
#include <fcntl.h>
#endif

/* Return codes for shell(): non-negative values are child exit codes.
 * Negative values are library error codes.
 */
#define SHELL_OK 0
#define SHELL_ERR_GENERIC -1
#define SHELL_ERR_TOKENIZE -2
#define SHELL_ERR_EVAL -3
#define SHELL_ERR_PIPE -4
#define SHELL_ERR_FORK -5
#define SHELL_ERR_READ -6

typedef void (*shell_stream_cb_t)(const char *data, size_t len, void *userdata);

typedef struct shell_io {
    /* I/O capture and streaming control structure
     *
     * - Pass a non-NULL `shell_io *` to `shell()` to enable capture/streaming.
     * - If `out_cb` and/or `err_cb` are set, `shell()` invokes the callback
     *   as chunks of data arrive on the child's STDOUT/STDERR. When callbacks
     *   are provided, `shell()` does NOT fill the `out`/`err` buffers (they
     *   will remain NULL). Callbacks are useful for streaming large outputs
     *   without buffering everything in memory.
     * - If callbacks are NULL, `shell()` accumulates the full contents of
     *   STDOUT and STDERR into frshellly allocated, NUL-terminated buffers
     *   stored in `out` and `err` respectively. The sizes are in `out_len`
     *   and `err_len`. The caller is responsible for calling `free()` on
     *   `out` and `err` when done.
     * - To provide input to the child, set `in` to a buffer of length
     *   `in_len`. `shell()` will write the contents of `in` to the child's
     *   STDIN and then close it. Ownership of `in` stays with the caller.
     * - Return values: non-negative return values are the child's exit code.
     *   Negative return values are library-level errors (see SHELL_ERR_*).
     */
    /* Captured output buffers (allocated by shell() when callbacks are NULL). */
    char *out;    /* captured stdout (NUL-terminated) */
    size_t out_len;
    char *err;    /* captured stderr (NUL-terminated) */
    size_t err_len;

    /* Input to be written to child's STDIN before closing it. Not modified by shell(). */
    const char *in;
    size_t in_len;

    /* Streaming callbacks. If non-NULL, shell() will call these with each
     * incoming chunk. Callbacks receive `userdata` as the last argument.
     * When callbacks are used, `out`/`err` buffers are not populated.
     */
    shell_stream_cb_t out_cb;
    shell_stream_cb_t err_cb;
    void *userdata;
} shell_io;

/*!
 @function shell
 @param cmd The shell command to execute
 @param io The io object to capture command output (nilable)
 @return Returns last command's status code
 @brief Main entry point: execute the command line `cmd` with I/O control via `io`. If `io` is NULL, no I/O capture or streaming is performed. Returns child's exit code (non-negative) or negative error code.
 */
int shell(const char *cmd, shell_io *io);
/*!
 @function shell_fmt
 @param io The io object to capture command output (nilable)
 @param fmt The shell command to format and execute
 @return Returns last command's status code
 */
int shell_fmt(shell_io *io, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif // PAUL_SHELL_H

#if defined(PAUL_SHELL_IMPLEMENTATION) || defined(PAUL_IMPLEMENTATION)
typedef enum shell_token_type {
    SHELL_TOKEN_ERROR,
    SHELL_TOKEN_EOL,
    SHELL_TOKEN_ATOM,
    SHELL_TOKEN_PIPE = '|',
    SHELL_TOKEN_AMPERSAND = '&',
    SHELL_TOKEN_GREATER = '>',
    SHELL_TOKEN_LESSER = '<',
    SHELL_TOKEN_SEMICOLON = ';'
} shell_token_type;

typedef struct shell_token {
    shell_token_type type;
    unsigned char *begin;
    int length;
} shell_token_t;

typedef struct shell_lexer {
    unsigned char *begin;
    struct {
        unsigned char *ptr;
        wchar_t ch;
        int ch_length;
    } cursor;
    char *error;
    unsigned char *input_begin; /* original input start for position reporting */
    size_t error_pos; /* byte offset into input where error occurred */
} shell_lexer_t;

typedef enum shell_ast_type {
    SHELL_AST_CMD,
    SHELL_AST_BACKGROUND,
    SHELL_AST_SEQ,
    SHELL_AST_REDIR_IN,
    SHELL_AST_REDIR_OUT,
    SHELL_AST_PIPE
} shell_ast_type_t;

typedef struct shell_ast {
    shell_ast_type_t type;
    shell_token_t *token;
    struct shell_ast *left, *right;
} shell_ast_t;

typedef struct shell_token_array {
    shell_token_t *data;
    size_t count;
    size_t capacity;
} shell_token_array_t;

typedef struct shell_parser {
    shell_token_array_t tokens;
    shell_token_t *current;
    size_t cursor;
} shell_parser;

static void eprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void _perror(const char *msg) {
    eprintf("shell: %s: %s\n", msg, strerror(errno));
}

static void die(const char *msg) {
    if (errno)
        _perror(msg);
    exit(EXIT_FAILURE);
}

static void *xmalloc(size_t size) {
    void *p = malloc(size);
    if (!p)
        die("malloc");
    return p;
}

static void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p)
        die("realloc");
    return p;
}

static int xopen(const char *path, int flags, mode_t mode) {
    int fd = open(path, flags, mode);
    if (fd == -1)
        die("open");
    return fd;
}

static int utf8read(const unsigned char* c, wchar_t* out) {
    wchar_t u = *c, l = 1;
    if ((u & 0xC0) == 0xC0) {
        int a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;
        if (a < 6 || !(u & 0x02)) {
            u = ((u << (a + 1)) & 0xFF) >> (a + 1);
            for (int b = 1; b < a; ++b)
                u = (u << 6) | (c[l++] & 0x3F);
        }
    }
    if (out)
        *out = u;
    return l;
}

static void shell_lexer(shell_lexer_t *l, unsigned char* line) {
    l->begin = line;
    l->cursor.ptr = line;
    l->cursor.ch_length = utf8read(line, &l->cursor.ch);
    l->error = NULL;
    l->input_begin = line;
    l->error_pos = (size_t)-1;
}

static inline wchar_t peek(shell_lexer_t *l) {
    return l->cursor.ch;
}

static inline bool is_eof(shell_lexer_t *l) {
    return peek(l) == '\0';
}

static inline void update(shell_lexer_t *l) {
    l->begin = l->cursor.ptr;
    l->cursor.ch_length = utf8read(l->cursor.ptr, &l->cursor.ch);
    l->error = NULL;
}

static inline wchar_t advance(shell_lexer_t *l) {
    l->cursor.ptr += utf8read(l->cursor.ptr, NULL);
    l->cursor.ch_length = utf8read(l->cursor.ptr, &l->cursor.ch);
    return l->cursor.ch;
}

#define ADVANCE(L, N) \
    do { \
        int n = (N); \
        while (!is_eof((L)) && n-- > 0) \
            advance((L)); \
    } while(0)

static inline wchar_t next(shell_lexer_t *l) {
    if (is_eof(l))
        return '\0';
    wchar_t next;
    utf8read(l->cursor.ptr + l->cursor.ch_length, &next);
    return next;
}

static void skip_whitespace(shell_lexer_t *l) {
    for (;;) {
        if (is_eof(l))
            return;
        switch (peek(l)) {
            case ' ':
            case '\t':
            case '\v':
            case '\r':
            case '\n':
            case '\f':
                advance(l);
                break;
            default:
                return;
        }
    }
}

static shell_token_t new_token(shell_lexer_t *l, shell_token_type type) {
    return (shell_token_t) {
        .type = type,
        .begin = l->begin,
        .length = (int)(l->cursor.ptr - l->begin)
    };
}

static shell_token_t read_atom(shell_lexer_t *l) {
    for (;;) {
        if (is_eof(l))
            goto BAIL;
        switch (peek(l)) {
            case ' ':
            case '\t':
            case '\v':
            case '\r':
            case '\n':
            case '\f':
            case '|':
            case '&':
            case '<':
            case '>':
            case ';':
                goto BAIL;
            default:
                advance(l);
                break;
        }
    }
BAIL:
    return new_token(l, SHELL_TOKEN_ATOM);
}

static shell_token_t read_token(shell_lexer_t *l) {
    if (is_eof(l))
        return new_token(l, SHELL_TOKEN_EOL);
    update(l);
    wchar_t wc = peek(l);
    switch (wc) {
        case ' ':
        case '\t':
        case '\v':
        case '\r':
        case '\n':
        case '\f':
            skip_whitespace(l);
            update(l);
            return read_token(l);
        case '"':
        case '\'': {
            wchar_t quote = wc;
            /* start after opening quote */
            unsigned char *start = l->cursor.ptr + l->cursor.ch_length;
            unsigned char *p = start;
            for (;;) {
                wchar_t ch;
                int len = utf8read(p, &ch);
                if (ch == '\0') {
                    /* unterminated quote -> error token */
                    l->error = "unterminated quote";
                    l->begin = start;
                    l->cursor.ptr = p;
                    /* report error position as byte offset from input_begin */
                    if (l->input_begin)
                        l->error_pos = (size_t)(p - l->input_begin);
                    return new_token(l, SHELL_TOKEN_ERROR);
                }
                if (ch == quote) {
                    /* set token begin/ptr to produce length without quotes */
                    l->begin = start;
                    l->cursor.ptr = p; /* points at closing quote */
                    shell_token_t tok = new_token(l, SHELL_TOKEN_ATOM);
                    /* consume closing quote */
                    advance(l);
                    return tok;
                }
                p += len;
            }
        }
        case '|':
        case '&':
        case '<':
        case '>':
        case ';':
            advance(l);
            return new_token(l, (shell_token_type)wc);
        default:
            return read_atom(l);
    }
    return new_token(l, SHELL_TOKEN_ERROR);
}

static inline const char* token_string(shell_token_t *t) {
    switch (t->type) {
        case SHELL_TOKEN_ERROR:
            return "ERROR";
        case SHELL_TOKEN_EOL:
            return "END";
        case SHELL_TOKEN_ATOM:
            return "ATOM";
        case SHELL_TOKEN_PIPE:
            return "PIPE";
        case SHELL_TOKEN_AMPERSAND:
            return "AMPERSAND";
        case SHELL_TOKEN_GREATER:
            return "GREATER";
        case SHELL_TOKEN_LESSER:
            return "LESSER";
        case SHELL_TOKEN_SEMICOLON:
            return "SEMICOLON";
    }
}

static void shell_token_array_init(shell_token_array_t *arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

static bool shell_token_array_append(shell_token_array_t *arr, shell_token_t token) {
    if (arr->count + 1 > arr->capacity) {
        size_t new_capacity = arr->capacity == 0 ? 8 : arr->capacity * 2;
        shell_token_t *new_data = xrealloc(arr->data, sizeof(shell_token_t) * new_capacity);
        if (!new_data)
            return false;
        arr->data = new_data;
        arr->capacity = new_capacity;
    }
    arr->data[arr->count++] = token;
    return true;
}

static shell_token_array_t shell_parse(shell_lexer_t *l) {
    shell_token_array_t tokens;
    shell_token_array_init(&tokens);
    for (;;) {
        shell_token_t token = read_token(l);
        switch (token.type) {
            default:
                l->error = "unknown token";
            case SHELL_TOKEN_ERROR:
            case SHELL_TOKEN_EOL:
                goto BAIL;
            case SHELL_TOKEN_ATOM:
            case SHELL_TOKEN_PIPE:
            case SHELL_TOKEN_AMPERSAND:
            case SHELL_TOKEN_GREATER:
            case SHELL_TOKEN_LESSER:
            case SHELL_TOKEN_SEMICOLON:
                if (!shell_token_array_append(&tokens, token))
                    l->error = "out of memory";
                break;
        }
    }
BAIL:
    return tokens;
}

static inline shell_ast_t* new_ast(void) {
    shell_ast_t *result = malloc(sizeof(shell_ast_t));
    memset(result, 0, sizeof(shell_ast_t));
    return result;
}

static void free_ast(shell_ast_t *node) {
    if (!node)
        return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

static inline const char* ast_string(shell_ast_type_t t) {
    switch (t) {
        case SHELL_AST_CMD:
            return "COMMAND";
        case SHELL_AST_BACKGROUND:
            return "BACKGROUND";
        case SHELL_AST_SEQ:
            return "SEQUENCE";
        case SHELL_AST_REDIR_IN:
            return "REDIRECT_IN";
        case SHELL_AST_REDIR_OUT:
            return "REDIRECT_OUT";
        case SHELL_AST_PIPE:
            return "PIPE";
    }
}

static void shell_dump(shell_ast_t *node, int level) {
    if (!node)
        return;
    for (int i = 0; i < level; i++)
        printf("  ");
    if (!node->token)
        printf("[%s]\n", ast_string(node->type));
    else
        printf("[%s(%s)]\n", token_string(node->token), ast_string(node->type));

    shell_dump(node->right, level + 1);
    shell_dump(node->left, level + 1);
}

static shell_token_t* parser_peek(shell_parser *p) {
    return p->cursor < p->tokens.count ? &p->tokens.data[p->cursor] : NULL;
}

static shell_token_t* parser_next(shell_parser *p) {
    if (p->cursor + 1 >= p->tokens.count)
        return NULL;
    ++p->cursor;
    return &p->tokens.data[p->cursor];
}

static int match_token(shell_parser *p, shell_token_type type) {
        shell_token_t *t = parser_peek(p);
        return t != NULL && t->type == type;
}

static int expect_token(shell_parser *p, shell_token_type type) {
    shell_token_t *t = parser_peek(p);
    return t != NULL && t->type == type;
}

static shell_ast_t *simple_command(shell_parser *p) {
    shell_token_t *t = parser_peek(p);
    if (t->type != SHELL_TOKEN_ATOM)
        return NULL;
    shell_ast_t *ast = new_ast();
    ast->type = SHELL_AST_CMD;
    ast->token = t;
    if (parser_next(p) != NULL)
        ast->right = simple_command(p);
    return ast;
}

static inline shell_ast_t *handle_redirection(shell_parser *p, shell_ast_type_t type, shell_ast_t *simple) {
    if (!expect_token(p, SHELL_TOKEN_ATOM))
        return NULL;
    shell_ast_t *ast = new_ast();
    ast->type = type;
    ast->right = simple;
    ast->token = parser_next(p);
    parser_next(p);
    return ast;
}

static shell_ast_t *command(shell_parser *p) {
    shell_ast_t *simple = simple_command(p);
    if (!simple)
        return NULL;
    if (match_token(p, SHELL_TOKEN_GREATER)) {
        // <simple command> '>' <filename>
        shell_ast_t *ast = handle_redirection(p, SHELL_AST_REDIR_OUT, simple);
        if (ast == NULL)
            free_ast(simple);
        return ast;
    }
    if (match_token(p, SHELL_TOKEN_LESSER)) {
        // <simple command> '<' <filename>
        shell_ast_t *ast = handle_redirection(p, SHELL_AST_REDIR_IN, simple);
        if (!ast)
            free_ast(simple);
        return ast;
    }
    // <simple command>
    return simple;
}

static shell_ast_t *_pipe(shell_parser *p) {
    shell_ast_t *left = command(p);
    if (left == NULL)
        return NULL;
    // <command> '|' <pipe>
    if (match_token(p, SHELL_TOKEN_PIPE)) {
        if (!expect_token(p, SHELL_TOKEN_ATOM)) {
            free_ast(left);
            return NULL;
        }
        shell_ast_t *ast = new_ast();
        ast->type = SHELL_AST_PIPE;
        ast->left = left;
        parser_next(p);
        ast->right = _pipe(p);
        return ast;
    }
    // <command>
    return left;
}

static shell_ast_t *full_command(shell_parser *p) {
    shell_ast_t *left = _pipe(p);
    if (left == NULL)
        return NULL;
    // <pipe> '&' <full command> | <pipe> '&'
    if (match_token(p, SHELL_TOKEN_AMPERSAND)) {
        if (!expect_token(p, SHELL_TOKEN_ATOM) && parser_peek(p) != NULL) {
            free_ast(left);
            return NULL;
        }
        shell_ast_t *ast = new_ast();
        ast->type = SHELL_AST_BACKGROUND;
        ast->left = left;
        parser_next(p);
        ast->right = full_command(p);
        return ast;
    }
    // <pipe> ';' <full command> | <pipe> ';'
    if (match_token(p, SHELL_TOKEN_SEMICOLON)) {
        if (!expect_token(p, SHELL_TOKEN_ATOM) && parser_peek(p) != NULL) {
            free_ast(left);
            return NULL;
        }
        shell_ast_t *ast = new_ast();
        ast->type = SHELL_AST_SEQ;
        ast->left = left;
        parser_next(p);
        ast->right = full_command(p);
        return ast;
    }
    return left;
}

static shell_ast_t* shell_eval(shell_token_array_t tokens) {
    shell_parser parser = {
        .tokens = tokens,
        .cursor = 0
    };
    shell_ast_t *result = full_command(&parser);
    return tokens.count == 0 ? NULL : parser.cursor == tokens.count - 1 ? result : NULL;
}

typedef struct shell_command {
    int argc;
    char **argv;
    int input_fd;
    int output_fd;
    int bg;
} shell_command_t;

static int input_fd = -1;
static int output_fd = -1;
static int bg = 0;

static void command_argv_from_ast(shell_command_t *cmd, shell_ast_t *ast) {
    // Initialize command fields
    cmd->argc = 0;
    cmd->argv = NULL;

    // Count arguments safely
    for (shell_ast_t *n = ast; n != NULL; n = n->right)
        cmd->argc++;

    if (cmd->argc == 0) {
        cmd->argv = xmalloc(sizeof(char *));
        cmd->argv[0] = NULL;
        return;
    }

    cmd->argv = xmalloc(sizeof(char *) * (cmd->argc + 1));

    // Copy each token into a NUL-terminated C string
    for (int i = 0; i < cmd->argc; i++) {
        if (ast == NULL || ast->token == NULL) {
            // Defensive: ensure we don't dereference a NULL ast/token
            cmd->argv[i] = xmalloc(1);
            cmd->argv[i][0] = '\0';
        } else {
            size_t len = (size_t)ast->token->length;
            size_t size = len + 1; // room for terminating NUL
            cmd->argv[i] = xmalloc(size);
            memcpy(cmd->argv[i], ast->token->begin, len);
            cmd->argv[i][len] = '\0';
        }
        ast = ast ? ast->right : NULL;
    }
    cmd->argv[cmd->argc] = NULL;
}

typedef struct shell_builtin {
  char *name;
  void (*func)(int argc, char **argv);
} shell_builtin_t;

static void builtin_exit(int argc, char **argv) {
    (void)argc;
    (void)argv;
    exit(0);
}

static void builtin_cd(int argc, char **argv) {
    if (argc == 1) {
        if (chdir(getenv("HOME")) == -1)
            perror("cd");
    } else if (argc == 2) {
        if (chdir(argv[1]) == -1)
            perror("cd");
    } else
        eprintf("cd: too many arguments\n");
}

static void builtin_pwd(int argc, char **argv) {
    (void)argv;
    if (argc > 1) {
        eprintf("pwd: too many arguments\n");
        return;
    }
    char *cwd = getcwd(NULL, 0);
    printf("%s\n", cwd);
    free(cwd);
}

static shell_builtin_t builtins[] = {
    {"exit", &builtin_exit},
    {"cd", &builtin_cd},
    {"pwd", &builtin_pwd},
};

static shell_builtin_t *builtin_find_by_name(char *name) {
    for (size_t i = 0; i < sizeof(builtins) / sizeof(shell_builtin_t); i++)
        if (strcmp(builtins[i].name, name) == 0)
            return &builtins[i];
    return NULL;
}

static void builtin_execute(shell_builtin_t *builtin, int argc, char **argv) {
    if (builtin)
        builtin->func(argc, argv);
}

static void command_execute(shell_command_t *cmd) {
    if (cmd->argc == 0)
        return;

    // Builtins
    shell_builtin_t *builtin = builtin_find_by_name(cmd->argv[0]);
    if (builtin) {
        builtin_execute(builtin, cmd->argc, cmd->argv);
        return;
    }

    // Fork
    pid_t pid = fork();
    if (pid == -1) {
        _perror("fork");
        return;
    }

    // Child
    if (pid == 0) {
        if (cmd->bg) {
            printf("\n[BACKGROUND] started backgroundjob: %s\n", cmd->argv[0]);
            setpgid(0, 0);

            int fd = xopen("/dev/null", O_RDONLY, 0);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Pipe
        if (cmd->input_fd != -1) {
            dup2(cmd->input_fd, STDIN_FILENO);
            close(cmd->input_fd);
        }
        if (cmd->output_fd != -1) {
            dup2(cmd->output_fd, STDOUT_FILENO);
            close(cmd->output_fd);
        }

        // Execute
        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            die("execvp");
        }
    }
    // Wait for child
    if (!cmd->bg) {
        int status;
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        return;
    }

    // Background process
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // Restart interrupted syscalls,
    // don't handle stopped children
    sigaction(SIGCHLD, &sa, NULL);
}

static void command_destroy(shell_command_t *cmd) {
    if (cmd->argv) {
        for (int i = 0; i < cmd->argc; i++)
            free(cmd->argv[i]);
        free(cmd->argv);
    }
}

static void eval_commandtail(shell_ast_t *ast) {
    shell_command_t cmd;
    command_argv_from_ast(&cmd, ast);
    cmd.input_fd = input_fd;
    cmd.output_fd = output_fd;
    cmd.bg = bg;
    command_execute(&cmd);
    command_destroy(&cmd);
}

static void eval_sequence(shell_ast_t *ast) {
    if (ast->type == SHELL_AST_BACKGROUND)
        bg = 1;
    shell_exec(ast->left);
    shell_exec(ast->right);
    if (ast->type == SHELL_AST_BACKGROUND)
        bg = 0;
}

static void eval_redirection(shell_ast_t *ast) {
    int fd;
    unsigned char c = ast->token->begin[ast->token->length];
    ast->token->begin[ast->token->length] = '\0';
    if (ast->type == SHELL_AST_REDIR_IN) {
        fd = open((const char*)ast->token->begin, O_RDONLY);
        if (fd == -1) {
            _perror("open");
            return;
        }
        input_fd = fd;
    } else {
        fd = open((const char*)ast->token->begin,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1) {
            _perror("open");
            return;
        }
        output_fd = fd;
    }
    ast->token->begin[ast->token->length] = c;

    shell_exec(ast->right);
    close(fd);
    input_fd = -1;
    output_fd = -1;
}

static void eval_pipeline(shell_ast_t *ast) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        _perror("pipe");
        return;
    }

    input_fd = -1;
    output_fd = pipefd[1];
    shell_exec(ast->left);
    close(pipefd[1]);

    ast = ast->right;

    input_fd = pipefd[0];

    while (ast->type == SHELL_AST_PIPE) {
        if (pipe(pipefd) == -1) {
            _perror("pipe");
            return;
        }
        output_fd = pipefd[1];
        shell_exec(ast->left);
        close(pipefd[1]);
        close(input_fd);
        input_fd = pipefd[0];
        ast = ast->right;
    }
    output_fd = -1;
    input_fd = pipefd[0];
    shell_exec(ast);
    close(pipefd[0]);
}

static int shell_exec(shell_ast_t *ast) {
    if (!ast)
        goto BAIL;
    switch (ast->type) {
        case SHELL_AST_PIPE:
            eval_pipeline(ast);
            break;
        case SHELL_AST_REDIR_IN:
        case SHELL_AST_REDIR_OUT:
            eval_redirection(ast);
            break;
        case SHELL_AST_SEQ:
        case SHELL_AST_BACKGROUND:
            eval_sequence(ast);
            break;
        case SHELL_AST_CMD:
            eval_commandtail(ast);
            break;
        default:
            break;
    }
BAIL:
    return EXIT_SUCCESS;
}

#if defined(_WIN32) || defined(_WIN64)
/* Windows-specific implementation */

typedef struct {
    HANDLE handle;
    shell_stream_cb_t callback;
    void *userdata;
    char **buffer;
    size_t *length;
    size_t *capacity;
    int use_callback;
} win_reader_args_t;

static DWORD WINAPI win_reader_thread(LPVOID arg) {
    win_reader_args_t *args = (win_reader_args_t*)arg;
    char buffer[4096];
    DWORD bytes_read = 0;
    
    while (ReadFile(args->handle, buffer, sizeof(buffer), &bytes_read, NULL) && bytes_read > 0)
        if (args->use_callback) {
            args->callback(buffer, bytes_read, args->userdata);
        } else {
            // Ensure buffer has enough capacity
            if (*args->capacity - *args->length < bytes_read) {
                size_t new_capacity = (*args->capacity == 0) ? 4096 : *args->capacity * 2;
                while (new_capacity - *args->length < bytes_read) {
                    new_capacity *= 2;
                }
                *args->buffer = xrealloc(*args->buffer, new_capacity + 1);
                *args->capacity = new_capacity;
            }
            memcpy(*args->buffer + *args->length, buffer, bytes_read);
            *args->length += bytes_read;
        }
    return 0;
}

static int create_pipe_pair(HANDLE *read_handle, HANDLE *write_handle, int inherit_read) {
    SECURITY_ATTRIBUTES sa = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .bInheritHandle = TRUE,
        .lpSecurityDescriptor = NULL
    };
    
    if (!CreatePipe(read_handle, write_handle, &sa, 0))
        return SHELL_ERR_PIPE;
    
    // Set inheritance based on parameter
    HANDLE non_inherit = inherit_read ? *write_handle : *read_handle;
    SetHandleInformation(non_inherit, HANDLE_FLAG_INHERIT, 0);
    
    return SHELL_OK;
}

static void cleanup_handles(HANDLE *handles, size_t count) {
    for (size_t i = 0; i < count; i++)
        if (handles[i] != NULL && handles[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(handles[i]);
            handles[i] = NULL;
        }
}

static int win_shell_implementation(const char *cmd, shell_io *io) {
    if (!cmd)
        return SHELL_ERR_GENERIC;

    HANDLE pipes[6] = {0}; // stdin_r, stdin_w, stdout_r, stdout_w, stderr_r, stderr_w
    HANDLE *stdin_r = &pipes[0], *stdin_w = &pipes[1];
    HANDLE *stdout_r = &pipes[2], *stdout_w = &pipes[3]; 
    HANDLE *stderr_r = &pipes[4], *stderr_w = &pipes[5];
    
    // Create pipes
    int result;
    if ((result = create_pipe_pair(stdout_r, stdout_w, 0)) != SHELL_OK ||
        (result = create_pipe_pair(stderr_r, stderr_w, 0)) != SHELL_OK ||
        (result = create_pipe_pair(stdin_r, stdin_w, 0)) != SHELL_OK) {
        cleanup_handles(pipes, 6);
        return result;
    }
    
    // Prepare command line
    size_t cmd_len = strlen(cmd);
    char *cmdline = xmalloc(cmd_len + 1);
    strcpy(cmdline, cmd);
    
    // Setup process creation structures
    STARTUPINFOA si = {
        .cb = sizeof(STARTUPINFOA),
        .dwFlags = STARTF_USESTDHANDLES,
        .hStdInput = *stdin_r,
        .hStdOutput = *stdout_w,
        .hStdError = *stderr_w
    };
    
    PROCESS_INFORMATION pi = {0};
    
    // Create process
    BOOL success = CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    free(cmdline);
    
    if (!success) {
        cleanup_handles(pipes, 6);
        return SHELL_ERR_FORK;
    }
    
    // Close child's handles in parent
    CloseHandle(*stdout_w); *stdout_w = NULL;
    CloseHandle(*stderr_w); *stderr_w = NULL;
    CloseHandle(*stdin_r); *stdin_r = NULL;
    
    // Setup reader threads
    char *outbuf = NULL, *errbuf = NULL;
    size_t out_len = 0, out_cap = 0, err_len = 0, err_cap = 0;
    int use_out_cb = io && io->out_cb != NULL;
    int use_err_cb = io && io->err_cb != NULL;
    
    win_reader_args_t out_args = {
        .handle = *stdout_r,
        .callback = io ? io->out_cb : NULL,
        .userdata = io ? io->userdata : NULL,
        .buffer = &outbuf,
        .length = &out_len,
        .capacity = &out_cap,
        .use_callback = use_out_cb
    };
    
    win_reader_args_t err_args = {
        .handle = *stderr_r,
        .callback = io ? io->err_cb : NULL,
        .userdata = io ? io->userdata : NULL,
        .buffer = &errbuf,
        .length = &err_len,
        .capacity = &err_cap,
        .use_callback = use_err_cb
    };
    
    HANDLE threads[2];
    threads[0] = CreateThread(NULL, 0, win_reader_thread, &out_args, 0, NULL);
    threads[1] = CreateThread(NULL, 0, win_reader_thread, &err_args, 0, NULL);
    
    // Write input if provided
    if (io && io->in && io->in_len > 0) {
        DWORD written;
        WriteFile(*stdin_w, io->in, (DWORD)io->in_len, &written, NULL);
    }
    CloseHandle(*stdin_w); *stdin_w = NULL;
    
    // Wait for completion
    WaitForSingleObject(pi.hProcess, INFINITE);
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    
    // Get exit code
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    
    // Populate io structure
    if (io) {
        if (!use_out_cb) {
            io->out = outbuf ? (outbuf[out_len] = '\0', outbuf) : (outbuf = xmalloc(1), outbuf[0] = '\0', outbuf);
            io->out_len = out_len;
        } else {
            io->out = NULL;
            io->out_len = 0;
        }
        
        if (!use_err_cb) {
            io->err = errbuf ? (errbuf[err_len] = '\0', errbuf) : (errbuf = xmalloc(1), errbuf[0] = '\0', errbuf);
            io->err_len = err_len;
        } else {
            io->err = NULL;
            io->err_len = 0;
        }
    }
    
    // Cleanup
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    cleanup_handles(pipes, 6);
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    
    return (int)exit_code;
}

#else
/* POSIX implementation */

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return flags == -1 ? -1 : fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void close_pipe_pair(int pipe_fds[2]) {
    if (pipe_fds[0] != -1) {
        close(pipe_fds[0]);
        pipe_fds[0] = -1;
    }
    if (pipe_fds[1] != -1) {
        close(pipe_fds[1]);
        pipe_fds[1] = -1;
    }
}

static int write_input_to_child(int write_fd, const char *input, size_t input_len) {
    size_t written = 0;
    while (written < input_len) {
        ssize_t result = write(write_fd, input + written, input_len - written);
        if (result == -1) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            return -1;
        }
        written += result;
    }
    return 0;
}

static int ensure_buffer_capacity(char **buffer, size_t *capacity, size_t current_len, size_t needed) {
    if (*capacity - current_len >= needed)
        return 0;

    size_t new_capacity = (*capacity == 0) ? 4096 : *capacity * 2;
    while (new_capacity - current_len < needed)
        new_capacity *= 2;
    
    *buffer = xrealloc(*buffer, new_capacity + 1);
    *capacity = new_capacity;
    return 0;
}

static int posix_shell_with_io(const char *cmd, shell_io *io) {
    int pipes[6] = {-1, -1, -1, -1, -1, -1}; // in[2], out[2], err[2]
    int *inpipe = &pipes[0], *outpipe = &pipes[2], *errpipe = &pipes[4];
    
    // Create pipes
    if (pipe(inpipe) == -1 || pipe(outpipe) == -1 || pipe(errpipe) == -1) {
        for (int i = 0; i < 6; i += 2)
            close_pipe_pair(&pipes[i]);
        return SHELL_ERR_PIPE;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        for (int i = 0; i < 6; i += 2)
            close_pipe_pair(&pipes[i]);
        return SHELL_ERR_FORK;
    }
    
    if (pid == 0) {
        // Child process
        close(inpipe[1]);   // Close write end of input
        close(outpipe[0]);  // Close read end of output
        close(errpipe[0]);  // Close read end of error
        
        // Redirect standard file descriptors
        if (dup2(inpipe[0], STDIN_FILENO) == -1 ||
            dup2(outpipe[1], STDOUT_FILENO) == -1 ||
            dup2(errpipe[1], STDERR_FILENO) == -1) {
            die("dup2");
        }
        
        close(inpipe[0]);
        close(outpipe[1]);
        close(errpipe[1]);
        
        // Parse and execute command
        shell_lexer_t lexer;
        shell_lexer(&lexer, (unsigned char*)cmd);
        shell_token_array_t tokens = shell_parse(&lexer);
        
        if (!tokens.data || lexer.error) {
            if (tokens.data)
                free(tokens.data);
            _exit(SHELL_ERR_TOKENIZE);
        }
        
        shell_ast_t *ast = shell_eval(tokens);
        if (!ast) {
            free(tokens.data);
            _exit(SHELL_ERR_EVAL);
        }
        
        int result = shell_exec(ast);
        free(tokens.data);
        free_ast(ast);
        _exit(result == 0 ? SHELL_OK : result);
    }
    
    // Parent process
    close(inpipe[0]);   // Close read end of input
    close(outpipe[1]);  // Close write end of output  
    close(errpipe[1]);  // Close write end of error
    
    // Set pipes to non-blocking
    set_nonblocking(inpipe[1]);
    set_nonblocking(outpipe[0]);
    set_nonblocking(errpipe[0]);
    
    // Write input if provided
    if (io->in && io->in_len > 0)
        write_input_to_child(inpipe[1], io->in, io->in_len);
    close(inpipe[1]);
    
    // Setup buffers and callbacks
    char *outbuf = NULL, *errbuf = NULL;
    size_t out_len = 0, out_cap = 0, err_len = 0, err_cap = 0;
    int use_out_cb = io->out_cb != NULL;
    int use_err_cb = io->err_cb != NULL;
    
    // Poll for output
    struct pollfd fds[2] = {
        {.fd = outpipe[0], .events = POLLIN | POLLHUP},
        {.fd = errpipe[0], .events = POLLIN | POLLHUP}
    };
    
    int active_fds = 2;
    while (active_fds > 0) {
        int poll_result = poll(fds, 2, -1);
        if (poll_result == -1) {
            if (errno == EINTR)
                continue;
            break; // Fatal error
        }
        
        for (int i = 0; i < 2; i++) {
            if (!(fds[i].revents & (POLLIN | POLLHUP)) || fds[i].fd == -1)
                continue;

            char buffer[4096];
            ssize_t bytes_read;
            
            while ((bytes_read = read(fds[i].fd, buffer, sizeof(buffer))) > 0) {
                if (i == 0) { // stdout
                    if (use_out_cb)
                        io->out_cb(buffer, bytes_read, io->userdata);
                    else {
                        ensure_buffer_capacity(&outbuf, &out_cap, out_len, bytes_read);
                        memcpy(outbuf + out_len, buffer, bytes_read);
                        out_len += bytes_read;
                    }
                } else { // stderr
                    if (use_err_cb)
                        io->err_cb(buffer, bytes_read, io->userdata);
                    else {
                        ensure_buffer_capacity(&errbuf, &err_cap, err_len, bytes_read);
                        memcpy(errbuf + err_len, buffer, bytes_read);
                        err_len += bytes_read;
                    }
                }
            }
            
            if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN && errno != EINTR)) {
                fds[i].fd = -1;
                active_fds--;
            }
        }
    }
    
    // Finalize buffers
    if (!use_out_cb) {
        io->out = outbuf ? (outbuf[out_len] = '\0', outbuf) : (outbuf = xmalloc(1), outbuf[0] = '\0', outbuf);
        io->out_len = out_len;
    } else {
        io->out = NULL;
        io->out_len = 0;
    }
    
    if (!use_err_cb) {
        io->err = errbuf ? (errbuf[err_len] = '\0', errbuf) : (errbuf = xmalloc(1), errbuf[0] = '\0', errbuf);
        io->err_len = err_len;
    } else {
        io->err = NULL;
        io->err_len = 0;
    }
    
    close(outpipe[0]);
    close(errpipe[0]);
    
    // Wait for child and get exit status
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static int posix_shell_inline(const char *cmd) {
    shell_lexer_t lexer;
    shell_lexer(&lexer, (unsigned char*)cmd);
    
    shell_token_array_t tokens = shell_parse(&lexer);
    if (!tokens.data || lexer.error) {
        if (lexer.error) {
            const char *error_fmt = (lexer.error_pos != (size_t)-1) 
                ? "error: '%s' at byte %zu\n" 
                : "error: '%s'\n";
            printf(error_fmt, lexer.error, lexer.error_pos);
        } else
            printf("error: failed to tokenize\n");
        if (tokens.data)
            free(tokens.data);
        return -1;
    }
    
    shell_ast_t *ast = shell_eval(tokens);
    if (!ast) {
        printf("error: failed to evaluate\n");
        free(tokens.data);
        return -1;
    }
    
    int result = shell_exec(ast);
    free(tokens.data);
    free_ast(ast);
    return result;
}

#endif /* _WIN32 */

int shell(const char *cmd, shell_io *io) {
    if (!cmd)
        return SHELL_ERR_GENERIC;

#if defined(_WIN32) || defined(_WIN64)
    return win_shell_implementation(cmd, io);
#else
    return io ? posix_shell_with_io(cmd, io) : posix_shell_inline(cmd);
#endif
}

int shell_fmt(shell_io *io, const char *fmt, ...) {
    if (!fmt)
        return SHELL_ERR_GENERIC;

    va_list args;
    va_start(args, fmt);
    char *cmd = NULL;
    if (vasprintf(&cmd, fmt, args) == -1) {
        va_end(args);
        return SHELL_ERR_GENERIC;
    }
    va_end(args);

    int result = shell(cmd, io);
    free(cmd);
    return result;
}
#endif // SHELL_IMPLEMENTATION
