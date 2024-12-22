#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache/cache-storage.h"
#include "logger/logger.h"
#include "proxy/proxy.h"
#include "threadpool/threadpool.h"

#define NUM_WORKERS             16
#define TASK_QUEUE_CAP          100

int main(int argc, char **argv) {
    loggerInit(LOG_DEBUG);

    cacheStorage_t *cache = cacheStorageCreate();
    if (!cache) {
        loggerCritical("Failed to create cache storage");
        abort();
    }

    threadPool_t *threadpool = threadPoolCreate(NUM_WORKERS, TASK_QUEUE_CAP);
    if (!threadpool) {
        loggerCritical("Failed to create threadpool");
        abort();
    }

    proxy_t *proxy = proxyCreate(cache, threadpool);
    if (!proxy) {
        loggerCritical("Error creating proxy");
        abort();
    }

    int err = proxyStart(proxy);
    if (err) {
        loggerCritical("Error starting proxy");
        abort();
    }

    proxyDestroy(proxy);
    threadPoolStop(threadpool);
    cacheStorageDestroy(cache);

    loggerFinalize();
    return 0;
}
