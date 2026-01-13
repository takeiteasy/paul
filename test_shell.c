#define PAUL_SHELL_IMPLEMENTATION
#include "paul_shell.h"
#include <stdio.h>
#include <assert.h>

void my_echo(int argc, char **argv) {
    printf("CUSTOM ECHO: ");
    for (int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], i == argc - 1 ? "" : " ");
    }
    printf("\n");
}

int main() {
    printf("--- Test 1: Standard shell() ---\n");
    shell("echo Hello World", NULL);

    printf("\n--- Test 2: Custom Context with Builtin Override ---\n");
    shell_ctx *ctx = shell_ctx_create();
    shell_ctx_add_builtin(ctx, "echo", my_echo);
    shell_with_ctx("echo Hello World", NULL, ctx);

    printf("\n--- Test 3: Command Blacklist (rm) ---\n");
    shell_ctx_blacklist_cmd(ctx, "rm");
    shell_with_ctx("rm -rf /", NULL, ctx); 

    printf("\n--- Test 4: Path Blacklist (/etc/passwd) ---\n");
    shell_ctx_blacklist_path(ctx, "/etc/passwd");
    printf("Attempting: cat /etc/passwd\n");
    shell_with_ctx("cat /etc/passwd", NULL, ctx); 
    printf("Attempting: cat < /etc/passwd\n");
    shell_with_ctx("cat < /etc/passwd", NULL, ctx);

    printf("\n--- Test 5: Builtin Only Mode ---\n");
    ctx->builtin_only = true;
    shell_with_ctx("ls", NULL, ctx); 
    shell_with_ctx("echo I am allowed", NULL, ctx); 

    shell_ctx_destroy(ctx);
    
    printf("\n--- Test 6: Default Context Global ---\n");
    ctx = shell_ctx_create();
    shell_ctx_add_builtin(ctx, "foo", my_echo); 
    shell_set_default_ctx(ctx);
    shell("foo Using Global Default", NULL);
    shell_set_default_ctx(NULL);
    shell_ctx_destroy(ctx);

    return 0;
}