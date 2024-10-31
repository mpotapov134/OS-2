#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct {
    int num;
    char *str;
} typedef arg_t;

void *thread1_routine(void *arg);
void *thread2_routine(void *arg);

void *thread1_routine(void *arg) {
    arg_t arg_struct = {.num = 10, .str = "hello"};
    pthread_t thread2;
    pthread_attr_t thread2_attr;

    pthread_attr_init(&thread2_attr);
    pthread_attr_setdetachstate(&thread2_attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&thread2, &thread2_attr, thread2_routine, (void *) &arg_struct) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Thread 1 created thread 2\n");

    return NULL;
}

void *thread2_routine(void *arg) {
    printf("Thread 2 started\n");
    sleep(5);
    arg_t *arg_struct = (arg_t *) arg;
    printf("Struct print: %i %s\n", arg_struct->num, arg_struct->str);
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t thread1;

    if (pthread_create(&thread1, NULL, thread1_routine, NULL) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }

    // pthread_exit(NULL);

    sleep(10);
}
