#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


void sigint_handler(int signum) {
    printf("SIGINT captured in thread %i\n", gettid());
}

void handle_error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void *thread_routine_1(void *arg) {
    /* block all */
    sigset_t sigset;
    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    while (1) {
        printf("thread 1 with tid %i\n", gettid());
        sleep(1);
    }
}

void *thread_routine_2(void *arg) {
    /* block all except for SIGINT */
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    while (1) {
        printf("thread 2 with tid %i\n", gettid());
        sleep(1);
    }
}

void *thread_routine_3(void *arg) {
    /* block all except for SIGQUIT */
    int sig;
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    /* wait for SIGQUIT only */
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGQUIT);

    while (1) {
        printf("thread 3 with tid %i\n", gettid());
        sigwait(&sigset, &sig);
        printf("SIGQUIT captured synchronously in thread 3 with tid %i\n", gettid());
    }
}

int main(int argc, char **argv) {
    pthread_t thread1, thread2, thread3;
    sigset_t sigset;
    struct sigaction action;

    /* block SIGQUIT */
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) != 0) {
        handle_error("sigaction");
    }

    pthread_create(&thread1, NULL, thread_routine_1, NULL);
    pthread_create(&thread2, NULL, thread_routine_2, NULL);
    pthread_create(&thread3, NULL, thread_routine_3, NULL);

    sleep(3);

    pthread_kill(thread1, SIGINT);
    pthread_kill(thread2, SIGINT);
    pthread_kill(thread3, SIGQUIT);

    sleep(3);

    kill(getpid(), SIGINT);
    kill(getpid(), SIGQUIT);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
}
