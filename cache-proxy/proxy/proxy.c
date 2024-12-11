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
#define BACKLOG         10

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

proxy_t *proxyCreate() {
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

    return proxy;
}

void proxyDestroy(proxy_t *proxy) {
    if (!proxy) return;
    close(proxy->servSocket);
    proxy->running = 0;
    free(proxy);
}

int proxyStart(proxy_t *proxy) {
    int err = listen(proxy->servSocket, BACKLOG);
    if (err) return -1;

    loggerInfo("Proxy started");

    proxy->running = 1;
    while (proxy->running) {
        int clientSocket = accept(proxy->servSocket, NULL, NULL);
        if (clientSocket < 0) {
            loggerError("Error accepting connection");
            continue;
        }

        loggerInfo("Accepted new connection");
        handleClient(clientSocket);
    }

    return 0;
}
