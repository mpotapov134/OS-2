#include <stdio.h>
#include <pthread.h>

#include "mutex.h"

#define NUM_THREAD          10
#define NUM_ITER            100000

int counter;
mutex_t mutex;

void *thread_func(void *arg) {
    for (int i = 0; i < NUM_ITER; i++) {
        mutex_lock(&mutex);
        counter++;
        mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t threads[NUM_THREAD];

    mutex_init(&mutex);

    for (int i = 0; i < NUM_THREAD; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }

    for (int i = 0; i < NUM_THREAD; i++) {
        pthread_join(threads[i], NULL);
    }

    printf(
        "Counter value:     %d\n"
        "Expected value:    %d\n"
        "Difference:        %d\n",
        counter, NUM_THREAD * NUM_ITER, NUM_THREAD * NUM_ITER - counter
    );
}
