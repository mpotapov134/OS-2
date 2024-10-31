#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct {
    int num;
    char *str;
} typedef arg_t;

void *thread1_routine(void *arg) {
    printf("Thread 1 started\n");
    sleep(20);
    arg_t *arg_struct = (arg_t *) arg;
    printf("Struct print: %i %s\n", arg_struct->num, arg_struct->str);
    return NULL;
}

int main(int argc, char **argv) {
    arg_t arg_struct = {.num = 10, .str = "hello"};
    pthread_t thread1;
    pthread_attr_t thread1_attr;
    pid_t pid;

    pid = getpid();
    printf("%i\n", pid);

    sleep(10);

    pthread_attr_init(&thread1_attr);
    pthread_attr_setdetachstate(&thread1_attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&thread1, &thread1_attr, thread1_routine, NULL) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }

    sleep(3);
    pthread_exit(NULL);
}
