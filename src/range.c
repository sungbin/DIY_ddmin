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
extern int iter_no;

pthread_t p_threads[THREAD_N];
pthread_mutex_t check_mt;
pthread_mutex_t fill_mt;

int finded_n = 0;
int start_arr[THREAD_N];
char * fail_path[THREAD_N];

int
run_threads (char ** ret_list, void *test_range_func, int rs, char * program_path, char * mmap_addr, long f_size, char * err_msg) {

	memset(fail_path, -1, THREAD_N);
	printf("run_thread() rs=%d\n", rs);

	int ret_cnt = 0;
	struct pthread_data * p_data_arr[THREAD_N];

	char * checked_arr = malloc(sizeof(char) * (f_size - rs + 1));
	memset(checked_arr, 0, (f_size - rs + 1));

	struct range_data * range_data = malloc(sizeof(struct range_data));

	range_data->checked_arr = checked_arr;
	range_data->rs = rs;
	range_data->program_path = program_path;
	range_data->mmap_addr = mmap_addr;
	range_data->f_size = f_size;
	range_data->err_msg = err_msg;

	finded_n = 0;
	pthread_t p_threads[THREAD_N];
        for (int i = 0; i < THREAD_N; i++) {

		struct pthread_data * p_data = malloc(sizeof(struct pthread_data));

		p_data->range_data = range_data;
		p_data->thread_n = i;
		p_data->out_idx = file_no + i;
                p_data_arr[i] = p_data;

                int rc = pthread_create(&p_threads[i], NULL, test_range_func, (void*)p_data);
                if (rc) {
                        perror("ERROR: pthread create");
                        exit(1);
                }
        }

	int any_fail = 0;
	void * retval;
	int finded_thread_idx = -1;
	while (1) {
		for (int i = 0; i < THREAD_N; i++) {
			if (pthread_tryjoin_np(p_threads[i], retval) == 0) {
				finded_thread_idx = i;
				any_fail = 1;
				break;
			}
		}
		if (any_fail) {
			break;
		}
	}

	for (int i = 0; i < THREAD_N; i++) {
	
		if (i == finded_thread_idx) {
			continue;
		}
                int rc = pthread_join(p_threads[i], retval); 
                if (rc == -1) {
                        fprintf(stderr,"\nERROR: # Thread join %d \n", i);
                        exit(1);
		}
	}


	while (finded_n > 0) {
		int max_start = -1;
		int max_idx = -1;
		for (int i = 0; i < THREAD_N; i++) {
			if (max_start < start_arr[i]) {
				max_start = start_arr[i];
				max_idx = i;
			}
		}
		ret_list[ret_cnt] = fail_path[max_idx];
		ret_cnt ++;
		start_arr[max_idx] = -1;
		finded_n--;
	}

	free(range_data);
	free(checked_arr);

	return ret_cnt;
}

// data: (int ** parts, int max_range_n)
void * 
test_range (void *data) {

	struct pthread_data * p_data = (struct pthread_data *)data;

	char * checked_arr = p_data->range_data->checked_arr;
	int rs = p_data->range_data->rs;
	long f_size = p_data->range_data->f_size;
	char * program_path = p_data->range_data->program_path;
	char * mmap_addr = p_data->range_data->mmap_addr;
        char * err_msg = p_data->range_data->err_msg;

	int thread_n = p_data->thread_n;
	int out_idx = p_data->out_idx;

	char * out_path = malloc(sizeof(char)*256);
	sprintf(out_path, "./%d.part", out_idx);
	FILE * out_fp = fopen(out_path, "wb");
	int out_fd = fileno(out_fp);

	FILE * null_fp = fopen("/dev/null", "wb");

	//fprintf(stderr, "thread %d: %s (fsize=%ld, rs=%d)\n", thread_n, out_path, f_size, rs);

	for (int i = 0; i <= (f_size - rs + 1); i++) {

		pthread_mutex_lock(&check_mt);
		while (checked_arr[i]) {
			i++;
		}
		if (i > (f_size - rs + 1)) {
			pthread_mutex_unlock(&check_mt);
			break;
		}
		checked_arr[i] = 1;
		pthread_mutex_unlock(&check_mt);

		int start = i;
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

		int e_code = test_buffer_overflow_thread(program_path, out_path, err_msg, thread_n);
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

			pthread_mutex_lock(&fill_mt);

			fail_path[finded_n] = out_path;
			start_arr[finded_n] = start;
			finded_n++;

			pthread_mutex_unlock(&fill_mt);

			return NULL;
			//return (void*)out_path;
		}
		else {
			//fprintf(stderr, "PASS: (%d,%d) \n", start, (start+rs));
		}
		if (finded_n) {
			break;
		}
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

	/* ## thread start ## */
	int ret_cnt = run_threads(ret_list, test_range, rs, program_path, mmap_addr, f_size, err_msg);

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

		fprintf(stderr, "rs: %ld, ret_cnt: %d \n", rs, ret_cnt);

        	if (ret_cnt < 1) {
                	free(ret_list);
			rs--;
			continue;
        	}
		
        	// TODO: select one among many failing inputs
        	for (int i = 1; i < ret_cnt; i++) {
			
                	free(ret_list[i]);
        	}
		if (ret_list[0] != NULL) {
			input_path = ret_list[0];
			rs = byte_count_file(input_path)-1;
			fprintf(stderr, "find! %s, size: %ld\n", input_path, (rs+1));
			file_no += THREAD_N;
		}

		free(ret_list);
	}

	return input_path;

}

void init_cursor(int in_fd, int out_fd) {
	
	if (lseek(in_fd, 0, SEEK_SET) == -1) {
		perror("lseek(in_fd)");
		exit(1);
	}
	if (lseek(out_fd, 0, SEEK_SET) == -1) {
		perror("lseek(out_fd)");
		exit(1);
	}
}

void read_and_write(FILE * in_fp, FILE * out_fp, int n) {

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
	sprintf(out_file, "./%d.part", ++iter_no);

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
				sprintf(out_file, "./%d.part", ++iter_no);
				out_fp = fopen(out_file, "wb");
				in_fd = fileno(in_fp);
				out_fd = fileno(out_fp);

				begin = MAX(begin - rs + 1, 0);

			}
			else {
				begin++;
			}
		}
		rs ++;
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
	sprintf(out_file, "../%s/%d.part", exe_dir, ++iter_no);

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
				sprintf(out_file, "../%s/%d.part", exe_dir, ++iter_no);
				out_fp = fopen(out_file, "wb");
				in_fd = fileno(in_fp);
				out_fd = fileno(out_fp);

				begin = MAX(begin - rs + 1, 0);

			}
			else {
				begin++;
			}
		}
		rs ++;
	}
	return strdup(input_path);
}





char *
range_dir (char * program_path, char * input_dir, char * input_name, int rs, char * err_msg, char * exe_dir) {

	char input_path[256];
	sprintf(input_path, "../%s/%s", exe_dir, input_name);

	long input_size = byte_count_file(input_path);
	
	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "../%s/%d.part", exe_dir, ++iter_no);

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



