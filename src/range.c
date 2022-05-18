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


#include "../include/range.h"
#include "../include/ddmin.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern int file_no;
extern int fail_no;

struct pthread_data ** p_data_arr;
int * checked_rs_arr = 0x0;
int * checked_begin_arr = 0x0;

pthread_mutex_t begin_mt;
pthread_mutex_t wait_mt;
pthread_mutex_t rs_mt;

int finished;
int cur_rs;
int wait_cnt = 0;

// data: (int ** parts, int max_range_n)
void * 
test_range (void *data) {
	
	struct pthread_data * d = (struct pthread_data*) data;

	char out_file[256];
	sprintf(out_file, "./thread%d.part", d->thread_idx);
	
	FILE * in_fp = fopen(d->input_path, "rb");
	int in_fd = fileno(in_fp);
	FILE * null_fp = fopen("/dev/null", "wb");

	FILE * out_fp = fopen(out_file, "wb");
	if (out_fp == NULL) {
		perror("ERROR: open *FILE: out_fp");
		exit(1);
	}
	int out_fd = fileno(out_fp);

	while (d->rs > 0) {
		char fail_path_arr[2048][16];
		int f_cnt = 0;

		for (int begin = 0; begin <= d->input_size - d->rs; begin++) {
			//fprintf(stderr, "@@@ thread=%d, d->rs=%d, begin=%d \n", d->thread_idx, d->rs, begin);
	
	
			//fprintf(stderr, "@\twait lock @ thread=%d, d->rs=%d, begin=%d \n", d->thread_idx, d->rs, begin);
			pthread_mutex_lock(&begin_mt);
			//fprintf(stderr, "@\tget lock @ thread=%d, d->rs=%d, begin=%d \n", d->thread_idx, d->rs, begin);
                	while (checked_begin_arr[begin]) {
				//fprintf(stderr, "..");
                        	begin++;
                	}
                	if (begin > (d->input_size - d->rs)) {
				//fprintf(stderr, "@\t@ break: thread=%d, d->rs=%d, begin=%d \n", d->thread_idx, d->rs, begin);
                        	pthread_mutex_unlock(&begin_mt);
				//fprintf(stderr, "@\tunlock @ thread=%d, d->rs=%d, begin=%d \n", d->thread_idx, d->rs, begin);
                        	break;
                	}
                	checked_begin_arr[begin] = 1;
                	pthread_mutex_unlock(&begin_mt);
			//fprintf(stderr, "@\tunlock @ thread=%d, d->rs=%d, begin=%d \n", d->thread_idx, d->rs, begin);
	
	

			init_cursor(in_fd, out_fd);
			read_and_write(in_fp, out_fp, begin); //prefix
			read_and_write(in_fp, null_fp, d->rs); //rs
			read_and_write(in_fp, out_fp, d->input_size - (begin + d->rs)); //postfix
			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}
			int bt;
			if ((bt = byte_count_file(out_file)) > (d->input_size - d->rs)) {
				if (truncate(out_file, d->input_size - d->rs) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < (d->input_size - d->rs)) {
				fprintf(stderr, "ERROR: few written. bt:%d, correct: %ld \n", bt, (d->input_size - d->rs));
				exit(1);
			}
			int e_code = test_buffer_overflow(d->program_path, out_file, d->err_msg);
			if (file_no % 1000 == 0) {
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				fprintf(stderr, "(rs=%d)Test count:%d -  %d-%d-%d %d:%d:%d\n", d->rs, file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
			}
			file_no++;
	
			if (e_code == 1) {

				//fprintf(stderr, "1 find!(%s), begin=%d, rs=%d\n", out_file, begin, d->rs);

				fclose(out_fp);
				cur_rs = d->rs;

				strcpy(fail_path_arr[f_cnt], out_file);
                                f_cnt++;
				sprintf(out_file, "./thread%d.part", (d->thread_idx + (THREAD_N*f_cnt)));
				//fprintf(stderr, "next name(%s), begin=%d, rs=%d\n", out_file, begin, d->rs);
				out_fp = fopen(out_file, "wb");

				finished = 1;
				//checked_begin_arr[begin] = 2;
			}
		}

		pthread_mutex_lock(&wait_mt);
		wait_cnt++;
		pthread_mutex_unlock(&wait_mt);
		//fprintf(stderr, "@@@ wait thread=%d, rs=%d, val=%d wait_cnt:%d\n", d->thread_idx, d->rs, checked_rs_arr[d->rs], wait_cnt);
		while ((wait_cnt % 8 != 0) && !(checked_rs_arr[d->rs])) { }

		if (finished) {

			//fprintf(stderr, "finished: f_cnt=%d \n", f_cnt);

			fclose(null_fp);
			fclose(out_fp);
			fclose(in_fp);

			if (f_cnt < 1) {
				return NULL;
			}
			char ** fail_arr = malloc(sizeof(char*)*f_cnt);
			for (int j = 0; j < f_cnt; j++) {
				//fprintf(stderr, "find! %s \n", fail_path_arr[j]);
				fail_arr[j] = strdup(fail_path_arr[j]);
			}
			struct pthread_return * ret = malloc(sizeof(struct pthread_return));
			ret->fail_arr = fail_arr;
			ret->fail_n = f_cnt;
			return ret;
		}

		pthread_mutex_lock(&rs_mt);
		//fprintf(stderr, "#\t# rs lock thread=%d, rs=%d, val=%d wait_cnt:%d\n", d->thread_idx, d->rs, checked_rs_arr[d->rs], wait_cnt);
		if (checked_rs_arr[d->rs] == 0) {
			wait_cnt = 0;
			memset(checked_begin_arr, 0, sizeof(int)*(d->input_size - d->rs + 1));
			checked_rs_arr[d->rs] = 1;
		}
		pthread_mutex_unlock(&rs_mt);
		//fprintf(stderr, "#\t# rs unlock thread=%d, rs=%d, val=%d wait_cnt:%d\n", d->thread_idx, d->rs, checked_rs_arr[d->rs], wait_cnt);
		d->rs--;
	}

	//finished = 1;
	fclose(null_fp);
	fclose(out_fp);
	fclose(in_fp);
	return NULL;
}


char*
_range (char * program_path, char * input_path, char * err_msg, long input_size, int rs) {

	int last_file_no = file_no;

	//fprintf(stderr, "_range start! (f_size=%ld)\n", f_size);

	pthread_t p_threads[THREAD_N];
	wait_cnt = 0;
	finished = 0;

	for (int i = 0; i < THREAD_N; i++) {
		struct pthread_data * p_data = p_data_arr[i];
		p_data->program_path = program_path;
		p_data->input_path = input_path;
		p_data->input_size = input_size;
		p_data->err_msg = err_msg;
		p_data->rs = rs;
		p_data->thread_idx = i;
		int rc = pthread_create(&p_threads[i], NULL, test_range, (void*) p_data);
		if (rc) {
			perror("ERROR: pthread create");
			exit(1);
		}
	}
	char * fail_arr[2048];
	int fail_n = 0;
	void * retval;
	int first_finished_idx = -1;
	/*
	for (int i = 0; i < THREAD_N; i++) {
		if (pthread_tryjoin_np(p_threads[i], &retval) == 0) {
			if (retval != NULL) {
				 struct pthread_return * r = (struct pthread_return*) retval;
				 for (int j = 0; j < r->fail_n; j++) {
					fail_arr[fail_n++] = r->fail_arr[j];
					fprintf(stderr, "fail_arr[%d]: %s\n", (fail_n-1), fail_arr[r->fail_n-1]);
					exit(1);
				 }
			}
			first_finished_idx = i;
			break;
		}
	}
	*/
	for (int i = 0; i < THREAD_N; i++) {
		if (i == first_finished_idx) {
			continue;
		}
		void * retval;
		int rc = pthread_join(p_threads[i], &retval);
		if (rc == -1) {
			fprintf(stderr,"\nERROR: # Thread join %d \n", i);
		}
		if (retval != NULL) {
			struct pthread_return * r = (struct pthread_return*) retval;
			for (int j = 0; j < r->fail_n; j++) {
				fail_arr[fail_n++] = r->fail_arr[j];
				//fprintf(stderr, "fail_arr[%d]: %s\n", (fail_n-1), fail_arr[r->fail_n-1]);
			}
		}
	}

	if (fail_n < 1) {
		return input_path;
	}
	else {

		int rand_idx = rand()%fail_n;
		fprintf(stderr, "select %d in [0, %d) \n", rand_idx, fail_n);
		char * finded_path = fail_arr[rand_idx];

		for (int i = 0; i < fail_n; i++) {
			if (i != rand_idx) {
				free(fail_arr[i]);
			}
		}

		char * out_file = calloc(sizeof(char), 512);
        	sprintf(out_file, "./%d.part", ++fail_no);

		fprintf(stderr, "last minimized: %ld -> %ld(%s), used_test=%d\n", input_size, (input_size-cur_rs), out_file,(file_no-last_file_no));
		int rv = rename(finded_path, out_file);
		if (rv == -1) {
			fprintf(stderr, "%s (%s->%s)",strerror(errno), finded_path, out_file);
			exit(1);
		}

		memset(checked_begin_arr, 0, sizeof(int)*(input_size));
		memset(checked_rs_arr, 0, sizeof(int)*(input_size));

		free(finded_path);
		finded_path = _range(program_path, out_file, err_msg, (input_size-cur_rs), MIN(cur_rs, (input_size-cur_rs-1)));
		return finded_path;
	}
}

char *
range (char * program_path, char * input_path, char * err_msg) {

	long input_size = byte_count_file(input_path);
	checked_begin_arr = malloc(sizeof(int) * input_size);
	checked_rs_arr = malloc(sizeof(int) * input_size);
	memset(checked_begin_arr, 0, sizeof(int)*(input_size));
	memset(checked_rs_arr, 0, sizeof(int)*(input_size));
	p_data_arr = malloc(sizeof(struct pthread_data*) * THREAD_N);
	for (int i = 0; i < THREAD_N; i++) {
		p_data_arr[i] = malloc(sizeof(struct pthread_data));
	}

	char * ret_path = _range(program_path, input_path, err_msg, input_size, input_size-1);

	for (int i = 0; i < THREAD_N; i++) {
		free(p_data_arr[i]);
	}
	free(p_data_arr);
	free(checked_begin_arr);
	free(checked_rs_arr);

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



