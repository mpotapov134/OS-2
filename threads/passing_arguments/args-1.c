#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct {
    int num;
    char *str;
} typedef arg_t;

void *thread_routine(void *_arg) {
    arg_t *arg = (arg_t *) _arg;
    printf("thread_routine: %i %s\n", arg->num, arg->str);
    return NULL;
}

int main(int argc, char **argv) {
    arg_t arg = {.num = 10, .str = "hello"};
    pthread_t new_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&new_thread, &attr, thread_routine, (void *) &arg) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }

    pthread_attr_destroy(&attr);

    sleep(3);
}
