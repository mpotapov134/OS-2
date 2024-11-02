#include <stdio.h>
#include <unistd.h>

#include "uthread.h"


void thread_routine(void *arg) {
    while (1) {
        printf("%s\n", (char *) arg);
        sleep(1);
        uthread_schedule();
    }
}


int main(int argc, char **argv) {
    uthread_t thread1;
    uthread_t thread2;
    uthread_t thread3;
    struct uthread main_thread = {.name = "main thread"};

    uthreads[0] = &main_thread;
    thread_count = 1;

    uthread_create(&thread1, "uthread-1", thread_routine, (void *) "Hello");
    uthread_create(&thread2, "uthread-2", thread_routine, (void *) "world");
    uthread_create(&thread3, "uthread-3", thread_routine, (void *) "!");
    printf("Created threads\n");

    while (1) {
        uthread_schedule();
        printf("main is still active!\n");
    }
}
