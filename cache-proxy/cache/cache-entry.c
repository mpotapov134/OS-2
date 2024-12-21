#include <stdlib.h>
#include <string.h>

#include "cache-entry.h"
#include "logger/logger.h"

#define CHUNK_SIZE          1024

cacheEntry_t *cacheEntryCreate() {
    cacheEntry_t *entry = calloc(1, sizeof(*entry));
    if (!entry) return NULL;
    pthread_spin_init(&entry->lock, 0);

    loggerDebug("cacheEntryCreate: created %p", entry);

    return entry;
}

void cacheEntryDestroy(cacheEntry_t *entry) {
    if (!entry) return;
    free(entry->buf);
    entry->buf = NULL;
    pthread_spin_destroy(&entry->lock);
    free(entry);

    loggerDebug("cacheEntryDestroy: destroyed %p", entry);
}

int cacheEntryAppend(cacheEntry_t *entry, char *newData, size_t size) {
    if (!entry || !newData) return 0;

    size_t newSize = entry->size + size;
    if (entry->capacity < newSize) {
        size_t newCapacity = newSize + CHUNK_SIZE;
        char *newBuf = realloc(entry->buf, newCapacity);
        if (newBuf == NULL) {
            return -1;
        }
        entry->buf = newBuf;
        entry->capacity = newCapacity;
    }

    memcpy(entry->buf + entry->size, newData, size);
    entry->size += size;
    return 0;
}

void cacheEntryReference(cacheEntry_t *entry) {
    if (!entry) return;
    pthread_spin_lock(&entry->lock);
    entry->refCount++;
    pthread_spin_unlock(&entry->lock);
}

void cacheEntryDereference(cacheEntry_t *entry) {
    if (!entry) return;
    pthread_spin_lock(&entry->lock);
    entry->refCount--;
    if (entry->refCount == 0) {
        pthread_spin_unlock(&entry->lock);
        cacheEntryDestroy(entry);
        return;
    }
    pthread_spin_unlock(&entry->lock);
}
