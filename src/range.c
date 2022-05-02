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

char *
range (char * program_path, char * char_seq_path, char * err_msg) {
	long seq_len;
	seq_len = byte_count_file(char_seq_path);

	fprintf(stderr, "last minimized: %s, %ld\n", char_seq_path, seq_len);

	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "./%d.part", file_no++);
	FILE * in_fp = fopen(char_seq_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");

	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	for (int rs = seq_len-1; rs >=1; rs--) {
		fprintf(stderr,"rs:%d\n", rs);
		for (int begin = 0; begin <= seq_len - rs; begin++) {

			lseek(in_fd, 0, SEEK_SET);
			lseek(out_fd, 0, SEEK_SET);

			int end = begin + rs;
			int buf_size = 0;
			int _begin = begin;
			while (_begin > 0) {
				char buf[512];
				if (_begin < 511) {
					buf_size = fread(buf, 1, _begin, in_fp);
				}
				else {
					buf_size = fread(buf, 1, 511, in_fp);
				}
				_begin = _begin - buf_size;
				fwrite(buf, 1, buf_size, out_fp);

			}

			int _rs = rs;
			while (_rs > 0) {
				char buf[512];
				buf_size = fread(buf, 1, _rs, in_fp);
				_rs = _rs - buf_size;
			}

			int _last = seq_len - end;
			while(_last > 0) {
				buf_size = 0;
				char buf[512];
				if (_last < 511) {
					buf_size = fread(buf, 1, _last, in_fp);
				}
				else {
					buf_size = fread(buf, 1, 511, in_fp);
				}
				_last = _last - buf_size;
				fwrite(buf, 1, buf_size, out_fp);

			}

			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}

			int bt;
			if ((bt = byte_count_file(out_file)) != (seq_len - rs)) {
				if (truncate(out_file, seq_len - rs) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
                        if (e_code == 1) {
				fclose(out_fp);
				fclose(in_fp);
				return range(program_path, out_file, err_msg);
			}
		}
	}
	fclose(out_fp);
	fclose(in_fp);

	return char_seq_path;
}
