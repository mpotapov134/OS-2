#include <stdlib.h>
#include <string.h>

#include "cache-storage.h"
#include "logger/logger.h"

static unsigned int hash(const char *str, size_t len) {
    unsigned int hash = 0;
    for (size_t i = 0; i < len; i++) {
        hash = hash * 31 + *str++;
    }
    return hash % MAP_SIZE;
}

static int strEq(char *s1, char *s2, size_t len) {
    return strncmp(s1, s2, len) == 0;
}

cacheStorage_t *cacheStorageCreate() {
    cacheStorage_t *storage = malloc(sizeof(*storage));
    if (!storage) return NULL;

    storage->map = calloc(MAP_SIZE, sizeof(node_t*));
    if (!storage->map) {
        free(storage);
        return NULL;
    }

    pthread_mutex_init(&storage->mutex, NULL);

    loggerDebug("cacheStorageCreate: created %p", storage);

    return storage;
}

void cacheStorageDestroy(cacheStorage_t *storage) {
    if (!storage) return;

    pthread_mutex_lock(&storage->mutex);

    for (int i = 0; i < MAP_SIZE; i++) {
        node_t* current = storage->map[i];
        while (current) {
            node_t* temp = current;
            current = current->next;

            cacheEntryDereference(temp->response);  // очищаем ресурсы ноды и удаляем ссылку
            free(temp->request);
            free(temp);
        }
    }

    free(storage->map);
    pthread_mutex_unlock(&storage->mutex);
    pthread_mutex_destroy(&storage->mutex);
    free(storage);

    loggerDebug("cacheStorageDestroy: destroyed %p", storage);
}

int cacheStoragePut(cacheStorage_t *storage, char *req, size_t reqLen, cacheEntry_t *resp) {
    if (!storage || !req || !resp) return -1;

    pthread_mutex_lock(&storage->mutex);

    unsigned long index = hash(req, reqLen);
    node_t *current = storage->map[index];

    /* Проверяем на наличие записи по данному запросу */
    while (current) {
        if (strEq(current->request, req, reqLen)) {         // запись найдена
            if (current->response != resp) {
                cacheEntryDereference(current->response);   // удаляем ссылку на старый ответ
                current->response = resp;                   // заменяем на новый ответ
                cacheEntryReference(resp);                  // добавлена ссылка на новый ответ
            }

            loggerDebug("cacheStoragePut: modified node, key '%s'", req);
            pthread_mutex_unlock(&storage->mutex);
            return 0;
        }
        current = current->next;
    }

    /* Запись не найдена, создаем новую */
    loggerDebug("cacheStoragePut: created new node, key '%s'", req);
    node_t *newNode = malloc(sizeof(*newNode));
    char *request = malloc(reqLen);
    if (!newNode || !request) {
        free(newNode);
        free(request);
        pthread_mutex_unlock(&storage->mutex);
        return -1;
    }

    memcpy(request, req, reqLen);           // сохраняем содержимое запроса
    newNode->request = request;
    newNode->requestLen = reqLen;

    newNode->response = resp;
    cacheEntryReference(resp);              // добавлена ссылка на новый ответ

    newNode->next = storage->map[index];    // добавлем в начало списка
    storage->map[index] = newNode;

    pthread_mutex_unlock(&storage->mutex);
    return 0;
}

cacheEntry_t *cacheStorageGet(cacheStorage_t *storage, char *req, size_t reqLen) {
    if (!storage || !req) return NULL;

    pthread_mutex_lock(&storage->mutex);

    unsigned long index = hash(req, reqLen);
    node_t *current = storage->map[index];

    while (current) {
        if (strEq(current->request, req, reqLen)) {     // запись найдена
            cacheEntryReference(current->response);     // добавили ссылку на эту запись
            loggerDebug("cacheStorageGet: found node, key '%s'", current->request);
            pthread_mutex_unlock(&storage->mutex);
            return current->response;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&storage->mutex);
    return NULL;
}

int cacheStorageRemove(cacheStorage_t *storage, char *req, size_t reqLen) {
    if (!storage || !req) return -1;

    pthread_mutex_lock(&storage->mutex);

    unsigned long index = hash(req, reqLen);
    node_t* current = storage->map[index];
    node_t* previous = NULL;

    while (current) {
        if (strEq(current->request, req, reqLen)) {         // запись найдена
            if (previous) {
                previous->next = current->next;
            } else {
                storage->map[index] = current->next;
            }

            loggerDebug("cacheStorageRemove: found node, key '%s'", current->request);

            cacheEntryDereference(current->response);       // ссылки из мапы больше нет, удаляем
            free(current->request);                         // очищаем память, занятую нодой, но не ответом
            free(current);
            pthread_mutex_unlock(&storage->mutex);
            return 0;
        }
        previous = current;
        current = current->next;
    }

    pthread_mutex_unlock(&storage->mutex);
    return -1;
}