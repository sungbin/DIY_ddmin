#define THREAD_N 4

#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/ddmin.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int begin;
int cur_rs;
int ip_size;

pthread_mutex_t begin_mt;
pthread_mutex_t find_mt;

char * program_path;
char * input_path;
char * err_msg;

char * out_files[THREAD_N+1];

FILE * null_fp;

FILE * in_fp;
//FILE * tin_fp;

FILE * in_fps[THREAD_N+1];
FILE * out_fps[THREAD_N+1];

int in_fds[THREAD_N+1];
int in_fd;
//int tin_fd;

int out_fds[THREAD_N+1];

char ** fail_arr;
int fail_max = 256;
int total_fail_n = 0;
int fail_n = 0;

void
init_cursor(int in_fd, int out_fd) {

        if (lseek(in_fd, 0, SEEK_SET) == -1) {
                perror("lseek(in_fd)");
                exit(1);
        }
        if (lseek(out_fd, 0, SEEK_SET) == -1) {
                perror("lseek(out_fd)");
                exit(1);
        }
}

void
read_and_write(FILE * in_fp, FILE * out_fp, int n) {

        while (n > 0) {

                char buf[2048];
                int b_size;

                if (n > 2048) {
                        b_size = fread(buf, 1, 2048, in_fp);
                }
                else {
                        b_size = fread(buf, 1, n, in_fp);
                }
                //fprintf(stderr, "read %d (n=%d)\n", b_size, n);
                if (b_size != fwrite(buf, 1, b_size, out_fp)) {
                        fprintf(stderr, "ERROR on write\n");
                        exit(1);
                }
                //fprintf(stderr, "write %d (n=%d)\n", b_size, n);
                n -= b_size;
        }

}

void run_runner (int begin, int rs, int input_size, FILE * in_fp, FILE * out_fp, int in_fd, int out_fd, char * out_file, int th_idx) {



	init_cursor(in_fd, out_fd);
	read_and_write(in_fp, out_fp, begin); //prefix
	read_and_write(in_fp, null_fp, rs); //rs
	read_and_write(in_fp, out_fp, input_size - (begin + rs)); //postfix
	if (fflush(out_fp) == -1) {
		perror("ERROR: flush");
		exit(1);
	}
	int bt;
	if ((bt = byte_count_file(out_file)) > (input_size - rs)) {
		if (truncate(out_file, input_size - rs) == -1) {
			fprintf(stderr, "%s: bt=%d != (%d)\n", strerror(errno), bt, (input_size - rs));
			exit(1);
		}
	}
	else if (bt < (input_size - rs)) {
		fprintf(stderr, "ERROR: few written. bt:%d, correct: %d \n", bt, (input_size - rs));
		exit(1);
	}
	/*
	*/

	int e_code = test_buffer_overflow_thread(program_path, out_file, err_msg, th_idx);
	//int e_code = test_buffer_overflow_thread(program_path, input_path, err_msg, th_idx);

	if (e_code == 1) {

		fprintf(stderr, "find! rs=%d, begin=%d, th_idx=%d path=%s \n", rs, begin, th_idx, out_file);

		pthread_mutex_lock(&find_mt);
		int fail_idx = fail_n;
		fail_n++;
		pthread_mutex_unlock(&find_mt);
		fail_arr[fail_idx] = out_file;

		fclose(out_fps[th_idx]);
		//fclose(d->out_fp);
		out_file = malloc(sizeof(char) * 32);
		out_files[th_idx] = out_file;
		fail_n++;
		sprintf(out_files[th_idx], "./thread%d", (th_idx+THREAD_N*fail_n));
		out_fps[th_idx] = fopen(out_file, "wb");
		out_fds[th_idx] = fileno(out_fps[th_idx]);
	}
	/*
	*/

	
}

void test_json () {
	int ret = test_buffer_overflow(program_path, input_path, err_msg);
}

void test_json_th (int out_idx) {

	int ret = test_buffer_overflow_thread(program_path, input_path, err_msg, out_idx);
}

