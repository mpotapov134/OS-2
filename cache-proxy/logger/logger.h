#ifndef CACHE_PROXY_LOGGER
#define CACHE_PROXY_LOGGER

enum {
    LOG_CRITICAL    = 0,
    LOG_ERROR       = 1,
    LOG_INFO        = 2,
    LOG_DEBUG       = 3
} typedef logLevel_t;

struct {
    logLevel_t logLevel;
} typedef logger_t;

int loggerInit(logLevel_t logLevel);
void loggerFinalize();
void loggerCritical(char *msg);
void loggerError(char *msg);
void loggerInfo(char *msg);
void loggerDebug(char *msg);

#endif /* CACHE_PROXY_LOGGER */
