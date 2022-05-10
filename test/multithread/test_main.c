#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "./include/range.h"
#include "../../include/ddmin.h"

#define TRHEAD_N 8

int
main (int argc, char * argv[]) {

	char * target_path = argv[1];
	char * input_path = argv[2];

	if (access(target_path, F_OK)) {
                fprintf(stderr, "no program path: %s\n", target_path);
                exit(1);
        }

        if (access(input_path, F_OK)) {
                fprintf(stderr, "no input path: %s\n", input_path);
                exit(1);
        }	
	printf("target: %s, input: %s \n", target_path, input_path);

	long f_size = byte_count_file(input_path);
	int thread_n = TRHEAD_N;
	int rs = 300;
	int max_range_n = (int)ceilf(((f_size - rs + 1) / (float)thread_n));

	int *** parts = malloc(sizeof(int**)*thread_n);
	for (int i = 0; i < thread_n; i++) {
		parts[i] = malloc(sizeof(int*)*max_range_n);
		for (int j = 0; j < max_range_n; j++) {
			parts[i][j] = malloc(sizeof(int)*2);
			parts[i][j][0] = 0;
			parts[i][j][1] = 0;
		}
	}


	ranges(parts, f_size, thread_n, rs);
	//test_ranges(parts, thread_n, max_range_n);

	
	/* ## thread start ## */

	/*
	int select = 0;
	
	struct pthread_data * data;
	data = malloc(sizeof(struct pthread_data));
        data->part = parts[select];
        data->max_range_n = max_range_n;
        data->thread_n = select;
	
	pthread_t p_thread;
	int thr_id = pthread_create(&p_thread, NULL, test_range, (void *)data);
    	if (thr_id < 0) {
		perror("thread create error:");
		exit(1);
	}

	int ret;
	pthread_join(p_thread, (void *)&ret);

	*/

	pthread_t p_threads[TRHEAD_N];
	//pthread_t * p_threads = malloc(sizeof(pthread_t) * thread_n);
	for (int i = 0; i < thread_n; i++) {
		struct pthread_data * data;
		data = malloc(sizeof(struct pthread_data));
		data->part = parts[i];
		data->max_range_n = max_range_n;
		data->thread_n = i;
		int rc = pthread_create(&p_threads[i], NULL, test_range, (void*)data);
		if (rc) {
			perror("ERROR: pthread create");
			exit(1);
		}
	}

	int ret;
	for (int i = 0; i < thread_n; i++) {
		int rc = pthread_join(p_threads[i], (void*)&ret);
		if (rc == -1) {
			fprintf(stderr,"\nERROR: # Thread join %d \n", i);
			exit(1);
		}
	}


	//thread end
	for (int i = 0; i < thread_n; i++) {
		for (int j = 0; j < max_range_n; j++) {
			free(parts[i][j]);
		}
		free(parts[i]);
	}
	free(parts);

	return 0;
}
// data: (int ** parts, int max_range_n)
void * test_range (void *data) {
	struct pthread_data * p_data = (struct pthread_data *)data;

	int ** part = p_data->part;
	int max_range_n = p_data->max_range_n;
	int thread_n = p_data->thread_n;

	for (int j = 0; j < max_range_n; j++) {
		if (part[j][0] == 0 && part[j][1] == 0) {
			continue;
		}
		int start = part[j][0];
		int end = part[j][1];

		fprintf(stderr,"(%d, %d) ", start, end);
	}

	int * ret = malloc(sizeof(int));
	ret[0] = 1;
	return (void*)(ret);
}

void test_ranges (int *** parts, int thread_n, int max_range_n) {

	for (int i = 0; i < thread_n; i++) {
		printf("# THREAD NUMBER: %d \n", i);
		for (int j = 0; j < max_range_n; j++) {
			if (parts[i][j][0] == 0 && parts[i][j][1] == 0) {
				continue;
			}
			int start = parts[i][j][0];
			int end = parts[i][j][1];

			printf("(%d, %d) ", start, end);
		}
		printf("\n");
	}
	
}
