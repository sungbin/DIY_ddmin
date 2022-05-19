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
//int * checked_rs_arr = 0x0;
//int * checked_begin_arr = 0x0;

pthread_mutex_t begin_mt;
pthread_mutex_t wait_mt;
pthread_mutex_t rs_mt;

int begin;
//int finished;
//int cur_rs;
//int wait_cnt = 0;

void * 
test_range (void *data) {
	
	struct pthread_data * d = (struct pthread_data*) data;

	int start;

	do {

		pthread_mutex_lock(&begin_mt);
		start = begin;
		if (start > d->input_size - d->rs) {
			pthread_mutex_unlock(&begin_mt);
			break;
		}
		begin++;
		pthread_mutex_unlock(&begin_mt);
		//fprintf(stderr, "lock (th=%d, rs=%d, begin=%d, out=%s) \n", d->thread_idx, d->rs, start, d->out_file);

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
		int e_code = test_buffer_overflow(d->program_path, d->out_file, d->err_msg);
		if (file_no % 1000 == 0) {
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			fprintf(stderr, "(rs=%d)Test count:%d -  %d-%d-%d %d:%d:%d\n", d->rs, file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
		file_no++;

		if (e_code == 1) {

			return d->out_file;
		}
		//fprintf(stderr, "unlock (th=%d, rs=%d, begin=%d, out=%s) \n", d->thread_idx, d->rs, start, d->out_file);
	} while (start <= d->input_size - d->rs);

	return NULL;
}


char*
_range (char * program_path, char * input_path, char * err_msg, long input_size, int rs) {

	fprintf(stderr, "last minimized: %s, %d\n", input_path, input_size);
	int last_file_no = file_no;

	pthread_t p_threads[THREAD_N];
	
	//fprintf(stderr, "_range program_path=%s, input_path=%s, input_size=%ld, rs=%d \n", program_path, input_path, input_size, rs);

	for (int i = 0; i < THREAD_N; i++) {
		struct pthread_data * p = p_data_arr[i];
		p->program_path = program_path;
		p->input_path = input_path;
		p->input_size = input_size;
		p->err_msg = err_msg;
		p->thread_idx = i;

		if (p->in_fp != 0x0) {
			fclose(p->in_fp);
		}
		p->in_fp = fopen(input_path, "rb");
		p->in_fd = fileno(p->in_fp);
		char * out_file = malloc(sizeof(char) * 32);
		sprintf(out_file, "./thread%d.part", (p->thread_idx+THREAD_N*p->total_finded_n));
		p->out_file = out_file;
		p->out_fp = fopen(out_file, "wb");
		p->out_fd = fileno(p->out_fp);

	}

	for (int cur_rs = rs; cur_rs > 0; cur_rs--) {

		begin = 0;
		
		for (int i = 0; i < THREAD_N; i++) {
			p_data_arr[i]->rs = cur_rs;
			int rc = pthread_create(&p_threads[i], NULL, test_range, (void*) p_data_arr[i]);
			if (rc) {
				perror("ERROR: pthread create");
				exit(1);
			}
		}

		char * fail_arr[2048];
		int fail_n = 0;
		int is_terminated_th_arr[THREAD_N] = { 0 };
		void * retval;


		
		do {
			for (int i = 0; i < THREAD_N; i++) {
				if (is_terminated_th_arr[i]) {
					continue;
				}
				int rc = pthread_tryjoin_np(p_threads[i], &retval);
				if (rc != 0) {
					continue;
				}
				if (retval != NULL) {

					char * fail_path = (char*) retval;
					fail_arr[fail_n++] = fail_path;

					fclose(p_data_arr[i]->out_fp);
					p_data_arr[i]->total_finded_n++;
					p_data_arr[i]->out_file = malloc(sizeof(char) * 32);
					sprintf(p_data_arr[i]->out_file, "./thread%d.part", (p_data_arr[i]->thread_idx+THREAD_N*p_data_arr[i]->total_finded_n));
					p_data_arr[i]->out_fp = fopen(p_data_arr[i]->out_file, "wb");
					p_data_arr[i]->out_fd = fileno(p_data_arr[i]->out_fp);

					int rc = pthread_create(&p_threads[i], NULL, test_range, (void*) p_data_arr[i]);
					if (rc) {
						perror("ERROR: pthread create");
						exit(1);
					}
				}
				else {
					is_terminated_th_arr[i] = 1;
				}
			}
		} while (begin <= input_size - rs);

		for (int i = 0; i < THREAD_N; i++) {

			if (! is_terminated_th_arr[i]) {

				int rc = pthread_join(p_threads[i], &retval);
				if (rc != 0) {
					perror("fail: pthread_join()");
					exit(1);
				}
				if (retval != NULL) {

					char * fail_path = (char*) retval;
					fail_arr[fail_n++] = fail_path;

					fclose(p_data_arr[i]->out_fp);
					p_data_arr[i]->total_finded_n++;
					p_data_arr[i]->out_file = malloc(sizeof(char) * 32);
					sprintf(p_data_arr[i]->out_file, "./thread%d.part", (p_data_arr[i]->thread_idx+THREAD_N*p_data_arr[i]->total_finded_n));
					p_data_arr[i]->out_fp = fopen(p_data_arr[i]->out_file, "wb");
					p_data_arr[i]->out_fd = fileno(p_data_arr[i]->out_fp);

				}
			}
		}
		
		if (fail_n > 0) {
	
			int rand_idx = rand()%fail_n;
			fprintf(stderr, "select %d in [0, %d) \n", rand_idx, fail_n);
			char * finded_path = fail_arr[rand_idx];
	
			for (int i = 0; i < fail_n; i++) {
				if (i != rand_idx) {
					free(fail_arr[i]);
				}
			}
	
			finded_path = _range(program_path, finded_path, err_msg, (input_size-cur_rs), MIN(cur_rs, (input_size-cur_rs-1)));
			return finded_path;
		}
	}
	return input_path;
}

char *
range (char * program_path, char * input_path, char * err_msg) {

	long input_size = byte_count_file(input_path);
	p_data_arr = malloc(sizeof(struct pthread_data*) * THREAD_N);
	for (int i = 0; i < THREAD_N; i++) {
		p_data_arr[i] = malloc(sizeof(struct pthread_data));
		p_data_arr[i]->in_fp = 0x0;
		p_data_arr[i]->null_fp = fopen("/dev/null", "wb");
		p_data_arr[i]->total_finded_n = 0;
	}

	char * ret_path = _range(program_path, input_path, err_msg, input_size, input_size-1);

	for (int i = 0; i < THREAD_N; i++) {

		fclose(p_data_arr[i]->null_fp);
		free(p_data_arr[i]);
	}
	free(p_data_arr);
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
		if (b_size != fwrite(buf, 1, b_size, out_fp)) {
			fprintf(stderr, "ERROR on write\n");
		}
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



