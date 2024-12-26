#ifndef CACHE_PROXY_CACHE_STORAGE
#define CACHE_PROXY_CACHE_STORAGE

#include <pthread.h>
#include <time.h>

#include "cache-entry.h"

#define MAP_SIZE            256
#define EXPIRY_TIME         (60 * 10)

typedef struct _node {
    char *request;
    cacheEntry_t *response;
    struct _node *next;

    time_t putTime;
} node_t;

struct {
    node_t **map;
    pthread_mutex_t mutex;
} typedef cacheStorage_t;

cacheStorage_t *cacheStorageCreate();
void cacheStorageDestroy(cacheStorage_t *storage);
int cacheStoragePut(cacheStorage_t *storage, char *req, cacheEntry_t *resp);
cacheEntry_t *cacheStorageGet(cacheStorage_t *storage, char *req);
int cacheStorageRemove(cacheStorage_t *storage, char *req);
int cacheStorageClean(cacheStorage_t *storage);

#endif /* CACHE_PROXY_CACHE_STORAGE */
