#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "../include/range.h"
#include "../include/ddmin.h"
#include "../include/runner.h"


#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


extern int file_no;
extern int iter_no;


<<<<<<< HEAD
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

	if (fflush(out_fp) == -1) {
		perror("ERROR: flush");
		exit(1);
	}
}

char *
range_increasing (char * program_path, char * input_path, int rs, char * err_msg) {
	
	long input_size = byte_count_file(input_path);
	fprintf(stderr, "last minimized: %s, %ld\n", input_path, input_size);
	
	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "./%d.part", file_no++);

	FILE * in_fp = fopen(input_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");
	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	while (rs < input_size) {
		for (int begin = 0; begin < (input_size - rs); begin++) {

			init_cursor(in_fd, out_fd);

			read_and_write(in_fp, out_fp, begin); //prefix

			FILE * null_fp = fopen("/dev/null", "wb");
			read_and_write(in_fp, null_fp, rs); //rs
			fclose(null_fp);

			read_and_write(in_fp, out_fp, input_size - (begin + rs)); //postfix

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

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
                        if (e_code == 1) {
				fclose(out_fp);
				fclose(in_fp);
				free(input_path);
				return range(program_path, out_file, rs, err_msg);
			}

		}
		rs++;
	}

	return input_path;
}

char *
range (char * program_path, char * char_seq_path, int rs, char * err_msg) {
	long seq_len;
	seq_len = byte_count_file(char_seq_path);

	fprintf(stderr, "last minimized: %s, %ld\n", char_seq_path, seq_len);

	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "./%d.part", file_no++);
	FILE * in_fp = fopen(char_seq_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");
	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	while (rs > 0) {
		fprintf(stderr,"rs:%d\n", rs);
		for (int begin = 0; begin <= seq_len - rs; begin++) {


			init_cursor(in_fd, out_fd);

			int end = begin + rs;
			int buf_size = 0;
			int _begin = begin;
			fprintf(stderr, "begin: %d \n", begin);
			while (_begin > 0) {
				char _buf[512];
				if (_begin < 511) {
					buf_size = fread(_buf, 1, _begin, in_fp);
				}
				else {
					buf_size = fread(_buf, 1, 511, in_fp);
				}
				if (buf_size < 1) {
					break;
				}
				_begin = _begin - buf_size;
				fwrite(_buf, 1, buf_size, out_fp);
				//_buf[buf_size] = 0x0;
				//fprintf(stderr, "** pre : %s, size: %d\n", _buf, buf_size);

			}

			int _rs = rs;
			while (_rs > 0) {
				char buf[512];
				buf_size = fread(buf, 1, _rs, in_fp);
				_rs = _rs - buf_size;
			}

			int _last = seq_len - end;
			fprintf(stderr, "last: %d \n", _last);
			while(_last > 0) {
				buf_size = 0;
				char buf[512];
				if (_last < 511) {
					buf_size = fread(buf, 1, _last, in_fp);
				}
				else {
					buf_size = fread(buf, 1, 511, in_fp);
				}
				if (buf_size < 1) {
					break;
				}
				_last = _last - buf_size;
				fwrite(buf, 1, buf_size, out_fp);
				//buf[buf_size] = 0x0;
				//fprintf(stderr, "** post: %s\n", buf);

			}

			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}

			int bt;
			if ((bt = byte_count_file(out_file)) > (seq_len - rs)) {
				if (truncate(out_file, seq_len - rs) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < (seq_len - rs)) {
				fprintf(stderr, "ERROR: few written. bt:%d, correct: %ld \n", bt, (seq_len - rs));
				exit(1);
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
                        if (e_code == 1) {
				fclose(out_fp);
				fclose(in_fp);
				fprintf(stderr, "last minimized:%s, %ld\n", out_file, (seq_len - rs));
				return range(program_path, out_file, rs, err_msg);
			}
		}
		rs--;
	}
	fclose(out_fp);
	fclose(in_fp);

	return char_seq_path;
}
