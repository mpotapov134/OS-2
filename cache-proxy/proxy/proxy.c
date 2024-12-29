#define _GNU_SOURCE

#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client-handler.h"
#include "logger/logger.h"
#include "proxy.h"

#define PORT            80
#define BACKLOG         30

static int createServSocket() {
    int servSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (servSocket == -1) {
        loggerError("Failed to create server socket, error: %s", strerror(errno));
        return -1;
    }

    int true = 1;
    setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true));

    struct sockaddr_in sAddr;
    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;
    sAddr.sin_addr.s_addr = INADDR_ANY;
    sAddr.sin_port = htons(PORT);

    int err = bind(servSocket, (struct sockaddr*) &sAddr, sizeof(sAddr));
    if (err) {
        loggerError("Failed to bind server socket, error: %s", strerror(errno));
        close(servSocket);
        return -1;
    }

    return servSocket;
}

static int createGCTimer(proxy_t *proxy) {
    struct sigevent event;
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_signo = 0;
    event.sigev_value.sival_ptr = proxy;
    event.sigev_notify_function = garbageCollectorRoutine;
    event.sigev_notify_attributes = NULL;

    struct itimerspec tmrSpec;
    tmrSpec.it_value.tv_sec = EXPIRY_TIME;
    tmrSpec.it_value.tv_nsec = 0;
    tmrSpec.it_interval.tv_sec = EXPIRY_TIME;
    tmrSpec.it_interval.tv_nsec = 0;

    int err = timer_create(CLOCK_REALTIME, &event, &proxy->gcTimer);
    if (err) {
        loggerError("Failed to create timer for garbage collector");
        return -1;
    }

    err = timer_settime(proxy->gcTimer, 0, &tmrSpec, NULL);
    if (err) {
        loggerError("Failed to arm timer for garbage collector");
        return -1;
    }

    return 0;
}

static void fireGCTimer(proxy_t *proxy) {
    struct itimerspec tmrSpec;
    tmrSpec.it_value.tv_sec = 0;
    tmrSpec.it_value.tv_nsec = 1000;
    tmrSpec.it_interval.tv_sec = 0;
    tmrSpec.it_interval.tv_nsec = 0;

    timer_settime(proxy->gcTimer, 0, &tmrSpec, NULL);
}

proxy_t *proxyCreate(cacheStorage_t *cache, threadPool_t *threadpool) {
    if (!cache) {
        loggerError("proxyCreate: invalid cache");
        return NULL;
    }

    proxy_t *proxy = malloc(sizeof(*proxy));
    if (!proxy) {
        loggerError("proxyCreate: allocation failed");
        return NULL;
    }

    proxy->servSocket = createServSocket();
    if (proxy->servSocket == -1) {
        free(proxy);
        return NULL;
    }

    proxy->running = 1;
    proxy->cache = cache;
    proxy->threadpool = threadpool;

    proxy->gcFinished = 0;
    pthread_cond_init(&proxy->gcFinishedCond, NULL);
    pthread_mutex_init(&proxy->mutex, NULL);

    int err = createGCTimer(proxy);
    if (err) {
        close(proxy->servSocket);
        pthread_cond_destroy(&proxy->gcFinishedCond);
        pthread_mutex_destroy(&proxy->mutex);
        free(proxy);
        return NULL;
    }

    clientHandlerInit();

    return proxy;
}

void proxyDestroy(proxy_t *proxy) {
    if (!proxy) return;

    pthread_mutex_lock(&proxy->mutex);

    proxy->running = 0;
    close(proxy->servSocket);

    fireGCTimer(proxy);
    while (!proxy->gcFinished) {
        pthread_cond_wait(&proxy->gcFinishedCond, &proxy->mutex);
    }

    timer_delete(proxy->gcTimer);

    pthread_mutex_unlock(&proxy->mutex);
    pthread_mutex_destroy(&proxy->mutex);
    pthread_cond_destroy(&proxy->gcFinishedCond);
    free(proxy);

    clientHandlerFinalize();
}

int proxyStart(proxy_t *proxy) {
    int err = listen(proxy->servSocket, BACKLOG);
    if (err) return -1;

    loggerInfo("Proxy started");

    while (proxy->running) {
        int clientSocket = accept(proxy->servSocket, NULL, NULL);
        if (clientSocket < 0) {
            loggerError("Error accepting connection: %s", strerror(errno));
            continue;
        }

        loggerInfo("Accepted new connection");

        clientHandlerArgs_t *args = malloc(sizeof(*args));
        if (!args) {
            loggerError("Failed to allocate memory for client handler arguments");
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
            continue;
        }

        args->sockToClient = clientSocket;
        args->cache = proxy->cache;
        threadPoolSubmit(proxy->threadpool, handleClient, args);
    }

    return 0;
}

void garbageCollectorRoutine(union sigval arg) {
    proxy_t *proxy = (proxy_t*) arg.sival_ptr;

    pthread_mutex_lock(&proxy->mutex);

    if (!proxy->running) {
        proxy->gcFinished = 1;
        pthread_cond_signal(&proxy->gcFinishedCond);
        pthread_mutex_unlock(&proxy->mutex);
        loggerDebug("Garbage collector finished");
        return;
    }

    int removedEntries = cacheStorageClean(proxy->cache);
    pthread_mutex_unlock(&proxy->mutex);

    loggerInfo("Garbage collector: removed %i cache entries", removedEntries);
}
