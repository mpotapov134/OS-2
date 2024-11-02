#ifndef UTHREAD_H
#define UTHREAD_H

#include <ucontext.h>

#define PAGE_SIZE                   4096
#define STACK_SIZE                  PAGE_SIZE * 5
#define THREAD_NAME_LEN             30
#define MAX_NUM_THREADS             8

struct uthread {
    char                name[THREAD_NAME_LEN];
    void                (*thread_func)(void *);
    void                *arg;
    ucontext_t          context;
};

typedef struct uthread *uthread_t;

extern uthread_t uthreads[MAX_NUM_THREADS];
extern int thread_count;

int uthread_create(uthread_t *thread, char *name, void (*thread_func)(void *), void *arg);
void uthread_schedule();

#endif
