#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <sys/types.h>
#include <ucontext.h>

#define PAGE_SIZE       4096
#define STACK_SIZE      PAGE_SIZE * 3

typedef void *(*start_routine_t)(void*);

struct mythread {
    size_t              id;
    start_routine_t     start_routine;
    void                *arg;
    void                *retval;
    ucontext_t          context_before_start;

    int                 finished;
    int                 canceled;
    int                 joined;
};

typedef struct mythread *mythread_t;

int mythread_create(mythread_t *thread, start_routine_t start_routine, void *arg);
int mythread_startup(void *arg);

#endif
