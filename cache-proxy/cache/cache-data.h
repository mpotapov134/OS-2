#ifndef CACHE_PROXY_CACHE_DATA
#define CACHE_PROXY_CACHE_DATA

#include <stdlib.h>

struct _cacheData {
    char *buf;
    size_t size;
    size_t capacity;
};

typedef struct _cacheData *cacheData_t;

cacheData_t cacheDataCreate();
void cacheDataDestroy(cacheData_t data);
int cacheDataAppend(cacheData_t data, char *newData, size_t size);

#endif /* CACHE_PROXY_CACHE_DATA */
