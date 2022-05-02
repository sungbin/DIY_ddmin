#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "./include/ddmin.h"
#include "./include/runner.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int file_no = 1;
int iter_no = 0;
char * minimized_fname = 0x0;

// return value: minimum input path
char *
ddmin (char * program_path, char * byte_seq_path, char * err_msg) {

	if (access(program_path, F_OK)) {
		fprintf(stderr, "no program path: %s\n", program_path);
		exit(1);
	}

	if (access(byte_seq_path, F_OK)) {
		fprintf(stderr, "no input path: %s\n", byte_seq_path);
		exit(1);
	}

	long f_size;
	int n = 2;
	minimized_fname = malloc(sizeof(char)*256);
	strcpy(minimized_fname, byte_seq_path);

	while ((f_size = byte_count_file(minimized_fname)) > 1) {
		fprintf(stderr, "len: %ld, n: %d, path: %s, file_no: %d\n", f_size, n, minimized_fname, iter_no);

		FILE * in_fp = fopen(minimized_fname, "rb");
		if (in_fp == 0x0) {
			perror("in_fp");
			fprintf(stderr, "in_fp: minimized_fname:%s\n",minimized_fname);
			exit(1);
		}

		int any_failed = 0;
		char out_file[256];
		int in_fd = fileno(in_fp);
		int _f_size = f_size;
		int _n = n;
		do {
			sprintf(out_file, "./%d.part", file_no++);
			FILE * out_fp = fopen(out_file, "wb");
			int part_size = ceilf((float)_f_size / (float)_n);
			int _part_size = part_size;
			while (_part_size > 0) {
				
				char buf[2048];
				int buf_size;
				if (_part_size < 2048) {
			       		buf_size = fread(buf, 1, _part_size, in_fp);
				}
				else {
					buf_size = fread(buf, 1, 2048, in_fp);
				}
				_part_size -= buf_size;

				buf_size = fwrite(buf, 1, buf_size, out_fp);
				fprintf(stderr, "part %d:%d %s\n", file_no, buf_size, buf);
			}
			_f_size -= part_size;

			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}
			fclose(out_fp);
			
			long bt;
			if ((bt = byte_count_file(out_file)) != part_size) {
				fprintf(stderr, "ERROR: reduce_to_subset\n");
				fprintf(stderr, "outfile: %ld,  p_size: %d\n", bt, part_size);
				exit(1);
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (e_code == 1) {
				strcpy(minimized_fname, out_file);
				fprintf(stderr, "last minimized: %s\n", minimized_fname);

				n = 2;
				any_failed = 1;
				break;
			}
			_n --;

		} while (_n > 0);

		
		if (any_failed) {
			fclose(in_fp);
			strcpy(minimized_fname, out_file);
			continue;
		}

		int c_start = 0;
		any_failed = 0;
		_n = n;
		do {
			
			sprintf(out_file, "./%d.part", file_no++);
			FILE * out_fp = fopen(out_file, "wb");

			int part_size = ceilf((float)(f_size - c_start) / (float)_n);
			int c_end = c_start + part_size;

			lseek(in_fd, 0, SEEK_SET);
			while (c_start > 0) {

				char buf[2048];
				int buf_size;
				if (c_start < 2048) {
			       		buf_size = fread(buf, 1, c_start, in_fp);
				}
				else {
					buf_size = fread(buf, 1, 2048, in_fp);
				}
				c_start -= buf_size;
				fprintf(stderr, "cpart %d:%d %s\n", file_no, buf_size, buf);
				fwrite(buf, 1, buf_size, out_fp);

			}

			{
				char buf[2048];
				int bf = fread(buf, 1, part_size, in_fp);
				if (bf != part_size) {
					fprintf(stderr, "complement (space) eeror!\n");
					exit(1);
				}
			}

			long _f_size = f_size - c_end;

			while (_f_size > 0) {

				char buf[2048];
				int buf_size;
				if (_f_size < 2048) {
			       		buf_size = fread(buf, 1, _f_size, in_fp);
				}
				else {
					buf_size = fread(buf, 1, 2048, in_fp);
				}
				_f_size -= buf_size;
				fprintf(stderr, "cpart %d:%d %s\n", file_no, buf_size, buf);
				fwrite(buf, 1, buf_size, out_fp);
			}
			fclose(out_fp);
			if (byte_count_file(out_file) != (f_size - part_size)) {
				fprintf(stderr, "ERROR: reduce_to_complement\n");
				fprintf(stderr, "outfile: %ld,  p_size: %ld\n", byte_count_file(out_file), (f_size - part_size));
				exit(1);
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (e_code == 1) {
				strcpy(minimized_fname, out_file);
				fprintf(stderr, "last minimized: %s\n", minimized_fname);

				n = 2;
				any_failed = 1;
				break;
			}
			c_start = c_end;
			_n --;

		} while (c_start < f_size);

		if (any_failed) {
			fclose(in_fp);
			strcpy(minimized_fname, out_file);
			continue;
		}
		
		fclose(in_fp);
		if (f_size > n) {
			n = MIN(n*2, f_size);
		}
		else {
			fprintf(stderr,"len: %ld, n: %d\n", f_size, n);
			break;
		}
	}

	return minimized_fname; 
}

long
byte_count_file (char * path) {

	struct stat _stat;
        if (stat(path, &_stat) == -1) {
		fprintf(stderr, "stat errno: %d (%s)\n", errno, path);
		exit(1);
	}
        long f_size = (long) _stat.st_size;

	//fprintf(stderr, "file: %s, %ld\n", path, f_size);

	return f_size;
}

int
test_buffer_overflow (char * program_path, char * input_seq_path, char * err_msg) {
	
	FILE * t_fp = fopen("./program.strerr", "wb");
	fclose(t_fp);
	runner_error_code error_code = runner(program_path, input_seq_path, "./program.strout", "./program.strerr");

        FILE * fp = fopen("./program.strerr", "rb");
	if (fp == 0x0) {
		perror("test_buffer_overflow()");
		exit(1);
	}

        char line[512];
        while (fgets(line, 512, fp) != 0x0) {
                if (strstr(line, err_msg) != 0x0) {
                        fclose(fp);
                        remove("./program.strout");
                        remove("./program.strerr");
                        return 1;
                }
        }

        fclose(fp);
        remove("./program.strout");
        remove("./program.strerr");

        return 0;

}
