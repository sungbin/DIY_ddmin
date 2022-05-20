#define _GNU_SOURCE
#define THREAD_N 8

#include <unistd.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int begin;
int cur_rs;
int ip_size;
pthread_mutex_t begin_mt;

void no_thread (int input_size, int rs) {

	while (rs > 0) {

		//fprintf(stderr, "rs=%d \n", rs);

		for (int begin = 0; begin <= input_size - rs; begin++) {

			usleep(1000);
		}
		rs--;
	}
	
}

void * thread_func (void * data) {

	int start;

	do {

		pthread_mutex_lock(&begin_mt);
		start = begin;
		begin++;
		pthread_mutex_unlock(&begin_mt);
		if (start > ip_size - cur_rs) {
			break;
		}
		usleep(1);

	} while (start <= ip_size-cur_rs);


	return NULL;
}

void thread (int input_size, int rs) {

	pthread_t p_threads[THREAD_N];

	while (rs > 0) {

		begin = 0;
		cur_rs = rs;
		ip_size = input_size;

		for (int i = 0; i < THREAD_N; i++) {
			pthread_create(&p_threads[i], NULL, thread_func, NULL);
		}

		for (int i = 0; i < THREAD_N; i++) {
			pthread_join(p_threads[i], NULL);
		}

		rs--;
	}
}

int main () {

	pthread_mutex_init(&begin_mt, NULL);

	int start_input_size = 300;
	int start_rs = 299;

	time_t t = time(NULL);
	clock_t start, end;
	float res;

	start = clock();
	no_thread(start_input_size, start_rs);
	end = clock();
	res = (float)(end - start)/CLOCKS_PER_SEC;
	fprintf(stderr, "no_thread(): %f \n", res);

	start = clock();
	thread(start_input_size, start_rs);
	end = clock();
	res = (float)(end - start)/CLOCKS_PER_SEC;
	fprintf(stderr, "thread(): %f \n", res);

	pthread_mutex_destroy(&begin_mt);
}
