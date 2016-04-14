#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define NUM_THREADS 25

typedef struct _thread_data_t {
	int tid;
	int amount;
} thread_data_t;

// global variables - used by all threads
int waiting_c = 0;
int waiting_h = 0;
sem_t S;
sem_t SC;
sem_t SH;


void *carbon(void *arg) {
	
	thread_data_t *data = (thread_data_t *) arg;
	if (sem_wait(&S) < 0) {
		exit(errno);
	}
	fflush(stdout);
	printf("C enters.\n");
	fflush(stdout);
	if (waiting_h >= 4) {
		for (int i = 0; i < 4; i++) {
			if (sem_post(&SH) < 0) {
				exit(errno);
			}
		}
		waiting_h -= 4;
		fflush(stdout);
		printf("\nC combining to make CH4!\n\n");
		if (sem_post(&S) < 0) {
			exit(errno);
		}
	} else {
		waiting_c++;
		fflush(stdout);
		printf("C waiting. C [%d] H [%d]\n", waiting_c, waiting_h);
		fflush(stdout);
		if (sem_post(&S) < 0) {
			exit(errno);
		}
		if (sem_wait(&SC) < 0) {
			exit(errno);
		}
	}
	
	pthread_exit(NULL);

}

void *hydrogen(void *arg) {

	thread_data_t *data = (thread_data_t *) arg;
	if (sem_wait(&S) < 0) {
		exit(errno);
	}		
	fflush(stdout);
	printf("H enters.\n");
	fflush(stdout);
	if (waiting_h >= 3 && waiting_c >= 1) {
		for (int i = 0; i < 3; i++) {
			if (sem_post(&SH) < 0) {
				exit(errno);
			}
		}
		waiting_h -= 3;
		if (sem_post(&SC) < 0) {
			exit(errno);
		}
		waiting_c -= 1;
		fflush(stdout);
		printf("\nH combining to make CH4!\n\n");
		if (sem_post(&S) < 0) {
			exit(errno);
		}
	} else {
		waiting_h++;
		fflush(stdout);
		printf("H waiting. C [%d] H [%d]\n", waiting_c, waiting_h);
		fflush(stdout);
		if (sem_post(&S) < 0) {
			exit(errno);
		}
		if (sem_wait(&SH) < 0) {
			exit(errno);
		}
	}
	
	pthread_exit(NULL);

}

int main(int argc, char **argv) {

	pthread_t threads[NUM_THREADS];
	thread_data_t thread_data[NUM_THREADS];

	// init semaphores
	if (sem_init(&S, 0, (unsigned int)1) < 0 | 
	sem_init(&SC, 0, (unsigned int)0) < 0 |
	sem_init(&SH, 0, (unsigned int)0) < 0) {
		exit(errno);
	}

	// init thread data
	int i;
	// Carbons
	for (i = 0; i < NUM_THREADS / 5; i++) {
		thread_data[i].tid = i;
		thread_data[i].amount = 25;
		if (pthread_create(&threads[i], 0, carbon, &thread_data[i]) <0) {
			exit(errno);
		}
	} 	
	// Hyrdogens
	for (i = 5; i < NUM_THREADS; i++) {
		thread_data[i].tid = 1;
		thread_data[i].amount = 25;
		if (pthread_create(&threads[i], 0, hydrogen, &thread_data[i]) < 0) {
			exit(errno);
		}
	}
	
	// wait for threads to finish
	for (i = 0; i < NUM_THREADS; i++) {
		if (pthread_join(threads[i], 0) < 0) {
			exit(errno);
		}
	}
	
	// close semaphores
	if (sem_close(&S) < 0 |
	sem_close(&SC) < 0 |
	sem_close(&SH) < 0) {
		exit(errno);
	}

	exit(EXIT_SUCCESS);
}
