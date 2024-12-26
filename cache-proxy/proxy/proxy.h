#ifndef CACHE_PROXY_PROXY
#define CACHE_PROXY_PROXY

#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>

#include "cache/cache-storage.h"
#include "threadpool/threadpool.h"

typedef struct {
    int servSocket;
    int running;
    cacheStorage_t *cache;
    threadPool_t *threadpool;

    timer_t gcTimer;
    int gcFinished;
    pthread_cond_t gcFinishedCond;
    pthread_mutex_t mutex;
} proxy_t;

proxy_t *proxyCreate(cacheStorage_t *cache, threadPool_t *threadpool);
void proxyDestroy(proxy_t *proxy);

int proxyStart(proxy_t *proxy);

void garbageCollectorRoutine(union sigval arg);

#endif /* CACHE_PROXY_PROXY */
