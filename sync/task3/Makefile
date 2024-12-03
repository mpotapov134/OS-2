TARGET_1 = mutex.out
SRCS_1 = main-mutex.c list-mutex.c

TARGET_2 = spin.out
SRCS_2 = main-spin.c list-spin.c

# TARGET_3 = queue-threads-1
# SRCS_3 = queue.c queue-threads-1.c

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1} ${TARGET_2}

${TARGET_1}: list-mutex.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

${TARGET_2}: list-spin.h ${SRCS_2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_2} ${LIBS} -o ${TARGET_2}

# ${TARGET_3}: queue.h ${SRCS_3}
# 	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_3} ${LIBS} -o ${TARGET_3}

clean:
	${RM} -f *.o ${TARGET_1} ${TARGET_2}
