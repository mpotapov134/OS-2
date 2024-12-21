#ifndef CACHE_PROXY_PROXY
#define CACHE_PROXY_PROXY

#include <sys/socket.h>

#include "cache/cache-storage.h"

typedef struct {
    int servSocket;
    int running;
    cacheStorage_t *cache;
} proxy_t;

proxy_t *proxyCreate(cacheStorage_t *cache);
void proxyDestroy(proxy_t *proxy);

int proxyStart(proxy_t *proxy);

#endif /* CACHE_PROXY_PROXY */
