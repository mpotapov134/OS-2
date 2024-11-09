#include <stdlib.h>
#include <unistd.h>

#include "queue.h"


queue_t *queue_create() {
    int err;
    queue_t *queue = malloc(sizeof(queue_t));
    if (!queue) {
        return NULL;
    }

    queue->first = queue->last = NULL;
    queue->size = 0;

    err = pthread_mutex_init(&queue->mutex, NULL);
    if (err) {
        free(queue);
        return NULL;
    }

    return queue;
}


void queue_destroy(queue_t *queue) {
    if (!queue) {
        return;
    }

    while (!queue_is_empty(queue)) {
        queue_get(queue, NULL);
    }
    free(queue);
    pthread_mutex_destroy(&queue->mutex);
}


int queue_get(queue_t *queue, mythread_t *retval) {
    pthread_mutex_lock(&queue->mutex);

    if (queue_is_empty(queue)) {
        pthread_mutex_unlock(&queue->mutex);
        return 1;
    }

    qnode_t *node = queue->first;
    *retval = node->val;
	queue->first = queue->first->next;
    free(node);
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}


int queue_put(queue_t *queue, mythread_t val) {
    pthread_mutex_lock(&queue->mutex);

    qnode_t *new = malloc(sizeof(qnode_t));
	if (!new) {
        pthread_mutex_unlock(&queue->mutex);
		return 1;
	}

    new->val = val;
	new->next = NULL;

    if (!queue->first) {
		queue->first = queue->last = new;
	} else {
		queue->last->next = new;
		queue->last = queue->last->next;
	}
    queue->size++;

    pthread_mutex_unlock(&queue->mutex);
    // sleep(5);
    return 0;
}


int queue_is_empty(queue_t *queue) {
    return queue->size == 0;
}
