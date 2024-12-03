#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list-spin.h"

int getRandomNumber(int min, int max) {
	return rand() % (max - min + 1) + min;
}

static void generateString(char dest[], int maxSize) {
	/* длина строки без учета 0 */
	int length = getRandomNumber(1, maxSize - 1);

	memset(dest, 'a', length);
	dest[length] = 0;
}

static Node *createNode(const char value[]) {
	int err;

	Node *newNode = malloc(sizeof(*newNode));
	if (!newNode) {
		printf("Cannot allocate memory for a new node\n");
		abort();
	}

	err = pthread_spin_init(&newNode->lock, PTHREAD_PROCESS_PRIVATE);
	if (err) {
		printf("createNode: pthread_spin_init() failed\n");
		abort();
	}

	memcpy(newNode->value, value, STR_SIZE);
	newNode->next = NULL;

	return newNode;
}

Storage* listInit() {
	int err;
	char str[STR_SIZE];

	srand(time(NULL));

	Storage *list = malloc(sizeof(Storage));
	if (!list) {
		printf("Cannot allocate memory for a list\n");
		abort();
	}

	err = pthread_spin_init(&list->lock, PTHREAD_PROCESS_PRIVATE);
	if (err) {
		printf("listInit: pthread_spin_init() failed\n");
		abort();
	}

	list->first = NULL;

	for (int i = 0; i < STORAGE_SIZE; i++) {
		generateString(str, STR_SIZE);
		listAdd(list, str);
	}

	return list;
}

void listDestroy(Storage *list) {
	if (!list) {
		return;
	}

	pthread_spin_lock(&list->lock);

	Node *current = list->first;
	Node *next;

	while (current != NULL) {
		next = current->next;
		pthread_spin_destroy(&current->lock);
		free(current);
		current = next;
	}

	pthread_spin_unlock(&list->lock);
	pthread_spin_destroy(&list->lock);
	free(list);
}

void listAdd(Storage *list, const char value[]) {
	Node *newNode = createNode(value);
	if (!newNode) {
		printf("listAdd: failed to create a new node\n");
		abort();
	}

	pthread_spin_lock(&list->lock);
	pthread_spin_lock(&newNode->lock);

	newNode->next = list->first;
	list->first = newNode;

	pthread_spin_unlock(&list->lock);
	pthread_spin_unlock(&newNode->lock);
}
