#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void *thread_routine(void *arg) {
    printf("thread_routine: %i %i\n", getpid(), gettid());
    sleep(3);
    return NULL;
}

int main(int argc, char **argv) {
    printf("main: %i %i\n", getpid(), gettid());

    pthread_t new_thread;
    if (pthread_create(&new_thread, NULL, thread_routine, NULL) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(new_thread, NULL) != 0) {
        printf("pthread_join failed\n");
        exit(EXIT_FAILURE);
    }
}
