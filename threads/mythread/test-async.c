#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mythread.h"

void *thread_func(void *arg) {
    mythread_t thread = *((mythread_t*) arg);

    while (1) {
        printf("Hello from thread %u\n", thread->id);
        sleep(1);
    }
}

int main(int argc, char **argv) {
    pid_t self_pid;
    mythread_t thread;
    int err;

    mythread_init();

    self_pid = getpid();
    printf("Process id: %i\n", self_pid);

    err = mythread_create(&thread, 0, thread_func, (void*) &thread);
    if (err) {
        printf("mythread_create failed\n");
        exit(EXIT_FAILURE);
    }

    mythread_set_cancel_type(thread, CANCEL_ASYNC);
    printf("Set thread cancel type to ASYNC\n");

    sleep(5);

    printf("Cancelling thread...\n");
    mythread_cancel(thread);

    mythread_join(thread, NULL);
}
