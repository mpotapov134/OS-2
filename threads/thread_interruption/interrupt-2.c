#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void cleanup_routine(void *arg) {
    free(arg);
}

void *thread_routine(void *arg) {
    char *str = malloc(sizeof("hello world"));

    pthread_cleanup_push(cleanup_routine, (void *) str);

    strcpy(str, "hello world");
    while (1) {
        printf("thread_routine: %s\n", str);
        sleep(1);
    }

    pthread_cleanup_pop(1);
}

int main(int argc, char **argv) {
    pthread_t new_thread;
    if (pthread_create(&new_thread, NULL, thread_routine, NULL) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }

    sleep(5);
    if (pthread_cancel(new_thread) != 0) {
        printf("pthread_cancel failed\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(new_thread, NULL) != 0) {
        printf("pthread_join failed\n");
        exit(EXIT_FAILURE);
    }
}
