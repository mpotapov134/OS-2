#define _GNU_SOURCE
#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include "mythread.h"

typedef struct QueueNode {
	mythread_t val;
	struct QueueNode *next;
} qnode_t;

typedef struct Queue {
    int size;
	qnode_t *first;
	qnode_t *last;

	pthread_mutex_t mutex;
} queue_t;

queue_t *queue_create();
void queue_destroy(queue_t *queue);
int queue_get(queue_t *queue, mythread_t *retval);
int queue_put(queue_t *queue, mythread_t val);
int queue_is_empty(queue_t *queue);

#endif /* QUEUE_H */
