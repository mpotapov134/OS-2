#ifndef CACHE_PROXY_THREAD_POOL
#define CACHE_PROXY_THREAD_POOL

#include <stdatomic.h>
#include <pthread.h>

typedef struct {
    long id;
    void (*run) (void *);
    void *arg;
} task_t;

typedef struct {
    task_t *tasks;
    size_t capacity;
    size_t size;
    int first;
    int last;
    pthread_mutex_t mutex;
    pthread_cond_t gotTasks;
    pthread_cond_t gotSlots;

    pthread_t *threads;
    int numThreads;

    atomic_int stopped;
} threadPool_t;

threadPool_t *threadPoolCreate(int numThreads, size_t queueCapacity);
void threadPoolSubmit(threadPool_t *pool, void (*run) (void *), void *arg);
void threadPoolStop(threadPool_t *pool);

#endif // CACHE_PROXY_THREAD_POOL
