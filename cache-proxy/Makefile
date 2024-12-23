TARGET_1 = cache-proxy.out

SRCS_1 = 	main.c logger/logger.c cache/cache-storage.c								\
			cache/cache-entry.c proxy/proxy.c proxy/client-handler.c					\
			http-parser/picohttpparser.c threadpool/threadpool.c

INCL_1 =	logger/logger.h cache/cache-storage.h cache/cache-entry.h					\
			proxy/proxy.h proxy/client-handler.h http-parser/picohttpparser.h			\
			threadpool/threadpool.h

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: ${INCL_1} ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}
