TARGET_1 = cache-proxy
SRCS_1 = main.c logger/logger.c cache/cache-storage.c cache/cache-entry.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: logger/logger.h cache/cache-storage.h cache/cache-entry.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}
