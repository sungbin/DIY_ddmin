#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "../include/range.h"
#include "../include/ddmin.h"
#include "../include/runner.h"


#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


extern int file_no;
extern int iter_no;


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
range_increasing (char * program_path, char * input_path, int rs, char * err_msg) {
	
	long input_size = byte_count_file(input_path);
	fprintf(stderr, "(range_increasing) last minimized: %s, %ld\n", input_path, input_size);
	
	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "./%d.part", ++iter_no);

	FILE * in_fp = fopen(input_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");
	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	while (rs < input_size) {
		fprintf(stderr,"rs:%d\n", rs);
		for (int begin = 0; begin <= (input_size - rs); begin++) {

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

			
			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (file_no % 1000 == 0) {
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				fprintf(stderr, "Test count:%d -  %d-%d-%d %d:%d:%d\n", file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			}
			file_no++;
                        if (e_code == 1) {
				fclose(out_fp);
				fclose(in_fp);
				free(input_path);
				return range_increasing(program_path, out_file, rs, err_msg);
			}

		}
		rs++;
	}

	return input_path;
}

char *
range (char * program_path, char * input_path, int rs, char * err_msg) {

	long input_size = byte_count_file(input_path);
	
	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "./%d.part", ++iter_no);

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

			

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
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
				free(input_path);
				return range(program_path, out_file, rs, err_msg);
			}
		}
		rs--;
	}
	fclose(out_fp);
	fclose(in_fp);

	return input_path;
}
