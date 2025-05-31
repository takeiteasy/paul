#include "jeff.h"
#include <stdio.h>

void test_enter(void) {
    printf("test enter\n");
}

void test_exit(void) {
    printf("test exit\n");
}

void test_event(const sapp_event *event) {
    printf("test event\n");
}

void test_step(void) {
    printf("test step\n");
}
