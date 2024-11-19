#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "uthread.h"


void thread_routine(void *arg) {
    for (int i = 0; i < 10; i++) {
        usleep(1000 * 100); // sleep 100 ms
        uthread_schedule();
    }
}


int main(int argc, char **argv) {
    uthread_t thread;
    char thread_name[THREAD_NAME_LEN];
    int thread_counter = 0;
    int err;

    struct uthread main_thread = {.name = "main thread", .finished = 0};
    uthreads[0] = &main_thread;

    while (1) {
        snprintf(thread_name, sizeof(thread_name), "uthread-%i", thread_counter);
        err = uthread_create(&thread, thread_name, thread_routine, NULL);
        if (!err) {
            thread_counter++;
            printf("\e[32m" "Created new thread with name %s\n" "\e[0m", thread_name);
        }

        uthread_schedule();
    }
}
