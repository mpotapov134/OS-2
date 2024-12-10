#include <stdlib.h>
#include <string.h>

#include "cache-entry.h"
#include "logger/logger.h"

#define CHUNK_SIZE          1024

cacheEntry_t *cacheEntryCreate() {
    cacheEntry_t *entry = malloc(sizeof(*entry));
    if (!entry) return NULL;

    entry->buf = NULL;
    entry->size = 0;
    entry->capacity = 0;
    entry->referenceCount = 0;

    loggerDebug("cacheEntryCreate: created %p", entry);

    return entry;
}

void cacheEntryDestroy(cacheEntry_t *entry) {
    if (!entry) return;
    free(entry->buf);
    entry->buf = NULL;
    free(entry);

    loggerDebug("cacheEntryDestroy: destroyed %p", entry);
}

int cacheEntryAppend(cacheEntry_t *entry, char *newData, size_t size) {
    if (!entry || !newData) {
        return -1;
    }

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
    atomic_fetch_add(&entry->referenceCount, 1);
    loggerDebug("cacheEntryReference: %p", entry);
}

void cacheEntryDereference(cacheEntry_t *entry) {
    atomic_fetch_sub(&entry->referenceCount, 1);
    loggerDebug("cacheEntryDereference: %p", entry);
    if (entry->referenceCount == 0) {
        cacheEntryDestroy(entry);
    }
}
