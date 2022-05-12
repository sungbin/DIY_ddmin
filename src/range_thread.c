#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>


#include "../../include/range_thread.h"
#include "../../include/ddmin.h"

extern int file_no;
extern int iter_no;

void
partitions (int ** parts, long f_size, int rs) {

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

int
run_threads (char ** ret_list, int ** parts, void *test_range_func, int max_range_n, int rs, char * program_path, char * mmap_addr, long f_size, char * err_msg) {

	int ret_cnt = 0;

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

	void* (*fp)(void*) = test_range_func;
	char * fail_path = fp((void*)data);

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
		if (retval == NULL) {
		
		}
		else {
			char * fail_path = (char*)retval;
			//fprintf(stderr, "fail_path: %s \n", fail_path);
			ret_list[ret_cnt] = fail_path;
			ret_cnt++;
		}
        }
	if (fail_path != NULL) {
		 //fprintf(stderr, "fail_path: %s \n", fail_path);
		 ret_list[ret_cnt] = fail_path;
		 ret_cnt++;
	}
	free(data);

	//file_no = file_no + THREAD_N;
	return ret_cnt;
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

	//fprintf(stderr, "THREAD: %d - out_idx - %s \n", thread_n, out_path);

	//int temp = 0;
	for (int j = 0; j < max_range_n; j++) {
		if (part[j] == -1) {
			continue;
		}
		int start = part[j];
		//fprintf(stderr, "%s - start: %d, rs: %d \n", out_path, start, rs);
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
				buf_size = fwrite(mmap_addr+(f_size-_end), 1, _end, out_fp);
			}
			else {
				buf_size = fwrite(mmap_addr+(f_size-_end), 1, 2048, out_fp);
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
			fprintf(stderr, "(size=%ld, rs=%d)Test count:%d -  %d-%d-%d %d:%d:%d\n", f_size, rs, iter_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		}
		if (e_code == 1) {
			fclose(out_fp);
			fclose(null_fp);
			printf("minimized %s (%ld -> %ld) \n", out_path, f_size, (f_size-rs));
			printf("begin: %d, rs:%d \n\n", start, rs);
			return (void*)out_path;
		}
		else {
			//fprintf(stderr, "PASS: (%d,%d) \n", start, (start+rs));
		}
		//if ((++temp) == 1) {
		//	break;
		//}
	}

	free(out_path);
	fclose(out_fp);
	fclose(null_fp);

	return NULL;
}

int
range_thread (char ** ret_list, char * program_path, char * input_path, int rs, char * err_msg) {

	long f_size = byte_count_file(input_path);
	int fd;
	char * mmap_addr;

	if((fd = open(input_path, O_RDONLY)) == -1) {
		perror("ERROR: open fd");
		exit(1);
	}
	mmap_addr = mmap(0, f_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mmap_addr == MAP_FAILED) {
		perror("mmap error");
		exit(1);
	}

	//fprintf(stderr, "f_size: %ld \n", f_size);
	//fprintf(stderr, "mmap_addr: %s \n", mmap_addr);

	int max_range_n = (int)ceilf(((f_size - rs + 1) / (float)THREAD_N));

	int ** parts = malloc(sizeof(int**)*THREAD_N);
	for (int i = 0; i < THREAD_N; i++) {
		parts[i] = malloc(sizeof(int*)*max_range_n);
		for (int j = 0; j < max_range_n; j++) {
			parts[i][j] = -1;
		}
	}

	partitions(parts, f_size, rs);
	//test_ranges(parts, max_range_n, rs);

	/* ## thread start ## */
	int ret_cnt = run_threads(ret_list, parts, test_range, max_range_n, rs, program_path, mmap_addr, f_size, err_msg);

	for (int i = 0; i < THREAD_N; i++) {
		free(parts[i]);
	}
	free(parts);
	close(fd);

	return ret_cnt;
}

char*
range (char * program_path, char * input_path, char * err_msg) {

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	fprintf(stderr, "Test count:%d -  %d-%d-%d %d:%d:%d\n", iter_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	long rs = byte_count_file(input_path)-1;
	while (rs > 0) {

        	char ** ret_list = malloc(sizeof(char*) * THREAD_N);
        	int ret_cnt = range_thread(ret_list, program_path, input_path, rs, err_msg);

		//fprintf(stderr, "rs: %ld, ret_cnt: %d \n", rs, ret_cnt);

        	if (ret_cnt < 1) {
                	free(ret_list);
			rs--;
			continue;
        	}
		
        	// TODO: select one among many failing inputs
        	for (int i = 1; i < ret_cnt; i++) {
                	free(ret_list[i]);
        	}
		input_path = ret_list[0];
		rs = byte_count_file(input_path)-1;
		file_no += THREAD_N;

		free(ret_list);
	}

	return input_path;

}
