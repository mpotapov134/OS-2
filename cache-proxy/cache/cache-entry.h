#ifndef CACHE_PROXY_CACHE_ENTRY
#define CACHE_PROXY_CACHE_ENTRY

#include <pthread.h>
#include <stdatomic.h>

struct {
    char *buf;
    size_t size;
    size_t capacity;
    atomic_int referenceCount;
} typedef cacheEntry_t;

cacheEntry_t *cacheEntryCreate();
void cacheEntryDestroy(cacheEntry_t *entry);
int cacheEntryAppend(cacheEntry_t *entry, char *newData, size_t size);
void cacheEntryReference(cacheEntry_t *entry);
void cacheEntryDereference(cacheEntry_t *entry);

#endif /* CACHE_PROXY_CACHE_ENTRY */
