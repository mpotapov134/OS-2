#define _GNU_SOURCE
#include <assert.h>

#include "queue-sem.h"

void *qmonitor(void *arg) {
	queue_t *q = (queue_t *)arg;

	printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

	while (1) {
		queue_print_stats(q);
		sleep(1);
	}

	return NULL;
}

queue_t* queue_init(int max_count) {
	int err;

	queue_t *q = malloc(sizeof(queue_t));
	if (!q) {
		printf("Cannot allocate memory for a queue\n");
		abort();
	}

	q->first = NULL;
	q->last = NULL;
	q->max_count = max_count;
	q->count = 0;

	q->add_attempts = q->get_attempts = 0;
	q->add_count = q->get_count = 0;

	err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
	if (err) {
		printf("queue_init: pthread_create() failed: %s\n", strerror(err));
		abort();
	}

	err = sem_init(&q->mutex, 0, 1);
	if (err) {
		printf("queue_init: sem_init() mutex failed: %s\n", strerror(err));
		abort();
	}

	err = sem_init(&q->item_count, 0, 0);
	if (err) {
		printf("queue_init: sem_init() item count failed: %s\n", strerror(err));
		abort();
	}

	err = sem_init(&q->slot_count, 0, q->max_count);
	if (err) {
		printf("queue_init: sem_init() slot count failed: %s\n", strerror(err));
		abort();
	}

	return q;
}

void queue_destroy(queue_t *q) {
	int tmp;

	if (!q) {
		return;
	}

	pthread_cancel(q->qmonitor_tid);
    pthread_join(q->qmonitor_tid, NULL);
	while (q->count > 0) {
		queue_get(q, &tmp);
	}
	sem_destroy(&q->mutex);
	sem_destroy(&q->item_count);
	sem_destroy(&q->slot_count);
	free(q);
}

int queue_add(queue_t *q, int val) {
	sem_wait(&q->slot_count);
	sem_wait(&q->mutex);

	q->add_attempts++;

	assert(q->count <= q->max_count);

	qnode_t *new = malloc(sizeof(qnode_t));
	if (!new) {
		printf("Cannot allocate memory for new node\n");
		abort();
	}

	new->val = val;
	new->next = NULL;

	if (!q->first) {
		q->first = q->last = new;
	} else {
		q->last->next = new;
		q->last = q->last->next;
	}

	q->count++;
	q->add_count++;

	sem_post(&q->item_count);
	sem_post(&q->mutex);

	return 1;
}

int queue_get(queue_t *q, int *val) {
	sem_wait(&q->item_count);
	sem_wait(&q->mutex);

	q->get_attempts++;

	assert(q->count >= 0);

	qnode_t *tmp = q->first;
	*val = tmp->val;
	q->first = q->first->next;

	free(tmp);

	q->count--;
	q->get_count++;

	sem_post(&q->slot_count);
	sem_post(&q->mutex);

	return 1;
}

void queue_print_stats(queue_t *q) {
	sem_wait(&q->mutex);
	printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
		q->count,
		q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
		q->add_count, q->get_count, q->add_count - q->get_count);
	sem_post(&q->mutex);
}
