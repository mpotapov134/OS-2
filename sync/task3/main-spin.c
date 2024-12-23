#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "list-spin.h"

// #define SLEEP
#define SWAP_PROB_MODIFIER          1000

int iterIncrease, iterDecrease, iterEqual;
int countInc, countDec, countEq;
int swapCount;
pthread_spinlock_t swapCountLock;

void sLock(pthread_spinlock_t *spin) {
    int err = pthread_spin_lock(spin);
    if (err) {
        printf("pthread_spin_lock: %s\n", strerror(err));
        abort();
    }
}

void sUnlock(pthread_spinlock_t *spin) {
    int err = pthread_spin_unlock(spin);
    if (err) {
        printf("pthread_spin_unlock: %s\n", strerror(err));
        abort();
    }
}

void *monitor(void *arg) {
    while (1) {
        printf(
            "Search increase iter: %d\tcount: %d\n"
            "Search decrease iter: %d\tcount: %d\n"
            "Search equal    iter: %d\tcount: %d\n"
            "Swap count: %d\n",
            iterIncrease, countInc, iterDecrease, countDec, iterEqual, countEq, swapCount
        );
        usleep(1000 * 500);
    }
}

void *searchIncreasing(void *arg) {
    Storage *storage = (Storage*) arg;

    while (1) {
        Node *current = storage->first;
        Node *tmp;
        int countIncCur = 0;

        sLock(&current->lock);
        while (1) {
            tmp = current;
            if (current->next == NULL) {
                break;
            }

            sLock(&current->next->lock);

            if (strlen(current->value) < strlen(current->next->value)) {
                countIncCur++;
            }

            current = current->next;
            sUnlock(&tmp->lock);
        }

        sUnlock(&current->lock);
        iterIncrease++;
        countInc = countIncCur;
    }

    return NULL;
}

void *searchDecreasing(void *arg) {
    Storage *storage = (Storage*) arg;

    while (1) {
        Node *current = storage->first;
        Node *tmp;
        int countDecCur = 0;

        sLock(&current->lock);
        while (1) {
            tmp = current;
            if (current->next == NULL) {
                break;
            }

            sLock(&current->next->lock);

            if (strlen(current->value) > strlen(current->next->value)) {
                countDecCur++;
            }

            current = current->next;
            sUnlock(&tmp->lock);
        }

        sUnlock(&current->lock);
        iterDecrease++;
        countDec = countDecCur;
    }

    return NULL;
}

void *searchEqual(void *arg) {
    Storage *storage = (Storage*) arg;

    while (1) {
        Node *current = storage->first;
        Node *tmp;
        int countEqCur = 0;

        sLock(&current->lock);
        while (1) {
            tmp = current;
            if (current->next == NULL) {
                break;
            }

            sLock(&current->next->lock);

            if (strlen(current->value) == strlen(current->next->value)) {
                countEqCur++;
            }

            current = current->next;
            sUnlock(&tmp->lock);
        }

        sUnlock(&current->lock);
        iterEqual++;
        countEq = countEqCur;
    }

    return NULL;
}

void *swap(void *arg) {
    Storage *storage = (Storage*) arg;
    Node *current = storage->first;
    Node *n0, *n1, *n2, *n3;
    int rnd;

    while (1) {
        if (current == NULL) {
            current = storage->first;
        }

        sLock(&current->lock);

        /* reached end of list */
        if (current->next == NULL) {
            sUnlock(&current->lock);

            #ifdef SLEEP
            usleep(500);
            #endif

            current = storage->first;
            continue;
        }

        sLock(&current->next->lock);

        /* reached end of list */
        if (current->next->next == NULL) {
            sUnlock(&current->next->lock);
            sUnlock(&current->lock);

            #ifdef SLEEP
            usleep(500);
            #endif

            current = storage->first;
            continue;
        }

        sLock(&current->next->next->lock);

        n0 = current;
        n1 = n0->next;
        n2 = n1->next;
        n3 = n2->next;

        rnd = getRandomNumber(1, SWAP_PROB_MODIFIER);

        /* swap */
        if (rnd == 1) {
            n0->next = n2;
            n2->next = n1;
            n1->next = n3;

            sLock(&swapCountLock);
            swapCount++;
            sUnlock(&swapCountLock);
        }

        current = current->next;
        sUnlock(&n2->lock);
        sUnlock(&n1->lock);
        sUnlock(&n0->lock);
    }

    return NULL;
}

int main(int argc, char **argv) {
    int err;
    Storage *list;
    pthread_t tSearchInc, tSearchDec, tSearchEq;
    pthread_t tSwap1, tSwap2, tSwap3;
    pthread_t tMonitor;

    err = pthread_spin_init(&swapCountLock, PTHREAD_PROCESS_PRIVATE);
    if (err) {
        printf("main: pthread_spin_init() failed\n");
        abort();
    }

    list = listInit();

    pthread_create(&tSearchInc, NULL, searchIncreasing, (void*) list);
    pthread_create(&tSearchDec, NULL, searchDecreasing, (void*) list);
    pthread_create(&tSearchEq, NULL, searchEqual, (void*) list);

    pthread_create(&tSwap1, NULL, swap, (void*) list);
    pthread_create(&tSwap2, NULL, swap, (void*) list);
    pthread_create(&tSwap3, NULL, swap, (void*) list);

    pthread_create(&tMonitor, NULL, monitor, NULL);

    pthread_join(tSearchInc, NULL);
    pthread_join(tSearchDec, NULL);
    pthread_join(tSearchEq, NULL);

    pthread_join(tSwap1, NULL);
    pthread_join(tSwap2, NULL);
    pthread_join(tSwap3, NULL);

    pthread_join(tMonitor, NULL);

    pthread_spin_destroy(&swapCountLock);
    listDestroy(list);
}
