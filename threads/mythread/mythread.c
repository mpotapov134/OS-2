#define _GNU_SOURCE

#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "map.h"
#include "mythread.h"
#include "queue.h"

#define YELLOW              "\e[0;33m"
#define NO_COLOR            "\e[0m"

static Map *thread_map;
static queue_t *finished_queue;

void handle_error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/* futex syscall wrapper */
static int futex(uint32_t *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

static void fwait(uint32_t *futexp) {
    int s;
    s = futex(futexp, FUTEX_WAIT, FUTEX_INIT_VAL, NULL, NULL, 0);
    if (s == -1 && errno != EAGAIN) {
        handle_error("futex-FUTEX_WAIT");
    }
}



void async_cancel_handler(int sig) {
    mythread_t thread;

    printf(YELLOW "Cancelling asynchronously...\n" NO_COLOR);

    thread = (mythread_t) mapGet(thread_map, gettid());
    if (!thread) {
        printf("Async canlel error\n");
        abort();
    }

    mythread_testcancel(thread);
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
    struct sigaction act;

    finished_queue = queue_create();
    if (!finished_queue) {
        printf("mythread_init failed\n");
        abort();
    }

    thread_map = mapCreate();
    if (!thread_map) {
        printf("mythread_init failed\n");
        abort();
    }

    act.sa_handler = async_cancel_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGUSR1, &act, NULL) != 0) {
        handle_error("sigaction");
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

    new_thread->cancel_type = CANCEL_DEFERRED;

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
    new_thread->tid = child_pid;

    mapAdd(thread_map, child_pid, (void*) new_thread);

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
    mapRemove(thread_map, gettid());

    if (thread->detached) {
        printf(YELLOW "Thread %i finished\n" NO_COLOR, thread->id);
        queue_put(finished_queue, thread);
        return 0;
    }

    while (!thread->joined) {
        sleep(1);
    }

    printf(YELLOW "Thread %i finished\n" NO_COLOR, thread->id);

    return 0;
}

void mythread_detach(mythread_t thread) {
    thread->detached = 1;
}

void mythread_join(mythread_t thread, void **retval) {
    if (thread->detached) {
        if (retval != NULL) {
            *retval = NULL;
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

    fwait(&thread->futex_word);
    munmap(thread->thread_mem_reg, STACK_SIZE);
}

void mythread_cancel(mythread_t thread) {
    thread->retval = NULL;
    thread->canceled = 1;
    printf(YELLOW "Thread %i canceled\n" NO_COLOR, thread->id);

    /* Асинхронное прерывание потока при помощи сигналов */
    if (thread->cancel_type == CANCEL_ASYNC) {
        tgkill(getpid(), thread->tid, SIGUSR1);
    }
}

void mythread_testcancel(mythread_t thread) {
    if (thread->canceled) {
        setcontext(&thread->context_before_start);
    }
}

void mythread_set_cancel_type(mythread_t thread, cancelType_t new_type) {
    thread->cancel_type = new_type;
}
