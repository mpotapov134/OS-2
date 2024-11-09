#define _GNU_SOURCE

#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "mythread.h"
#include "queue.h"

#define YELLOW              "\e[0;33m"
#define NO_COLOR            "\e[0m"

static queue_t *finished_queue;

void handle_error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/* futex syscall wrapper */
static int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

static void fwait(int *futexp) {
    int s;
    s = futex(futexp, FUTEX_WAIT, FUTEX_INIT_VAL, NULL, NULL, 0);
    if (s == -1 && errno != EAGAIN) {
        handle_error("futex-FUTEX_WAIT");
    }
}

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

void mythread_init() {
    finished_queue = queue_create();
    if (!finished_queue) {
        printf("mythread_init failed\n");
        abort();
    }
}

int mythread_create(mythread_t *thread, int is_detached, start_routine_t start_routine, void *arg) {
    static unsigned int thread_id = 0;
    mythread_t finished_thread, new_thread;
    pid_t child_pid;
    void *new_reg, *thread_stack;
    int err;

    err = queue_get(finished_queue, &finished_thread);
    if (err) {
        /* no stacks available, allocate a new one */
        new_reg = create_stack();
        if (new_reg == NULL) return -1;
    } else {
        /* wait for the thread to finish and reuse its stack */
        fwait(&finished_thread->futex_word);
        new_reg = finished_thread->thread_mem_reg;
    }

    new_thread = (mythread_t) (new_reg + STACK_SIZE - PAGE_SIZE);
    new_thread->id = thread_id++;
    new_thread->start_routine = start_routine;
    new_thread->arg = arg;
    new_thread->retval = NULL;

    new_thread->thread_mem_reg = new_reg;
    new_thread->futex_word = FUTEX_INIT_VAL;

    new_thread->detached = is_detached;
    new_thread->finished = 0;
    new_thread->canceled = 0;
    new_thread->joined = 0;

    thread_stack = (void *) new_thread;

    child_pid = clone(mythread_startup, thread_stack, CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|
        CLONE_THREAD|CLONE_SYSVSEM|CLONE_CHILD_CLEARTID,
        (void *) new_thread, NULL, NULL, &new_thread->futex_word);
    if (child_pid == -1) {
        munmap(new_reg, STACK_SIZE);
        return -1;
    }

    *thread = new_thread;
    return 0;
}

int mythread_startup(void *arg) {
    mythread_t thread = (mythread_t) arg;
    getcontext(&thread->context_before_start);

    if (!thread->canceled) {
        printf(YELLOW "Thread %i started\n" NO_COLOR, thread->id);
        thread->retval = thread->start_routine(thread->arg);
    }

    thread->finished = 1;

    while (!thread->joined && !thread->detached) {
        sleep(1);
    }

    printf(YELLOW "Thread %i finished\n" NO_COLOR, thread->id);

    queue_put(finished_queue, thread);

    return 0;
}

void mythread_detach(mythread_t thread) {
    thread->detached = 1;
}

void mythread_join(mythread_t thread, void **retval) {
    if (thread->detached) {
        if (retval != NULL) {
            *retval = thread->retval;
        }
        return;
    }

    while (!thread->finished) {
        sleep(1);
    }

    printf(YELLOW "Thread %i joined\n" NO_COLOR, thread->id);

    if (retval != NULL) {
        *retval = thread->retval;
    }

    thread->joined = 1;

    // fwait(&thread->futex_word);
    // munmap(thread->thread_mem_reg, STACK_SIZE);
}

void mythread_cancel(mythread_t thread) {
    thread->retval = NULL;
    thread->canceled = 1;
    printf(YELLOW "Thread %i canceled\n" NO_COLOR, thread->id);
}

void mythread_testcancel(mythread_t thread) {
    if (thread->canceled) {
        setcontext(&thread->context_before_start);
    }
}
