#ifndef CACHE_PROXY_LOGGER
#define CACHE_PROXY_LOGGER

enum {
    LOG_ERROR   = 1,
    LOG_INFO    = 2
} typedef logLevel_t;

struct {
    logLevel_t logLevel;
} typedef logger_t;

logger_t *loggerCreate(logLevel_t logLevel);
void loggerDestroy(logger_t *logger);
void loggerError(logger_t *logger, char *msg);
void loggerInfo(logger_t *logger, char *msg);

#endif /* CACHE_PROXY_LOGGER */
