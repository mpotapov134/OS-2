#define _GNU_SOURCE
#include "mythread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sched.h>

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
    mythread_t new_thread;
    pid_t child_pid;
    void *thread_stack;

    thread_stack = create_stack();
    if (thread_stack == NULL) return MYTHREAD_ERROR;

    new_thread = (mythread_t) thread_stack + STACK_SIZE - PAGE_SIZE;
    new_thread->id = (size_t) new_thread;
    new_thread->start_routine = start_routine;
    new_thread->arg = arg;
    new_thread->retval = NULL;
    new_thread->finished = 0;
    new_thread->canceled = 0;
    new_thread->joined = 0;

    thread_stack = (void *) new_thread;

    child_pid = clone(mythread_startup, thread_stack, CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|
        CLONE_THREAD|CLONE_SYSVSEM, (void *) new_thread);

    *thread = new_thread;
    return 0;
}

int mythread_startup(void *arg) {
    mythread_t thread = (mythread_t) arg;
    getcontext(&thread->context_before_start);

    thread->start_routine(thread->arg);

}
