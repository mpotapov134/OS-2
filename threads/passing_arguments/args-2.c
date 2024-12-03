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
    arg_t *arg_struct_p = (arg_t*) arg;

    pid_t pid = getpid();
    printf("%i\n", pid);

    printf("Struct print: %i %s\n", arg_struct_p->num, arg_struct_p->str);
    free(arg_struct_p);
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t thread1;
    pthread_attr_t thread1_attr;
    pid_t pid;

    arg_t *arg_struct_p = malloc(sizeof(*arg_struct_p));
    if (!arg_struct_p) {
        printf("Insufficient memory\n");
        abort();
    }

    arg_struct_p->num = 10;
    arg_struct_p->str = "hello";

    pid = getpid();
    printf("%i\n", pid);

    sleep(10);

    pthread_attr_init(&thread1_attr);
    pthread_attr_setdetachstate(&thread1_attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&thread1, &thread1_attr, thread1_routine, (void*) arg_struct_p) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }

    sleep(3);
    pthread_exit(NULL);
}
