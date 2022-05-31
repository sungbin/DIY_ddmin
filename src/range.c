#define _GNU_SOURCE

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
#include <errno.h>


#include "../include/range.h"
#include "../include/ddmin.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern int file_no;
extern int fail_no;

struct pthread_data ** p_data_arr;

pthread_mutex_t begin_mt;
pthread_mutex_t find_mt;

int begin;

int fail_max = 16;
char ** fail_arr;
int fail_n = 0;

void * 
test_range (void *data) {
	
	struct pthread_data * d = (struct pthread_data*) data;

	int start;

	do {

		pthread_mutex_lock(&begin_mt);
		start = begin;
		begin++;
		pthread_mutex_unlock(&begin_mt);
		if (start > d->input_size - d->rs) {
			break;
		}

		//fprintf(stderr, "unlock() (rs=%d, begin=%d, th=%d, input=%s)\n", d->rs, start, d->thread_idx, d->input_path);

		init_cursor(d->in_fd, d->out_fd);
		read_and_write(d->in_fp, d->out_fp, start); //prefix
		read_and_write(d->in_fp, d->null_fp, d->rs); //rs
		read_and_write(d->in_fp, d->out_fp, d->input_size - (start + d->rs)); //postfix
		if (fflush(d->out_fp) == -1) {
			perror("ERROR: flush");
			exit(1);
		}
		int bt;
		if ((bt = byte_count_file(d->out_file)) > (d->input_size - d->rs)) {
			if (truncate(d->out_file, d->input_size - d->rs) == -1) {
				perror("ERROR: subset truncate");
				exit(1);
			}
		}
		else if (bt < (d->input_size - d->rs)) {
			fprintf(stderr, "ERROR: few written. bt:%d, correct: %ld \n", bt, (d->input_size - d->rs));
			exit(1);
		}

		int e_code = test_buffer_overflow_thread(d->program_path, d->out_file, d->err_msg, (d->thread_idx+THREAD_N*d->finded_n));
		if (file_no % 1000 == 0) {
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			fprintf(stderr, "(rs=%d)Test count:%d -  %d-%d-%d %d:%d:%d\n", d->rs, file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
		file_no++;

		if (e_code == 1) {

			pthread_mutex_lock(&find_mt);
			int fail_idx = fail_n;
			fail_n++;
			pthread_mutex_unlock(&find_mt);

			if (fail_n >= (fail_max - THREAD_N)) {
				fail_max *= 2;
				fail_arr = realloc(fail_arr, sizeof(char*)*fail_max);
			}
			fail_arr[fail_idx] = d->out_file;

			fclose(d->out_fp);
			d->out_file = malloc(sizeof(char) * 32);
			d->finded_n++;
			sprintf(d->out_file, "./thread%d.part", (d->thread_idx+THREAD_N*d->finded_n));
			d->out_fp = fopen(d->out_file, "wb");
			d->out_fd = fileno(d->out_fp);
		}
	} while (start <= d->input_size - d->rs);

	return NULL;
}


char*
_range (char * program_path, char * input_path, char * err_msg, long input_size, int rs) {

	fprintf(stderr, "last minimized: %s, %ld\n", input_path, input_size);

	pthread_t p_threads[THREAD_N];

	for (int i = 0; i < THREAD_N; i++) {
		p_data_arr[i]->program_path = program_path;
		p_data_arr[i]->input_path = input_path;
		p_data_arr[i]->input_size = input_size;
		p_data_arr[i]->err_msg = err_msg;
		p_data_arr[i]->thread_idx = i;

		p_data_arr[i]->out_file = malloc(sizeof(char)*16);
		sprintf(p_data_arr[i]->out_file, "./thread%d.part", (i+THREAD_N*p_data_arr[i]->finded_n));
		p_data_arr[i]->in_fp = fopen(input_path, "rb");
		p_data_arr[i]->out_fp = fopen(p_data_arr[i]->out_file, "wb");
		p_data_arr[i]->in_fd = fileno(p_data_arr[i]->in_fp);
		p_data_arr[i]->out_fd = fileno(p_data_arr[i]->out_fp);
	}

	fail_n = 0;
	for (int cur_rs = rs; cur_rs > 0; cur_rs--) {
		begin = 0;

		for (int i = 0; i < THREAD_N; i++) {

			p_data_arr[i]->rs = cur_rs;
			pthread_create(&p_threads[i], NULL, test_range, (void *)p_data_arr[i]);
		}
		for (int i = 0; i < THREAD_N; i++) {

			pthread_join(p_threads[i], NULL);
		}

		if (fail_n > 0) {

			//char ** fail_arr;
			//int fail_n = 0;
			int select_idx = rand() % fail_n;
			printf("select %d in [0, %d) \n", select_idx, fail_n);
			char * fail_path = fail_arr[select_idx];
			for (int i = 0; i < fail_n; i++) {
				if (i != select_idx) {
					free(fail_arr[i]);
				}
			}
			for (int i = 0; i < THREAD_N; i++) {
				fclose(p_data_arr[i]->in_fp);
				fclose(p_data_arr[i]->out_fp);
			}

			return _range(program_path, fail_path, err_msg, (input_size-cur_rs), MIN(cur_rs, (input_size-cur_rs-1)));

		}
	}

	for (int i = 0; i < THREAD_N; i++) {
		fclose(p_data_arr[i]->in_fp);
		fclose(p_data_arr[i]->out_fp);
	}
	return input_path;

}
char *
range (char * program_path, char * input_path, char * err_msg) {

	pthread_mutex_init(&begin_mt, NULL);
	pthread_mutex_init(&find_mt, NULL);

	fail_arr = malloc(sizeof(char*)*fail_max);
	long input_size = byte_count_file(input_path);
	p_data_arr = malloc(sizeof(struct pthread_data*) * THREAD_N);
	for (int i = 0; i < THREAD_N; i++) {
		p_data_arr[i] = malloc(sizeof(struct pthread_data));
		p_data_arr[i]->null_fp = fopen("/dev/null", "wb");
		p_data_arr[i]->finded_n  = 0;
	}

	char * ret_path = _range(program_path, input_path, err_msg, input_size, input_size-1);

	for (int i = 0; i < THREAD_N; i++) {

		fclose(p_data_arr[i]->null_fp);
		free(p_data_arr[i]);
	}
	free(p_data_arr);
	free(fail_arr);

	pthread_mutex_destroy(&begin_mt);
	pthread_mutex_destroy(&find_mt);

	return ret_path;
}

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

char *
range_increasing (char * program_path, char * input_path, char * err_msg) {

	int rs = 1;
	int input_size;

	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "./%d.part", ++fail_no);

	FILE * in_fp = fopen(input_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");
	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	while (rs < (input_size = byte_count_file(input_path))) {
		printf("rs: %d\n", rs);
	
		int begin = 0;
		while (begin <= ((input_size = byte_count_file(input_path)) - rs)) {

			init_cursor(in_fd, out_fd);
			read_and_write(in_fp, out_fp, begin); //prefix
			FILE * null_fp = fopen("/dev/null", "wb");
			read_and_write(in_fp, null_fp, rs); //rs
			fclose(null_fp);
			read_and_write(in_fp, out_fp, input_size - (begin + rs)); //postfix
			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}
			int bt;
			if ((bt = byte_count_file(out_file)) > (input_size - rs)) {
				if (truncate(out_file, input_size - rs) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < (input_size - rs)) {
				fprintf(stderr, "ERROR: few written. bt:%d, correct: %d \n", bt, (input_size - rs));
				exit(1);
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (file_no % 1000 == 0) {
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				printf("Test count:%d -  %d-%d-%d %d:%d:%d\n", file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			}
                        if (e_code == 1) {

				printf("last minimized: %s, %d\n", out_file, bt);

				fclose(in_fp);
				fclose(out_fp);
				free(input_path);
				input_path = strdup(out_file);
				in_fp = fopen(input_path, "rb");
				sprintf(out_file, "./%d.part", ++fail_no);
				out_fp = fopen(out_file, "wb");
				in_fd = fileno(in_fp);
				out_fd = fileno(out_fp);

				begin = MAX(begin - rs + 1, 0);

			}
			else {
				begin++;
			}
		}
		rs++;
	}
	return input_path;

}

char *
range_increasing_dir (char * program_path, char * input_dir, char * input_name, char * err_msg, char * exe_dir) {

	int rs = 1;
	int input_size;

	char input_path[256];
	sprintf(input_path, "../%s/%s", exe_dir, input_name);

	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "../%s/%d.part", exe_dir, ++fail_no);

	FILE * in_fp = fopen(input_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");
	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	while (rs < (input_size = byte_count_file(input_path))) {
		printf("rs: %d \n", rs);
	
		int begin = 0;
		while (begin <= ((input_size = byte_count_file(input_path)) - rs)) {

			init_cursor(in_fd, out_fd);
			read_and_write(in_fp, out_fp, begin); //prefix
			FILE * null_fp = fopen("/dev/null", "wb");
			read_and_write(in_fp, null_fp, rs); //rs
			fclose(null_fp);
			read_and_write(in_fp, out_fp, input_size - (begin + rs)); //postfix
			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}
			int bt;
			if ((bt = byte_count_file(out_file)) > (input_size - rs)) {
				if (truncate(out_file, input_size - rs) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < (input_size - rs)) {
				fprintf(stderr, "ERROR: few written. bt:%d, correct: %d \n", bt, (input_size - rs));
				exit(1);
			}

			char path[256];
			sprintf(path, "../%s/inputs/test_input", exe_dir);
                        if (remove(path) == -1) {
                                perror("ERROR: fail to remove test_input");
				fprintf(stderr, "path:%s\n", path);
                                exit(1);
                        }
			copy(out_file, path);

			int e_code = test_buffer_overflow(program_path, input_dir, err_msg);
			if (file_no % 1000 == 0) {
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				printf("Test count:%d -  %d-%d-%d %d:%d:%d\n", file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			}
			file_no++;
                        if (e_code == 1) {

				printf("last minimized: %s, %d\n", out_file, bt);

				fclose(in_fp);
				fclose(out_fp);
				strcpy(input_path, out_file);
				in_fp = fopen(input_path, "rb");
				sprintf(out_file, "../%s/%d.part", exe_dir, ++fail_no);
				out_fp = fopen(out_file, "wb");
				in_fd = fileno(in_fp);
				out_fd = fileno(out_fp);

				begin = MAX(begin - rs + 1, 0);

			}
			else {
				begin++;
			}
		}
		rs++;
	}
	return strdup(input_path);
}





char *
range_dir (char * program_path, char * input_dir, char * input_name, int rs, char * err_msg, char * exe_dir) {

	char input_path[256];
	sprintf(input_path, "../%s/%s", exe_dir, input_name);

	long input_size = byte_count_file(input_path);
	
	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "../%s/%d.part", exe_dir, ++fail_no);

	FILE * in_fp = fopen(input_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");
	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	while (rs > 0) {
		//fprintf(stderr,"rs:%d\n", rs);
		for (int begin = 0; begin <= input_size - rs; begin++) {

			init_cursor(in_fd, out_fd);

			read_and_write(in_fp, out_fp, begin); //prefix

			FILE * null_fp = fopen("/dev/null", "wb");
			read_and_write(in_fp, null_fp, rs); //rs
			fclose(null_fp);

			read_and_write(in_fp, out_fp, input_size - (begin + rs)); //postfix

			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}
			int bt;
			if ((bt = byte_count_file(out_file)) > (input_size - rs)) {
				if (truncate(out_file, input_size - rs) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < (input_size - rs)) {
				fprintf(stderr, "ERROR: few written. bt:%d, correct: %ld \n", bt, (input_size - rs));
				exit(1);
			}

			char path[256];
			sprintf(path, "../%s/inputs/test_input", exe_dir);
                        if (remove(path) == -1) {
                                perror("ERROR: fail to remove test_input");
				fprintf(stderr, "path:%s\n", path);
                                exit(1);
                        }
			copy(out_file, path);

			int e_code = test_buffer_overflow(program_path, input_dir, err_msg);
			if (file_no % 1000 == 0) {
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				fprintf(stderr, "Test count:%d -  %d-%d-%d %d:%d:%d\n", file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			}
			file_no++;
                        if (e_code == 1) {
				fprintf(stderr, "last minimized: %s (%ld -> %ld) \n", out_file, input_size, (input_size-rs));
				fprintf(stderr, "begin: %d, rs: %d \n\n", begin, rs);

				fclose(out_fp);
				fclose(in_fp);
				return range_dir(program_path, input_dir, out_file, rs, err_msg, exe_dir);
			}
		}
		rs--;
	}
	fclose(out_fp);
	fclose(in_fp);

	return strdup(input_path);

}



