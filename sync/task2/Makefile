TARGET_SPIN = spin.out
SRCS_SPIN = queue-spin.c main.c

TARGET_MUTEX = mutex.out
SRCS_MUTEX = queue-mutex.c main.c

TARGET_COND = cond.out
SRCS_COND = queue-cond.c main.c

TARGET_SEM = sem.out
SRCS_SEM = queue-sem.c main.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_SPIN} ${TARGET_MUTEX} ${TARGET_COND} ${TARGET_SEM}

${TARGET_SPIN}: queue-spin.h ${SRCS_SPIN}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_SPIN} ${LIBS} -o ${TARGET_SPIN}

${TARGET_MUTEX}: queue-mutex.h ${SRCS_MUTEX}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_MUTEX} ${LIBS} -o ${TARGET_MUTEX}

${TARGET_COND}: queue-cond.h ${SRCS_COND}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_COND} ${LIBS} -o ${TARGET_COND}

${TARGET_SEM}: queue-sem.h ${SRCS_SEM}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_SEM} ${LIBS} -o ${TARGET_SEM}

clean:
	${RM} -f *.o ${TARGET_SPIN} ${TARGET_MUTEX} ${TARGET_COND} ${TARGET_SEM}
