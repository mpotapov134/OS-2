#define _GNU_SOURCE
#include "mythread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sched.h>
#include <unistd.h>

enum {
    MYTHREAD_ERROR = -1
} typedef errorCode_t;

static void *create_stack() {
    void *stack;
    int err;

    stack = mmap(NULL, STACK_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) return NULL;

    /* last page is used to generate exceptions when the stack is exhausted */
    err = mprotect(stack + PAGE_SIZE, STACK_SIZE - PAGE_SIZE, PROT_READ | PROT_WRITE);
    if (err) {
        munmap(stack, STACK_SIZE);
        return NULL;
    }

    return stack;
}

int mythread_create(mythread_t *thread, start_routine_t start_routine, void *arg) {
    static unsigned int thread_id = 0;
    mythread_t new_thread;
    pid_t child_pid;
    void *new_stack, *thread_stack;

    new_stack = create_stack();
    if (new_stack == NULL) return MYTHREAD_ERROR;

    new_thread = (mythread_t) (new_stack + STACK_SIZE - PAGE_SIZE);
    new_thread->id = thread_id++;
    new_thread->start_routine = start_routine;
    new_thread->arg = arg;
    new_thread->retval = NULL;
    new_thread->finished = 0;
    new_thread->canceled = 0;
    new_thread->joined = 0;

    thread_stack = (void *) new_thread;

    child_pid = clone(mythread_startup, thread_stack, CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|
        CLONE_THREAD|CLONE_SYSVSEM, (void *) new_thread);
    if (child_pid == -1) {
        munmap(new_stack, STACK_SIZE);
        return MYTHREAD_ERROR;
    }

    *thread = new_thread;
    return 0;
}

int mythread_startup(void *arg) {
    mythread_t thread = (mythread_t) arg;
    getcontext(&thread->context_before_start);

    if (!thread->canceled) {
        printf("\nThread %i started\n\n", thread->id);
        thread->retval = thread->start_routine(thread->arg);
    }

    thread->finished = 1;

    while (!thread->joined) {
        sleep(1);
    }

    printf("\nThread %i finished\n\n", thread->id);

    return 0;
}

void mythread_join(mythread_t thread, void **retval) {
    while (!thread->finished) {
        sleep(1);
    }

    printf("\nThread %i joined\n\n", thread->id);

    if (retval != NULL) {
        *retval = thread->retval;
    }

    thread->joined = 1;
}

void mythread_cancel(mythread_t thread) {
    thread->retval = NULL;
    thread->canceled = 1;
    printf("\nThread %i canceled\n\n", thread->id);
}

void mythread_testcancel(mythread_t thread) {
    if (thread->canceled) {
        setcontext(&thread->context_before_start);
    }
}
