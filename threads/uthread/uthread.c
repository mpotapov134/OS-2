#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "uthread.h"

uthread_t uthreads[MAX_NUM_THREADS];
int thread_count = 0;
int cur_thread_ind = 0;

static void uthread_startup(int thread_ind);


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


int uthread_create(uthread_t *thread, char *name, void (*thread_func)(void *), void *arg) {
    uthread_t new_thread;
    void *stack;
    int err;

    if (thread_count >= MAX_NUM_THREADS) {
        printf("Reached number of threads limit\n");
        return -1;
    }

    stack = create_stack();
    if (stack == NULL) {
        *thread = NULL;
        return -1;
    }

    new_thread = (uthread_t) (stack + STACK_SIZE - PAGE_SIZE);
    uthreads[thread_count++] = new_thread;

    err = getcontext(&new_thread->context);
    if (err) {
        printf("getcontext failed\n");
        exit(EXIT_FAILURE);
    }

    new_thread->context.uc_link = NULL;
    new_thread->context.uc_stack.ss_sp = stack;
    new_thread->context.uc_stack.ss_flags = 0;
    new_thread->context.uc_stack.ss_size = STACK_SIZE - PAGE_SIZE;
    makecontext(&new_thread->context, (void (*)()) uthread_startup, 1, thread_count - 1);

    new_thread->thread_func = thread_func;
    new_thread->arg = arg;
    strncpy(new_thread->name, name, THREAD_NAME_LEN);

    *thread = new_thread;
    return 0;
}


void uthread_startup(int thread_ind) {
    uthread_t thread = uthreads[thread_ind];
    thread->thread_func(thread->arg);
}


void uthread_schedule() {
    int err;
    ucontext_t *cur_context, *next_context;

    cur_context = &uthreads[cur_thread_ind]->context;
    cur_thread_ind = (cur_thread_ind + 1) % thread_count;
    next_context = &uthreads[cur_thread_ind]->context;

    printf("%sScheduled thread [%i] with name [%s]\n%s",
        "\e[0;33m", cur_thread_ind, uthreads[cur_thread_ind]->name, "\e[0m");

    err = swapcontext(cur_context, next_context);
    if (err) {
        printf("swapcontext failed\n");
        exit(EXIT_FAILURE);
    }
}
