TARGET1 = test-main
SRCS1 = mythread.c queue.c map.c main.c

TARGET2 = test-detached
SRCS2 = mythread.c queue.c map.c test-detached.c

TARGET3 = test-async
SRCS3 = mythread.c queue.c map.c test-async.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-pthread
INCLUDE_DIR="."

all: ${TARGET1} ${TARGET2} ${TARGET3}

${TARGET1}: ${SRCS1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS1} ${LIBS} -o ${TARGET1}

${TARGET2}: ${SRCS2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS2} ${LIBS} -o ${TARGET2}

${TARGET3}: ${SRCS3}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS3} ${LIBS} -o ${TARGET3}

clean:
	${RM} -f *.o ${TARGET1} ${TARGET2} ${TARGET3}
