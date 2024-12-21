#ifndef CACHE_PROXY_CLIENT_HANDLER
#define CACHE_PROXY_CLIENT_HANDLER

#include "cache/cache-storage.h"

int handleClient(int sockToClient, cacheStorage_t *cache);

#endif /* CACHE_PROXY_CLIENT_HANDLER */
