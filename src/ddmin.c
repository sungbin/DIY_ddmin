#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "../include/ddmin.h"
#include "../include/runner.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int file_no = 0;
int fail_no = 0;
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
		fprintf(stderr, "len: %ld, n: %d, path: %s, file_no: %d\n", f_size, n, minimized_fname, fail_no);

		FILE * in_fp = fopen(minimized_fname, "rb");
		if (in_fp == 0x0) {
			perror("in_fp");
			fprintf(stderr, "in_fp: minimized_fname:%s\n",minimized_fname);
			exit(1);
		}

		int any_failed = 0;
		int in_fd = fileno(in_fp);

		int _f_size = f_size;
		int _n = n;

		char out_file[256];
		sprintf(out_file, "./%d.part", file_no++);
		FILE * out_fp = fopen(out_file, "wb");
		int out_fd = fileno(out_fp);
		do {

			lseek(out_fd, 0, SEEK_SET);

			int part_size = ceilf((float)_f_size / (float)_n);
			//fprintf(stderr, "subset part_size: %d \n", part_size);
			
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
			}
			_f_size -= part_size;

			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}
			
			long bt;
			if ((bt = byte_count_file(out_file)) > part_size) {
				if (truncate(out_file, part_size) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < part_size) {
				fprintf(stderr, "few elements written: bt=%ld, part_size=%d", bt, part_size);
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (e_code == 1) {
				fprintf(stderr, "last minimized: %s\n", minimized_fname);
				n = 2;
				any_failed = 1;
				break;
			}
			_n --;

		} while (_n > 0);

		
		if (any_failed) {
			fclose(in_fp);
			fclose(out_fp);
			strcpy(minimized_fname, out_file);
			continue;
		}

		int c_start = 0;
		any_failed = 0;
		_n = n;
		do {

			int part_size = ceilf((float)(f_size - c_start) / (float)_n);
			int c_end = c_start + part_size;

			lseek(in_fd, 0, SEEK_SET);
			lseek(out_fd, 0, SEEK_SET);

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
				fwrite(buf, 1, buf_size, out_fp);

			}

			{
				char buf[2048];
                                int buf_size;
				buf_size = fread(buf, 1, part_size, in_fp);
				if (buf_size != part_size) {
					fprintf(stderr, "ERROR: complement (space)\n");
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
				fwrite(buf, 1, buf_size, out_fp);
			}
			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}

			if (byte_count_file(out_file) != (f_size - part_size)) {
				if (truncate(out_file, (f_size - part_size)) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (e_code == 1) {
				fprintf(stderr, "last minimized: %s\n", minimized_fname);
				n = MAX(n-1, 2);
				any_failed = 1;
				break;
			}
			c_start = c_end;
			_n --;

		} while (c_start < f_size);

		if (any_failed) {
			fclose(in_fp);
			fclose(out_fp);
			strcpy(minimized_fname, out_file);
			continue;
		}
		
		fclose(in_fp);
		fclose(out_fp);

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
	
	char stdout_path[256];
	char stderr_path[256];

	sprintf(stdout_path, "./%s.stdout", input_seq_path);
	sprintf(stderr_path, "./%s.stderr", input_seq_path);

	FILE * t_fp = fopen(stderr_path, "wb");
	if (t_fp == 0x0) {
                fprintf(stderr, "no: %s \n", stderr_path);
                perror("test_buffer_overflow()");
                exit(1);
        }
	fclose(t_fp);
	runner_error_code error_code = runner(program_path, input_seq_path, stdout_path, stderr_path);

        FILE * fp = fopen(stderr_path, "rb");
	if (fp == 0x0) {
		fprintf(stderr, "no: %s \n", stderr_path);
		perror("test_buffer_overflow()");
		exit(1);
	}

        char line[512];
        while (fgets(line, 512, fp) != 0x0) {
                if (strstr(line, err_msg) != 0x0) {
                        fclose(fp);
                        remove(stdout_path);
                        remove(stderr_path);
                        return 1;
                }
        }

        fclose(fp);

        remove(stdout_path);
        remove(stderr_path);

        return 0;

}

int
test_buffer_overflow_thread (char * program_path, char * input_seq_path, char * err_msg, int out_idx) {
	
	char stdout_path[256];
	char stderr_path[256];

	sprintf(stdout_path, "./dummy%d.stdout", out_idx);
	sprintf(stderr_path, "./dummy%d.stderr", out_idx);

	FILE * t_fp = fopen(stderr_path, "wb");
	if (t_fp == 0x0) {
                fprintf(stderr, "no: %s \n", stderr_path);
                perror("test_buffer_overflow()");
                exit(1);
        }
	fclose(t_fp);
	runner_error_code error_code = runner(program_path, input_seq_path, stdout_path, stderr_path);

        FILE * fp = fopen(stderr_path, "rb");
	if (fp == 0x0) {
		fprintf(stderr, "no: %s \n", stderr_path);
		perror("test_buffer_overflow()");
		exit(1);
	}

        char line[512];
        while (fgets(line, 512, fp) != 0x0) {
                if (strstr(line, err_msg) != 0x0) {
                        fclose(fp);
                        remove(stdout_path);
                        remove(stderr_path);
                        return 1;
                }
        }

        fclose(fp);

        remove(stdout_path);
        remove(stderr_path);

        return 0;

}

char *
ddmin_dir (char * program_path, char * input_dir, char * err_msg, char * exe_dir) {

	if (access(program_path, F_OK)) {
		fprintf(stderr, "no program path: %s\n", program_path);
		exit(1);
	}

	if (access(input_dir, F_OK)) {
		fprintf(stderr, "no input path: %s\n", input_dir);
		exit(1);
	}

	long f_size;
	int n = 2;
	minimized_fname = malloc(sizeof(char)*256);
	sprintf(minimized_fname, "../%s/test_input", exe_dir); // inputs/test_input
	
	while ((f_size = byte_count_file(minimized_fname)) > 1) {
		fprintf(stderr, "len: %ld, n: %d, path: %s, file_no: %d\n", f_size, n, minimized_fname, fail_no);

		FILE * in_fp = fopen(minimized_fname, "rb");
		if (in_fp == 0x0) {
			perror("in_fp");
			fprintf(stderr, "in_fp: minimized_fname:%s\n",minimized_fname);
			exit(1);
		}

		int any_failed = 0;
		int in_fd = fileno(in_fp);

		int _f_size = f_size;
		int _n = n;

		char out_file[256];
		sprintf(out_file, "../%s/%d.part", exe_dir, file_no++);
		FILE * out_fp = fopen(out_file, "wb");
		int out_fd = fileno(out_fp);
		do {

			lseek(out_fd, 0, SEEK_SET);

			int part_size = ceilf((float)_f_size / (float)_n);
			//fprintf(stderr, "subset part_size: %d \n", part_size);

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
			}
			_f_size -= part_size;

			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}

			long bt;
			if ((bt = byte_count_file(out_file)) > part_size) {
				if (truncate(out_file, part_size) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < part_size) {
				fprintf(stderr, "few elements written: bt=%ld, part_size=%d", bt, part_size);
			}

			char path[256];
			sprintf(path, "%s/test_input", input_dir);
			if (remove(path) == -1) {
				perror("ERROR: fail to remove test_input");
				exit(1);
			}

			copy(out_file, path);

			int e_code = test_buffer_overflow(program_path, input_dir, err_msg);
			if (e_code == 1) {
				fprintf(stderr, "last minimized: %s\n", minimized_fname);
				n = 2;
				any_failed = 1;
				break;
			}
			_n --;

		} while (_n > 0);


		if (any_failed) {
			fclose(in_fp);
			fclose(out_fp);
			strcpy(minimized_fname, out_file);
			continue;
		}

		int c_start = 0;
		any_failed = 0;
		_n = n;
		do {

			int part_size = ceilf((float)(f_size - c_start) / (float)_n);
			int c_end = c_start + part_size;

			lseek(in_fd, 0, SEEK_SET);
			lseek(out_fd, 0, SEEK_SET);

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
				fwrite(buf, 1, buf_size, out_fp);

			}

			{
				char buf[2048];
                                int buf_size;
				buf_size = fread(buf, 1, part_size, in_fp);
				if (buf_size != part_size) {
					fprintf(stderr, "ERROR: complement (space)\n");
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
				fwrite(buf, 1, buf_size, out_fp);
			}
			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}

			if (byte_count_file(out_file) != (f_size - part_size)) {
				if (truncate(out_file, (f_size - part_size)) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (e_code == 1) {
				fprintf(stderr, "last minimized: %s\n", minimized_fname);
				n = MAX(n-1, 2);
				any_failed = 1;
				break;
			}
			c_start = c_end;
			_n --;

		} while (c_start < f_size);

		if (any_failed) {
			fclose(in_fp);
			fclose(out_fp);
			strcpy(minimized_fname, out_file);
			continue;
		}

		fclose(in_fp);
		fclose(out_fp);

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
int
copy (const char *src, const char *dst) {
	int src_fd, dst_fd;
	struct stat srcstat;
	char in;
	int n;
	char buf[1024] = {0x00,};

	if ((src_fd = open(src, O_RDONLY)) <0) {
    		fprintf(stdout, "Can not open src file : %s\n", src);
		exit(1);
	}

	fstat(src_fd, &srcstat);

	if (access(dst, F_OK) == 0){
		fprintf(stderr, "already exist file in copy()\n");
		exit(1);
	}

	if ((dst_fd = open(dst, O_WRONLY|O_CREAT, srcstat.st_mode)) < 0){
        	fprintf(stdout, "Can not open dst file : %s\n", dst);
        	close(src_fd);
		exit(1);
	}
	while((n = read(src_fd, buf, 1024)) > 0) {
        	write(dst_fd, buf, n);
	}

	fchown(dst_fd, srcstat.st_uid, srcstat.st_gid);
	close(src_fd);
	close(dst_fd);

	return 0;
}
