#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "list-rw.h"

// #define SLEEP
#define SWAP_PROB_MODIFIER          1000

int iterIncrease, iterDecrease, iterEqual;
int countInc, countDec, countEq;
int swapCount;
pthread_mutex_t swapCountLock;

void lockWr(pthread_rwlock_t *rwlock) {
    int err = pthread_rwlock_wrlock(rwlock);
    if (err) {
        printf("pthread_rwlock_wrlock: %s\n", strerror(err));
        abort();
    }
}

void lockRd(pthread_rwlock_t *rwlock) {
    int err = pthread_rwlock_rdlock(rwlock);
    if (err) {
        printf("pthread_rwlock_rdlock: %s\n", strerror(err));
        abort();
    }
}

void unlock(pthread_rwlock_t *rwlock) {
    int err = pthread_rwlock_unlock(rwlock);
    if (err) {
        printf("pthread_rwlock_unlock: %s\n", strerror(err));
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
    Node *current = storage->first;
    int countIncCur = 0;

    while (1) {
        if (current == NULL) {
            current = storage->first;
        }

        lockRd(&current->lock);

        /* reached end of list */
        if (current->next == NULL) {
            iterIncrease++;
            countInc = countIncCur;
            countIncCur = 0;
            unlock(&current->lock);

            #ifdef SLEEP
            usleep(500);
            #endif

            current = storage->first;
            continue;
        }

        /* have not reached end of list yet */
        lockRd(&current->next->lock);

        if (strlen(current->value) < strlen(current->next->value)) {
            countIncCur++;
        }

        unlock(&current->next->lock);
        unlock(&current->lock);

        current = current->next;
    }

    return NULL;
}

void *searchDecreasing(void *arg) {
    Storage *storage = (Storage*) arg;
    Node *current = storage->first;
    int countDecCur = 0;

    while (1) {
        if (current == NULL) {
            current = storage->first;
        }

        lockRd(&current->lock);

        /* reached end of list */
        if (current->next == NULL) {
            iterDecrease++;
            countDec = countDecCur;
            countDecCur = 0;
            unlock(&current->lock);

            #ifdef SLEEP
            usleep(500);
            #endif

            current = storage->first;
            continue;
        }

        /* have not reached end of list yet */
        lockRd(&current->next->lock);

        if (strlen(current->value) > strlen(current->next->value)) {
            countDecCur++;
        }

        unlock(&current->next->lock);
        unlock(&current->lock);

        current = current->next;
    }

    return NULL;
}

void *searchEqual(void *arg) {
    Storage *storage = (Storage*) arg;
    Node *current = storage->first;
    int countEqCur = 0;

    while (1) {
        if (current == NULL) {
            current = storage->first;
        }

        lockRd(&current->lock);

        /* reached end of list */
        if (current->next == NULL) {
            iterEqual++;
            countEq = countEqCur;
            countEqCur = 0;
            unlock(&current->lock);

            #ifdef SLEEP
            usleep(500);
            #endif

            current = storage->first;
            continue;
        }

        /* have not reached end of list yet */
        lockRd(&current->next->lock);

        if (strlen(current->value) == strlen(current->next->value)) {
            countEqCur++;
        }

        unlock(&current->next->lock);
        unlock(&current->lock);

        current = current->next;
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

        lockWr(&current->lock);

        /* reached end of list */
        if (current->next == NULL) {
            unlock(&current->lock);

            #ifdef SLEEP
            usleep(500);
            #endif

            current = storage->first;
            continue;
        }

        lockWr(&current->next->lock);

        /* reached end of list */
        if (current->next->next == NULL) {
            unlock(&current->next->lock);
            unlock(&current->lock);

            #ifdef SLEEP
            usleep(500);
            #endif

            current = storage->first;
            continue;
        }

        lockWr(&current->next->next->lock);

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

            pthread_mutex_lock(&swapCountLock);
            swapCount++;
            pthread_mutex_unlock(&swapCountLock);
        }

        unlock(&n2->lock);
        unlock(&n1->lock);
        unlock(&n0->lock);

        current = current->next;
    }

    return NULL;
}

int main(int argc, char **argv) {
    int err;
    Storage *list;
    pthread_t tSearchInc, tSearchDec, tSearchEq;
    pthread_t tSwap1, tSwap2, tSwap3;
    pthread_t tMonitor;

    err = pthread_mutex_init(&swapCountLock, NULL);
    if (err) {
        printf("main: pthread_mutex_init failed\n");
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

    pthread_mutex_destroy(&swapCountLock);
    listDestroy(list);
}
