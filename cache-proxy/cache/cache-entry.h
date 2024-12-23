#ifndef CACHE_PROXY_CACHE_ENTRY
#define CACHE_PROXY_CACHE_ENTRY

#define _GNU_SOURCE

#include <pthread.h>
#include <stdatomic.h>

struct {
    char *buf;
    size_t size;
    size_t capacity;

    int completed;
    int canceled;
    pthread_mutex_t mutex;
    pthread_cond_t updated;

    int refCount;
    pthread_spinlock_t refCountLock;
} typedef cacheEntry_t;

cacheEntry_t *cacheEntryCreate();
void cacheEntryDestroy(cacheEntry_t *entry);
int cacheEntryAppend(cacheEntry_t *entry, char *newData, size_t size);

void cacheEntrySetCompleted(cacheEntry_t *entry);
void cacheEntrySetCanceled(cacheEntry_t *entry);

void cacheEntryReference(cacheEntry_t *entry);
void cacheEntryDereference(cacheEntry_t *entry);

#endif /* CACHE_PROXY_CACHE_ENTRY */
