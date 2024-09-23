#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

char global;

void *mythread(void *arg) {
	char local = 5;
	static char local_static;
	const char local_const;

	pthread_t passed_tid = *(pthread_t *) arg;
	pthread_t real_tid = pthread_self();
	printf("%s\n", pthread_equal(passed_tid, real_tid) ? "match" : "mismatch");

	printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());

	printf("local:\t\t%p\n"
		   "local static:\t%p\n"
		   "local const:\t%p\n"
		   "global:\t\t%p\n", &local, &local_static, &local_const, &global);

	if (pthread_equal(passed_tid, real_tid)) {
		local = 10;
	}

	sleep(60);
	printf("value of local: %i\n", local);
	return NULL;
}

int main() {
	pthread_t tid1, tid2, tid3, tid4, tid5;
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	printf("=========================================================================================\n");
	err = pthread_create(&tid1, NULL, mythread, &tid1);
	printf("=========================================================================================\n");
	err = pthread_create(&tid2, NULL, mythread, &tid1);
	err = pthread_create(&tid3, NULL, mythread, &tid1);
	err = pthread_create(&tid4, NULL, mythread, &tid1);
	err = pthread_create(&tid5, NULL, mythread, &tid1);
	if (err) {
	    printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}

	sleep(65);

	return 0;
}

