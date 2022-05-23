#define THREAD_N 8

#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/ddmin.h"

int begin;
int cur_rs;
int ip_size;
pthread_mutex_t begin_mt;

char * program_path;
char * input_path;
char * err_msg;

char * out_files[THREAD_N+1];

FILE * null_fp;

FILE * in_fp;
FILE * tin_fp;

//FILE * in_fps[THREAD_N+1];
FILE * out_fps[THREAD_N+1];

//int in_fds[THREAD_N+1];
int in_fd;
int tin_fd;

int out_fds[THREAD_N+1];

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
init_cursor_thread(int out_fd) {

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

void cut_range_thread (int begin, int rs, int input_size, FILE * in_fp, FILE * out_fp, int in_fd, int out_fd, char * out_file) {

	//ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);

	//ssize_t pwrite(int filedes, void *buf, size_t nbyte, off_t offset);
	
	/*
	int j;
	for (int i = 0; i < 2000; i++) {
		j+=i;
	}

	*/
	init_cursor_thread(out_fd);

	char buf[2048];
	int buf_size = 0;
	int w_size = 0;

	if (input_size < 2048) {
		buf_size = pread(in_fd, buf, begin, 0);
		fwrite(buf, 1, buf_size, out_fp);

		buf_size = pread(in_fd, buf, (input_size-(begin+rs)), (begin+rs));
		fwrite((buf+begin), 1, buf_size, out_fp);
	}

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

}
void cut_range (int begin, int rs, int input_size, FILE * in_fp, FILE * out_fp, int in_fd, int out_fd, char * out_file) {

	//fprintf(stderr, "begin=%d, rs=%d, size=%d, file=%s \n", begin, rs, input_size, out_file);

	/*
	int j;
	for (int i = 0; i < 2000; i++) {
		j+=i;
	}
	*/
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
}

void test_json () {
	int ret = test_buffer_overflow(program_path, input_path, err_msg);
}

void test_json_th (int out_idx) {

	int ret = test_buffer_overflow_thread(program_path, input_path, err_msg, out_idx);
}

void no_thread (int input_size, int rs) {

	while (rs > 0) {
		ip_size = input_size;


		for (int begin = 0; begin <= input_size - rs; begin++) {

			//test_json();
			cut_range(begin, rs, ip_size, tin_fp, out_fps[0], tin_fd, out_fds[0], out_files[0]);
			cut_range(begin, rs, ip_size, tin_fp, out_fps[0], tin_fd, out_fds[0], out_files[0]);
			cut_range(begin, rs, ip_size, tin_fp, out_fps[0], tin_fd, out_fds[0], out_files[0]);
			cut_range(begin, rs, ip_size, tin_fp, out_fps[0], tin_fd, out_fds[0], out_files[0]);
			cut_range(begin, rs, ip_size, tin_fp, out_fps[0], tin_fd, out_fds[0], out_files[0]);
			
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
		//fprintf(stderr, "start=%d, rs=%d, size=%d \n", start, cur_rs, ip_size);
		//fprintf(stderr, "begin=%d, rs=%d, th=%d \n", start, cur_rs, th_idx);
		//test_json_th(th_idx);
		cut_range_thread(start, cur_rs, ip_size, tin_fp, out_fps[th_idx], tin_fd, out_fds[th_idx], out_files[th_idx]);
		cut_range_thread(start, cur_rs, ip_size, tin_fp, out_fps[th_idx], tin_fd, out_fds[th_idx], out_files[th_idx]);
		cut_range_thread(start, cur_rs, ip_size, tin_fp, out_fps[th_idx], tin_fd, out_fds[th_idx], out_files[th_idx]);
		cut_range_thread(start, cur_rs, ip_size, tin_fp, out_fps[th_idx], tin_fd, out_fds[th_idx], out_files[th_idx]);
		cut_range_thread(start, cur_rs, ip_size, tin_fp, out_fps[th_idx], tin_fd, out_fds[th_idx], out_files[th_idx]);
		//fprintf(stderr, "!! begin=%d, rs=%d, th=%d \n", start, cur_rs, th_idx);

	} while (start <= ip_size-cur_rs);


	return NULL;
}

void thread (int input_size, int rs) {

	pthread_t p_threads[THREAD_N];

	while (rs > 0) {

		//fprintf(stderr, "rs=%d \n", rs);

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

	fprintf(stderr, "program: %s, input: %s, err: %s \n", argv[1], argv[2], argv[3]);
	pthread_mutex_init(&begin_mt, NULL);

	program_path = argv[1];
	input_path = argv[2];
	err_msg = argv[3];

	int start_input_size = 378;
	int start_rs = start_input_size - 1;

	struct timeval  tv;
	double start, end;
	double res;

	for (int i = 0; i < THREAD_N+1; i++) {
		out_files[i] = malloc(sizeof(32));
		sprintf(out_files[i], "thread%d",i);
		//in_fps[i] = fopen(input_path, "rb");
		out_fps[i] = fopen(out_files[i], "wb");
		//in_fds[i] = fileno(in_fps[i]);
		out_fds[i] = fileno(out_fps[i]);
	}
	null_fp = fopen("/dev/null", "wb");
	in_fp = fopen(input_path, "rb");
	tin_fp = fopen(input_path, "rb");
	in_fd = fileno(in_fp);
	tin_fd = fileno(tin_fp);

	gettimeofday(&tv, NULL);
	start = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	no_thread(start_input_size, start_rs);
	gettimeofday(&tv, NULL);
	end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	res = (end - start)/1000;
	fprintf(stderr, "no_thread(): %f \n", res);
	/*
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
		free(out_files[i]);
	}
	fclose(null_fp);
	fclose(in_fp);
	fclose(tin_fp);
	pthread_mutex_destroy(&begin_mt);

	return 0;
}
