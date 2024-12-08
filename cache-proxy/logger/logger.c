#include <stdio.h>
#include <stdlib.h>

#include "logger.h"

static void loggerWrite(logger_t *logger, char *lvl, char *msg) {
    fprintf(stderr, "%s: %s\n", lvl, msg);
}

logger_t *loggerCreate(logLevel_t logLevel) {
    logger_t *logger = malloc(sizeof(*logger));
    if (!logger) {
        return NULL;
    }
    logger->logLevel = logLevel;
    return logger;
}

void loggerDestroy(logger_t *logger) {
    free(logger);
}

void loggerError(logger_t *logger, char *msg) {
    if (logger->logLevel >= LOG_ERROR) {
        loggerWrite(logger, "ERROR", msg);
    }
}

void loggerInfo(logger_t *logger, char *msg) {
    if (logger->logLevel >= LOG_INFO) {
        loggerWrite(logger, "INFO", msg);
    }
}
