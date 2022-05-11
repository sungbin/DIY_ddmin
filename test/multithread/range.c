#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>


#include "./include/range.h"
#include "../../include/ddmin.h"

extern int file_no;
extern int iter_no;

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

	} while (thread_idx < THREAD_N);

}

void
run_threads (int ** parts, void *test_range_func, int max_range_n, int rs, char * program_path, char * mmap_addr, long f_size, char * err_msg) {

	pthread_t p_threads[THREAD_N];
	struct pthread_data * data_list[THREAD_N];
        for (int i = 0; i < THREAD_N-1; i++) {
                struct pthread_data * data;
                data = malloc(sizeof(struct pthread_data));
                data->part = parts[i];
                data->max_range_n = max_range_n;
                data->thread_n = i;
                data->rs = rs;
		data->program_path = program_path;
		data->mmap_addr = mmap_addr;
                data->f_size = f_size;
		data->out_idx = file_no + i;
                data->err_msg = err_msg;
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
        data->f_size = f_size;
	data->program_path = program_path;
	data->mmap_addr = mmap_addr;
	data->err_msg = err_msg;
	data->out_idx = file_no + 7;

	//data_list[THREAD_N-1] = data;

	int (*fp)(void*) = test_range_func;
	fp((void*)data);
	free(data);

        for (int i = 0; i < THREAD_N-1; i++) {
		void * retval;
                int rc = pthread_join(p_threads[i], (void*)&retval);
                if (rc == -1) {
                        fprintf(stderr,"\nERROR: # Thread join %d \n", i);
                        exit(1);
                }

		free(data_list[i]);
		if (retval == PTHREAD_CANCELED) {
			fprintf(stderr, "PTHREAD_CANCELED: THREAD :%d", i);
			exit(1);
		}
		////fprintf(stderr, "THREAD %d: return: %d\n", i, ()retval);
        }
	
	file_no = file_no + THREAD_N;
}

// data: (int ** parts, int max_range_n)
void * 
test_range (void *data) {

	struct pthread_data * p_data = (struct pthread_data *)data;

	int * part = p_data->part;
	int max_range_n = p_data->max_range_n;
	int thread_n = p_data->thread_n;
	int rs = p_data->rs;
	int out_idx = p_data->out_idx;
	long f_size = p_data->f_size;
	char * program_path = p_data->program_path;
	char * mmap_addr = p_data->mmap_addr;
        char * err_msg = p_data->err_msg;

	char * out_path = malloc(sizeof(char) * 256);
	sprintf(out_path, "./%d.part", out_idx);
	FILE * out_fp = fopen(out_path, "wb");
	int out_fd = fileno(out_fp);

	FILE * null_fp = fopen("/dev/null", "wb");

	fprintf(stderr, "THREAD: %d - out_idx - %s \n", thread_n, out_path);

	for (int j = 0; j < max_range_n; j++) {
		if (part[j] == -1) {
			continue;
		}
		int start = part[j];
		//int end = start + rs;

		if (lseek(out_fd, 0, SEEK_SET) == -1) {
                	perror("lseek(out_fd)");
                	exit(1);
        	}
		
		int _start = start;
		while (_start > 0) {
			char buf[2048];
			int buf_size;
		
			if (_start < 2048) {	
				buf_size = fwrite(mmap_addr+(start-_start), 1, _start, out_fp);
			}
			else {
				buf_size = fwrite(mmap_addr+(start-_start), 1, 2048, out_fp);
			}
			_start -= buf_size;
		}

		int _rs = rs;
		while (_rs > 0) {
			char buf[2048];
			int buf_size;
			//char * next_p = mmap_addr+(start+(rs-_rs))
			
			if (_rs < 2048) {	
				buf_size = fwrite(mmap_addr+(start+(rs-_rs)), 1, _rs, null_fp);
			}
			else {
				buf_size = fwrite(mmap_addr+(start+(rs-_rs)), 1, 2048, null_fp);
			}
			_rs -= buf_size;
		}

		int _end = f_size - (start + rs);
		while (_end > 0) {
			char buf[2048];
			int buf_size;

			if (_end < 2048) {
				buf_size = fwrite(mmap_addr+start+rs+(f_size-_end), 1, _end, out_fp);
			}
			else {
				buf_size = fwrite(mmap_addr+start+rs+(f_size-_end), 1, 2048, out_fp);
			}
			_end -= buf_size;
		}
                        
		if (fflush(out_fp) == -1) {
			perror("ERROR: flush");
			exit(1);
		}

		int bt;
		if ((bt = byte_count_file(out_path)) > (f_size - rs)) {
			if (truncate(out_path, f_size - rs) == -1) {
				perror("ERROR: subset truncate");
				exit(1);
			}
		}
		else if (bt < (f_size - rs)) {
			fprintf(stderr, "ERROR: few written. bt:%d, correct: %ld \n", bt, (f_size - rs));
			exit(1);
		}

		int e_code = test_buffer_overflow(program_path, out_path, err_msg);
		if ((++iter_no) % 1000 == 0) {
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			fprintf(stderr, "Test count:%d -  %d-%d-%d %d:%d:%d\n", iter_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		}
		file_no++;
		if (e_code == 1) {
			fclose(out_fp);
			fclose(null_fp);
			fprintf(stderr, "FAIL: (%d,%d) \n", start, (start+rs));
			return (void*)out_path;
		}
		else {
			fprintf(stderr, "PASS: (%d,%d) \n", start, (start+rs));
		}
	}

	free(out_path);
	fclose(out_fp);
	fclose(null_fp);

	return NULL;
}
