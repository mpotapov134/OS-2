#include <stdio.h>
#include <pthread.h>

#include "spin.h"

#define NUM_THREAD          10
#define NUM_ITER            100000

int counter;
spinlock_t spin;

void *thread_func(void *arg) {
    for (int i = 0; i < NUM_ITER; i++) {
        spinlock_lock(&spin);
        counter++;
        spinlock_unlock(&spin);
    }
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t threads[NUM_THREAD];

    spinlock_init(&spin);

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
