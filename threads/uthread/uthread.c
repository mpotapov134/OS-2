#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "uthread.h"

uthread_t uthreads[MAX_NUM_THREADS];

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
    int thread_slot = -1;

    /* Search for a viable slot */
    for (int i = 0; i < MAX_NUM_THREADS; i++) {
        if (uthreads[i] == NULL || uthreads[i]->finished) {
            thread_slot = i;
            break;
        }
    }

    if (thread_slot == -1) {
        printf("\e[31m" "Reached number of threads limit\n" "\e[0m");
        return -1;
    }

    stack = create_stack();
    if (stack == NULL) {
        *thread = NULL;
        return -1;
    }

    new_thread = (uthread_t) (stack + STACK_SIZE - PAGE_SIZE);

    err = getcontext(&new_thread->context);
    if (err) {
        printf("getcontext failed\n");
        exit(EXIT_FAILURE);
    }

    new_thread->context.uc_link = NULL;
    new_thread->context.uc_stack.ss_sp = stack;
    new_thread->context.uc_stack.ss_flags = 0;
    new_thread->context.uc_stack.ss_size = STACK_SIZE - PAGE_SIZE;
    makecontext(&new_thread->context, (void (*)()) uthread_startup, 1, thread_slot);

    new_thread->thread_func = thread_func;
    new_thread->arg = arg;
    strncpy(new_thread->name, name, THREAD_NAME_LEN);
    new_thread->finished = 0;

    uthreads[thread_slot] = new_thread;

    *thread = new_thread;
    return 0;
}


void uthread_startup(int thread_slot) {
    uthread_t thread = uthreads[thread_slot];
    thread->thread_func(thread->arg);

    thread->finished = 1;
    printf("\e[0;34m" "Thread [%s] finished\n" "\e[0m", thread->name);
    uthread_schedule();
}


void uthread_schedule() {
    static int cur_thread_ind = 0;

    int err;
    int next_thread_ind;
    ucontext_t *cur_context, *next_context;

    /* Search for the next runnable thread */
    for (int i = 1; i <= MAX_NUM_THREADS; i++) {
        next_thread_ind = (cur_thread_ind + i) % MAX_NUM_THREADS;
        if (uthreads[next_thread_ind] != NULL && !uthreads[next_thread_ind]->finished) {
            break;
        }
    }

    cur_context = &uthreads[cur_thread_ind]->context;
    next_context = &uthreads[next_thread_ind]->context;
    cur_thread_ind = next_thread_ind;

    printf("\e[0;33m" "Scheduled thread [%i] with name [%s]\n" "\e[0m",
        cur_thread_ind, uthreads[cur_thread_ind]->name);

    err = swapcontext(cur_context, next_context);
    if (err) {
        printf("swapcontext failed\n");
        exit(EXIT_FAILURE);
    }
}
