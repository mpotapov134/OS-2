#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include "mythread.h"

void *thread_func(void *arg) {

    return NULL;
}

int main(int argc, char **argv) {
    pid_t self_pid;
    mythread_t thread;
    int err;

    mythread_init();

    self_pid = getpid();
    printf("Process id: %i\n", self_pid);
    sleep(5);

    for (int i = 0; i < 100000; i++) {
        err = mythread_create(&thread, 1, thread_func, NULL);
        if (err) {
            printf("mythread_create failed\n");
            exit(EXIT_FAILURE);
        }
        printf("%i threads created\n", i + 1);
        usleep(1000);
    }

    sleep(5);
}