void no_thread (int input_size, int rs) {

	while (rs > 0) {

		fprintf(stderr, "rs=%d \n", rs);

		ip_size = input_size;

		char p_data_arr[256][256];
		int cnt = 0;
		fail_n = 0;
		for (int begin = 0; begin <= input_size - rs; begin++) {
			run_runner(begin, rs, ip_size, in_fps[0], out_fps[0], in_fds[0], out_fds[0], out_files[0], 0);
		}

// (int begin, int rs, int input_size, FILE * in_fp, FILE * out_fp, int in_fd, int out_fd, char * out_file, int th_idx)

		if (fail_n > 0) {

			int select_idx = rand() % fail_n;
			printf("select %d in [0, %d) \n", select_idx, fail_n);
			char * fail_path = fail_arr[select_idx];
			for (int i = 0; i < fail_n; i++) {
				if (i == select_idx) {
					continue;
				}
				free(fail_arr[i]);
			}

			for (int i = 0; i < THREAD_N+1; i++) {
				fclose(in_fps[i]);
				fclose(out_fps[i]);

				in_fps[i] = fopen(fail_path, "rb");
				in_fds[i] = fileno(in_fps[i]);
			}
			input_path = fail_path;
			no_thread((input_size-rs), MIN(rs, (ip_size - rs - 1)));
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
		run_runner(start, cur_rs, ip_size, in_fps[th_idx], out_fps[th_idx], in_fds[th_idx], out_fds[th_idx], out_files[th_idx], th_idx);

	} while (start <= ip_size-cur_rs);


	return NULL;
}

void thread (int input_size, int rs) {

	pthread_t p_threads[THREAD_N];

	while (rs > 0) {

		fprintf(stderr, "rs=%d \n", rs);

		fail_n = 0;
		begin = 0;
		cur_rs = rs;
		ip_size = input_size;

		for (int i = 0; i < THREAD_N; i++) {
			pthread_create(&p_threads[i], NULL, thread_func, (void*) (i+1));
		}

		for (int i = 0; i < THREAD_N; i++) {
			pthread_join(p_threads[i], NULL);
		}

		if (fail_n > 0) {

			int select_idx = rand() % fail_n;
                        printf("select %d in [0, %d) \n", select_idx, fail_n);
                        char * fail_path = fail_arr[select_idx];

			total_fail_n++;
			char * ret_path = malloc(sizeof(char)*16);
			sprintf(ret_path, "./fail%d.part", total_fail_n);
			rename(fail_path, ret_path);
			input_path = ret_path;


                        for (int i = 0; i < fail_n; i++) {
				free(fail_arr[i]);
                        }

                        for (int i = 0; i < THREAD_N; i++) {
				fclose(in_fps[i]);
				fclose(out_fps[i]);

				sprintf(out_files[i], "thread%d",i);
				in_fps[i] = fopen(input_path, "rb");
				out_fps[i] = fopen(out_files[i], "wb");
				in_fds[i] = fileno(in_fps[i]);
				out_fds[i] = fileno(out_fps[i]);
                        }

			input_size = input_size - cur_rs;
			rs = MIN(cur_rs, input_size - 1);
			input_path = ret_path;
		}	


		rs--;
	}
}

int main (int argc, char * argv[]) {

	fprintf(stderr, "program: %s, input: %s, err: %s \n", argv[1], argv[2], argv[3]);
	pthread_mutex_init(&begin_mt, NULL);


	program_path = argv[1];
	input_path = strdup(argv[2]);
	err_msg = argv[3];

	int start_input_size = 378;
	int start_rs = start_input_size - 1;
	//int start_rs = 1;


	struct timeval  tv;
	double start, end;
	double res;

	fail_arr = malloc(sizeof(char*)*fail_max);

	for (int i = 0; i < THREAD_N+1; i++) {
		out_files[i] = malloc(sizeof(32));
		sprintf(out_files[i], "thread%d",i);
		in_fps[i] = fopen(input_path, "rb");
		out_fps[i] = fopen(out_files[i], "wb");
		in_fds[i] = fileno(in_fps[i]);
		out_fds[i] = fileno(out_fps[i]);
	}
	null_fp = fopen("/dev/null", "wb");
	in_fp = fopen(input_path, "rb");
	//tin_fp = fopen(input_path, "rb");
	in_fd = fileno(in_fp);
	//tin_fd = fileno(tin_fp);

	/*
	gettimeofday(&tv, NULL);
	start = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	no_thread(start_input_size, start_rs);
	gettimeofday(&tv, NULL);
	end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	res = (end - start)/1000;
	fprintf(stderr, "no_thread(): %f \n", res);
	*/

	gettimeofday(&tv, NULL);
	start = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	thread(start_input_size, start_rs);
	gettimeofday(&tv, NULL);
	end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	res = (end - start)/1000;
	fprintf(stderr, "thread(): %f \n", res);

	/*
*/
	/*
	gettimeofday(&tv, NULL);
	start = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	int ret;
	for (int i = 0; i < 100; i++)
		ret = test_buffer_overflow(argv[1], argv[2], argv[3]);
	gettimeofday(&tv, NULL);
	end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	res = (float)(end - start)/1000;
	fprintf(stderr, "ret: %d \n", ret);
	fprintf(stderr, "time: %f \n", res);

	*/
	for (int i = 0; i < THREAD_N; i++) {
		fclose(out_fps[i]);
		fclose(in_fps[i]);
		free(out_files[i]);
	}
	fclose(null_fp);
	fclose(in_fp);
	//fclose(tin_fp);
	pthread_mutex_destroy(&begin_mt);

	return 0;
}
