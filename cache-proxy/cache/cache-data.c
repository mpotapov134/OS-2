#include <stdlib.h>
#include <string.h>

#include "cache-data.h"

#define CHUNK_SIZE          1024

cacheData_t cacheDataCreate() {
    cacheData_t data = malloc(sizeof(*data));
    if (data == NULL) {
        return NULL;
    }

    data->buf = NULL;
    data->size = 0;
    data->capacity = 0;
    return data;
}

void cacheDataDestroy(cacheData_t data) {
    if (!data) {
        return;
    }
    free(data->buf);
    data->buf = NULL;
    free(data);
}

int cacheDataAppend(cacheData_t data, char *newData, size_t size) {
    if (!data || !newData) {
        return -1;
    }

    size_t newSize = data->size + size;
    if (data->capacity < newSize) {
        size_t newCapacity = newSize + CHUNK_SIZE;
        char *newBuf = realloc(data->buf, newCapacity);
        if (newBuf == NULL) {
            return -1;
        }
        data->buf = newBuf;
        data->capacity = newCapacity;
    }

    memcpy(data->buf + data->size, newData, size);
    data->size += size;
    return 0;
}
