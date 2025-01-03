#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <stdint.h>
#include <sys/types.h>
#include <ucontext.h>

#define PAGE_SIZE       4096
#define STACK_SIZE      PAGE_SIZE * 10
#define FUTEX_INIT_VAL  1

typedef void *(*start_routine_t)(void*);

enum {
    CANCEL_DEFERRED,
    CANCEL_ASYNC
} typedef cancelType_t;

struct mythread {
    unsigned int        id;
    start_routine_t     start_routine;
    void                *arg;
    void                *retval;
    ucontext_t          context_before_start;
    void                *thread_mem_reg;
    uint32_t            futex_word;
    pid_t               tid;

    cancelType_t        cancel_type;

    int                 detached;
    int                 finished;
    int                 canceled;
    int                 joined;
};

typedef struct mythread *mythread_t;

void mythread_init();
int mythread_create(mythread_t *thread, int is_detached, start_routine_t start_routine, void *arg);
int mythread_startup(void *arg);
void mythread_detach(mythread_t thread);
void mythread_join(mythread_t thread, void **retval);
void mythread_cancel(mythread_t thread);
void mythread_testcancel(mythread_t thread);
void mythread_set_cancel_type(mythread_t thread, cancelType_t new_type);

#endif
