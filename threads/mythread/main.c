#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mythread.h"

void *thread_func(void *arg) {
    mythread_t thread = *((mythread_t *) arg);
    while (1) {
        mythread_testcancel(thread);
        printf("Hello from thread %u\n", thread->id);
        sleep(1);
    }
}

void *thread_func1(void *arg) {
    mythread_t thread = *((mythread_t *) arg);
    for (int i = 0; i < 5; i++) {
        printf("Hello from thread %u\n", thread->id);
        sleep(1);
    }
    return "thread_func1 return";
}

int main(int argc, char **argv) {
    pid_t self_pid;
    mythread_t t1, t2;
    int err;

    self_pid = getpid();
    printf("Process id: %i\n", self_pid);
    sleep(10);

    err = mythread_create(&t1, thread_func, (void*) &t1);
    if (err) {
        printf("mythread_create failed\n");
        exit(EXIT_FAILURE);
    }

    sleep(1);

    err = mythread_create(&t2, thread_func1, (void*) &t2);
    if (err) {
        printf("mythread_create failed\n");
        exit(EXIT_FAILURE);
    }

    void *t2_retval;
    mythread_join(t2, &t2_retval);
    printf("Return value: %s\n", (char*) t2_retval);

    sleep(5);
    mythread_cancel(t1);
    mythread_join(t1, NULL);
    sleep(5);
}
