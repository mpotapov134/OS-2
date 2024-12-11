#ifndef CACHE_PROXY_PROXY
#define CACHE_PROXY_PROXY

#include <sys/socket.h>

typedef struct {
    int servSocket;
    int running;
} proxy_t;

proxy_t *proxyCreate();
void proxyDestroy(proxy_t *proxy);

int proxyStart(proxy_t *proxy);

#endif /* CACHE_PROXY_PROXY */
