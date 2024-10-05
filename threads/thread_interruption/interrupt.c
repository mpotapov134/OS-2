#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *thread_routine(void *arg) {
    int counter = 0;
    while (1) {
        printf("thread_routine: %i\n", counter++);
        sleep(1);
    }
}

int main(int argc, char **argv) {
    pthread_t new_thread;
    if (pthread_create(&new_thread, NULL, thread_routine, NULL) != 0) {
        printf("pthread_create failed\n");
        exit(EXIT_FAILURE);
    }

    sleep(5);
    if (pthread_cancel(new_thread) != 0) {
        printf("pthread_cancel failed\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(new_thread, NULL) != 0) {
        printf("pthread_join failed\n");
        exit(EXIT_FAILURE);
    }
}
