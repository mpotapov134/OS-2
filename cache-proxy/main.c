#include "logger/logger.h"

int main(int argc, char **argv) {
    logger_t *logger = loggerCreate(LOG_INFO);
    return 0;
}
