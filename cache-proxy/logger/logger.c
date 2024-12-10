#include <stdio.h>
#include <stdlib.h>

#include "logger.h"

static logger_t *logger;

static void loggerLog(logLevel_t requiredLvl, char *lvlText, char *msg) {
    if (logger->logLevel < requiredLvl) {
        return;
    }
    fprintf(stderr, "%s: %s\n", lvlText, msg);
}

int loggerInit(logLevel_t logLevel) {
    logger = malloc(sizeof(*logger));
    if (!logger) {
        return -1;
    }
    logger->logLevel = logLevel;
    return 0;
}

void loggerFinalize() {
    free(logger);
}

void loggerCritical(char *msg) {
    loggerLog(LOG_CRITICAL, "CRITICAL", msg);
}

void loggerError(char *msg) {
    loggerLog(LOG_ERROR, "ERROR", msg);
}

void loggerInfo(char *msg) {
    loggerLog(LOG_INFO, "INFO", msg);
}

void loggerDebug(char *msg) {
    loggerLog(LOG_DEBUG, "DEBUG", msg);
}
