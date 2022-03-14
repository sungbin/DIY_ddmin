#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/ddmin.h"
#include "../include/runner.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int
test_buffer_overflow (char * program_path, char * input_seq_path);

int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n);

long
byte_count_file (char * path);

void
free_paths (char ** path_arr, int len);

char *
complement_seq_files (int n, char ** seq_file_arr, int arr_len);

void
delete_files_except (char ** path_arr, int arr_len, int except_idx);

void
delete_files (char ** path_arr, int arr_len);

// return value: minimum input path
char *
ddmin (char * program_path, char * byte_seq_path) {

	long seq_len;
	int n = 2;
	char * seq_path = strdup(byte_seq_path);
	while ((seq_len = byte_count_file(seq_path)) > 1) {

		char ** partition_path_arr = malloc(sizeof(char *) * 512);
		int splited_n = split_to_file(partition_path_arr, seq_path, n);

		int any_failed = 0;
		for (int i = 0; i < splited_n; i++) {
			char * part_path = partition_path_arr[i];
			if (access(part_path, F_OK ) != 0) {
				continue;
			} 

			if (test_buffer_overflow(program_path, part_path)) {

				delete_files_except(partition_path_arr, splited_n, i);
				free(seq_path);
				seq_path = part_path;
				free_paths(partition_path_arr, splited_n);

				n = 2;
				any_failed = 1;

				break;
			}
		}
		if (any_failed) {
			continue;
		}

		// any_failed = 0
		for (int i = 0; i < splited_n; i++) {

			char * cpart_path = complement_seq_files(i, partition_path_arr, splited_n);
			if (access(cpart_path, F_OK ) != 0) {
				continue;
			} 

			if (test_buffer_overflow(program_path, cpart_path)) {

				delete_files(partition_path_arr, splited_n);
				free(seq_path);
				seq_path = cpart_path;
				free_paths(partition_path_arr, splited_n);

				n = MAX(n-1, 2);
				any_failed = 1;

				break;
			} else {
				remove(cpart_path);
				free(cpart_path);
			}
		}
		if (any_failed) {
			continue;
		}

		free_paths(partition_path_arr, splited_n);
		if (seq_len > n) {
			n = MIN(n*2, seq_len);
		}
		else {
			break;
		}
	}

	return seq_path; //TODO: save sequence as file, and return the path.
}


int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n) {

        FILE * ifp = fopen(char_seq_path, "r");
        int splited_n = 0;
        int allocated_file_n = 512;

	struct stat _stat;
	int fd = fileno(ifp);
        fstat(fd, &_stat);
        long f_size = (long) _stat.st_size;
        int part_size = ceil(f_size / n);

        char buf[512];
        int buf_size = 0;
        int c_idx = 0;

        do {

                buf_size = fread(buf, 1, part_size, ifp);

		if (buf_size <= 0) {
			break;
		}

                if (splited_n > allocated_file_n) {
                        allocated_file_n *= 2;
                        realloc(partition_path_arr, sizeof(char *) * allocated_file_n);
                }

                partition_path_arr[splited_n] = malloc(sizeof(char) * FILENAME_MAX);
                sprintf(partition_path_arr[splited_n], "./%d-%d.part", c_idx, (c_idx+buf_size));

                c_idx += buf_size;
                FILE * out_fp = fopen(partition_path_arr[splited_n], "w");
                splited_n ++;

                fwrite(buf, 1, buf_size, out_fp);
                fclose(out_fp);


        } while(buf_size > 0);

        fclose(ifp);
        return splited_n;
}


char *
complement_seq_files (int n, char ** seq_file_arr, int arr_len) {

	FILE ** fp_arr = malloc(sizeof(FILE *) * arr_len);

	for (int i = 0; i < arr_len; i++) {
		fp_arr[i] = fopen(seq_file_arr[i], "r");
	}


	int min_com_idx = 0;
	while (min_com_idx == n) {
		min_com_idx++;
	}

	int max_com_idx = arr_len - 1;
	while (max_com_idx == n) {
		max_com_idx--;
	}

	int start, end;
	int temp;

	sscanf(seq_file_arr[min_com_idx], "./%d-%d.part", &start, &temp);
	sscanf(seq_file_arr[max_com_idx], "./%d-%d.part", &temp, &end);

	char * compl = malloc(sizeof(char) * FILENAME_MAX);
	sprintf(compl, "./%d-%d.part", start, end);
	
	// merge sequence files	except for index-n
	char buf[512];
	int buf_size;
	FILE * out_fp = fopen(compl, "w");

	for (int i = 0; i < arr_len; i++) {
		if (i == n) {
			continue;
		}

        	do {
			FILE * ifp = fp_arr[i];
                	buf_size = fread(buf, 1, 512, ifp);
                	fwrite(buf, 1, buf_size, out_fp);
	        } while (buf_size > 0);
	}

	
	for (int i = 0; i < arr_len; i++) {
		if (fp_arr[i] != 0x0) {
			fclose(fp_arr[i]);
		}
	}

	fclose(out_fp);

	return compl;
}

long
byte_count_file (char * path) {

	FILE * ifp = fopen(path, "r");

	struct stat _stat;
        int fd = fileno(ifp);
        fstat(fd, &_stat);
        long f_size = (long) _stat.st_size;

	fclose(ifp);
	return f_size;
}

void
free_paths (char ** path_arr, int len) {
	for (int i = 0; i < len; i++) {
		char * path = path_arr[i];
		free(path);
	}
	free(path_arr);
}

void
delete_files_except (char ** path_arr, int arr_len, int except_idx) {

	for (int i = 0; i < arr_len; i++) {
		if (i == except_idx) {
			continue;
		}
		remove(path_arr[i]);
	}
}

void
delete_files (char ** path_arr, int arr_len) {

	for (int i = 0; i < arr_len; i++) {
		remove(path_arr[i]);
	}
}

int
test_buffer_overflow (char * program_path, char * input_seq_path) {
	
	runner_error_code error_code = runner(program_path, input_seq_path, "./program.strout", "./program.strerr");
	remove("./program.strout");
	remove("./program.strerr");
	if (error_code.type == E_TIMEOUT_KILL) {
		return 1;
	} else {
		return 0;
	}
}
