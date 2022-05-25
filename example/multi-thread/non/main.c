#define THREAD_N 16

#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../../include/ddmin.h"

pthread_mutex_t begin_mt;

char * program_path;
char * input_path;
char * err_msg;

int begin;
int cur_rs;
int ip_size;

void no_thread (int input_size, int rs) {

	while (rs > 0) {
		int i = 0;

		for (int begin = 0; begin <= input_size - rs; begin++) {

			i = i + 1;
						
		}
		rs--;
	}
	
}

void * thread_func (void * data) {

	int start;
	int th_idx = (int) data;

	do {

		pthread_mutex_lock(&begin_mt);
		start = begin;
		begin++;
		pthread_mutex_unlock(&begin_mt);
		if (start > ip_size - cur_rs) {
			break;
		}

		
	} while (start <= ip_size-cur_rs);


	return (void*) (th_idx+1);
}

void thread (int input_size, int rs) {

	pthread_t p_threads[THREAD_N];

	while (rs > 0) {

		begin = 0;
		cur_rs = rs;
		ip_size = input_size;

		for (int i = 0; i < THREAD_N; i++) {
			pthread_create(&p_threads[i], NULL, thread_func, (void*) (i+1));
		}

		for (int i = 0; i < THREAD_N; i++) {
			pthread_join(p_threads[i], NULL);
		}

		rs--;
	}
}

int main (int argc, char * argv[]) {

	fprintf(stderr, "there is no thread: %s, input: %s, err: %s \n", argv[1], argv[2], argv[3]);
	pthread_mutex_init(&begin_mt, NULL);

	program_path = argv[1];
	input_path = argv[2];
	err_msg = argv[3];

	int start_input_size = 378;
	int start_rs = start_input_size - 1;

	struct timeval  tv;
	double start, end;
	double res;

	gettimeofday(&tv, NULL);
	start = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	no_thread(start_input_size, start_rs);
	gettimeofday(&tv, NULL);
	end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	res = (end - start)/1000;
	fprintf(stderr, "no_thread(): %f \n", res);

	gettimeofday(&tv, NULL);
	start = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	thread(start_input_size, start_rs);
	gettimeofday(&tv, NULL);
	end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	res = (end - start)/1000;
	fprintf(stderr, "thread(): %f \n", res);

	pthread_mutex_destroy(&begin_mt);

	return 0;
}
