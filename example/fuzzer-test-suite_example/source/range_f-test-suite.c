#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "./include/range.h"
#include "./include/ddmin.h"
#include "./include/runner.h"


#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


extern int file_no;
extern int iter_no;

char *
range (char * program_path, char * char_seq_path, char * err_msg) {
	long seq_len;
	seq_len = byte_count_file(char_seq_path);

	fprintf(stderr, "last minimized: %s, %ld\n", char_seq_path, seq_len);
	
	for (int rs = seq_len-1; rs >=1; rs--) {
		fprintf(stderr,"rs:%d\n", rs);
		for (int begin = 0; begin <= seq_len - rs; begin++) {
			int end = begin + rs;

			char * f_name = calloc(sizeof(char), 512);
			sprintf(f_name, "./%d.part", file_no++);
			FILE * out_fp = fopen(f_name, "wb");
			FILE * in_fp = fopen(char_seq_path, "rb");

			
			int buf_size = 0;
			int _begin = begin;
			//fprintf(stderr,"_begin: %d\n", _begin);
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
				//fprintf(stderr,"buf_size: %d\n", buf_size);
			}

			int _last = seq_len - end;
			//fprintf(stderr,"_last: %d\n", _last);
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

			fclose(in_fp);
			fclose(out_fp);

			int e_code = test_buffer_overflow(program_path, f_name, err_msg);
			//fprintf(stderr,"test %s, %s, %s, %d\n", program_path, f_name, err_msg, e_code);
                        if (e_code == 1) {
				return range(program_path, f_name, err_msg);
			}

			free(f_name);
		}
	}

	return char_seq_path;
}
