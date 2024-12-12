#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache/cache-storage.h"
#include "logger/logger.h"
#include "proxy/proxy.h"

int main(int argc, char **argv) {
    loggerInit(LOG_DEBUG);

    proxy_t *proxy = proxyCreate();
    if (!proxy) {
        loggerCritical("Error creating proxy");
        abort();
    }

    int err = proxyStart(proxy);
    if (err) {
        loggerCritical("Error starting proxy");
        abort();
    }

    proxyDestroy(proxy);

    loggerFinalize();
    return 0;
}
