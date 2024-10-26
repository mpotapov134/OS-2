#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>

void handle_error(int err, char *msg) {
    printf("%s: %s\n", msg, strerror(err));
    exit(EXIT_FAILURE);
}

void *thread_routine(void *arg) {
    // printf("thread_routine: %li\n", pthread_self());
    sleep(60);
    return NULL;
}

int main(int argc, char **argv) {
    struct rlimit rlim;
    getrlimit(RLIMIT_NPROC, &rlim);
    printf("soft: %li; hard: %li\n", rlim.rlim_cur, rlim.rlim_max);
    sleep(3);

    int err = 0;
    int thread_counter = 0;
    pthread_t new_thread;
    pthread_attr_t attr;

    /* id может быть переиспользован, если joinable поток завершился и был присоединен
     * при помощи pthread_join или если detached поток завершился */
    while (1) {
        // err = pthread_create(&new_thread, NULL, thread_routine, NULL);
        // if (err) {
        //     handle_error(err, "pthread_create");
        // }

        // pthread_detach(new_thread);

        // err = pthread_join(new_thread, NULL);
        // if (err) {
        //     handle_error(err, "pthread_create");
        // }

        thread_counter++;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        err = pthread_create(&new_thread, &attr, thread_routine, NULL);
        if (err) {
            printf("%i\n", thread_counter);
            handle_error(err, "pthread_create");
        }

        // sleep(1);
    }

    pthread_attr_destroy(&attr);
}
