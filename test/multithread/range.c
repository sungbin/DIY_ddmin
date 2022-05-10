#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "./include/range.h"

void
ranges (int ** parts, long f_size, int rs) {

	int thread_idx = 0;
	int idx = 0;
	int max_idx = (f_size - rs + 1);

	do {
		int range_cnt = ceilf((max_idx / (float)(THREAD_N - thread_idx)));

		for (int i = 0; i < range_cnt; i++) {

			int start = i + idx;
			parts[thread_idx][i] = start;
		}
		
		idx += range_cnt;
		max_idx -= range_cnt;
		thread_idx++;

	} while(thread_idx < THREAD_N);

}

void
run_threads (int ** parts, void *test_range_func, int max_range_n, int rs) {

	pthread_t p_threads[THREAD_N];
	struct pthread_data * data_list[THREAD_N];
        for (int i = 0; i < THREAD_N-1; i++) {
                struct pthread_data * data;
                data = malloc(sizeof(struct pthread_data));
                data->part = parts[i];
                data->max_range_n = max_range_n;
                data->thread_n = i;
                data->rs = rs;
		data_list[i] = data;
                int rc = pthread_create(&p_threads[i], NULL, test_range_func, (void*)data);
                if (rc) {
                        perror("ERROR: pthread create");
                        exit(1);
                }
        }
        struct pthread_data * data;
        data = malloc(sizeof(struct pthread_data));
        data->part = parts[THREAD_N-1];
        data->max_range_n = max_range_n;
        data->thread_n = THREAD_N-1;
        data->rs = rs;
	data_list[THREAD_N-1] = data;

	int (*fp)(void*) = test_range_func;
	fp((void*)data);
	free(data);

        int ret;
        for (int i = 0; i < THREAD_N-1; i++) {
                int rc = pthread_join(p_threads[i], (void*)&ret);
                if (rc == -1) {
                        fprintf(stderr,"\nERROR: # Thread join %d \n", i);
                        exit(1);
                }
		free(data_list[i]);
        }

}
// data: (int ** parts, int max_range_n)
void * 
test_range (void *data) {
	struct pthread_data * p_data = (struct pthread_data *)data;

	int * part = p_data->part;
	int max_range_n = p_data->max_range_n;
	int thread_n = p_data->thread_n;
	int rs = p_data->rs;

	for (int j = 0; j < max_range_n; j++) {
		if (part[j] == -1) {
			continue;
		}
		int start = part[j];
		int end = start + rs;

		fprintf(stderr,"(%d, %d) ", start, end);
	}

	int * ret = malloc(sizeof(int));
	ret[0] = 1;
	return (void*)(ret);
}


