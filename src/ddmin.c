#include <unistd.h>
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

int file_no = 1;
int iter_no = 0;
char * minimized_fname = 0x0;

// return value: minimum input path
char *
ddmin (char * program_path, char * byte_seq_path) {

	long seq_len;
	int n = 2;
	minimized_fname = strdup(byte_seq_path);
	while ((seq_len = byte_count_file(minimized_fname)) > 1) {

		fprintf(stderr, "len: %ld, n: %d, path: %s, iteration: %d\n", seq_len, n, minimized_fname, ++iter_no);

		char ** partition_path_arr = malloc(sizeof(char *) * 512);
		int splited_n = split_to_file(partition_path_arr, minimized_fname, n);

		int any_failed = 0;
		for (int i = 0; i < splited_n; i++) {
			char * part_path = partition_path_arr[i];

			if (access(part_path, F_OK ) != 0) {
				continue;
			}

			int e_code = test_buffer_overflow(program_path, part_path);
			if (e_code == 1) {
				
				minimized_fname = strdup(part_path);
				fprintf(stderr, "last minimized: %s\n", minimized_fname);

				delete_files(partition_path_arr, splited_n);
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

			int e_code = test_buffer_overflow(program_path, cpart_path);
			if (e_code == 1) {

				free(minimized_fname);
				minimized_fname = strdup(cpart_path);
				fprintf(stderr, "last minimized: %s\n", minimized_fname);

				free(cpart_path);
				delete_files(partition_path_arr, splited_n);
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

	return minimized_fname; 
}



char *
complement_seq_files (int n, char ** seq_file_arr, int arr_len) {

	FILE ** fp_arr = malloc(sizeof(FILE *) * arr_len);

	for (int i = 0; i < arr_len; i++) {
		fp_arr[i] = fopen(seq_file_arr[i], "r");
	}

	char * compl = malloc(sizeof(char) * FILENAME_MAX);
	sprintf(compl, "./%d.part", file_no);
	file_no++;
	
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

	struct stat _stat;
        if (stat(path, &_stat) == -1) {
		fprintf(stderr, "stat errno: %d (%s)\n", errno, path);
		exit(1);
	}
        long f_size = (long) _stat.st_size;

	//fprintf(stderr, "file: %s, %ld\n", path, f_size);

	return f_size;
}


void
free_paths (char ** path_arr, int len) {
	for (int i = 0; i < len; i++) {
		char * path = path_arr[i];
		if (path != 0x0) {
			free(path);
		}
	}
	free(path_arr);
}


void
delete_files (char ** path_arr, int arr_len) {

	for (int i = 0; i < arr_len; i++) {
		if (minimized_fname != 0x0 && strcmp(minimized_fname, path_arr[i]) == 0) {
			continue;
		}
		remove(path_arr[i]);
	}
}

int
test_buffer_overflow (char * program_path, char * input_seq_path) {
	
	char error_message[256] = "in dump example/jsondump.c:44";

	runner_error_code error_code = runner(program_path, input_seq_path, "./program.strout", "./program.strerr");

	if (error_code.exit_code != 1) {
		remove("./program.strout");
		remove("./program.strerr");
		return 0;
	}

	FILE * fp = fopen("./program.strerr", "rb");

	char line[512];
	while (fgets(line, 512, fp) != 0x0) {
		if (strstr(line, error_message) != 0x0) {
			fclose(fp);
			remove("./program.strout");
			remove("./program.strerr");
			//fprintf(stderr, "find!\n");
			return 1;
		}
	}

	fclose(fp);
	remove("./program.strout");
	remove("./program.strerr");

	return 0;
}

int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n) {

        FILE * ifp = fopen(char_seq_path, "rb");
        int splited_n = 0;

	long f_size = byte_count_file(char_seq_path);

        char buf[512];
        int buf_size = 0;
        int c_idx = 0;

        do {

		//fprintf(stderr,"\t @ @\n");
        	int part_size = ceilf((float)f_size / (float)n);
		//fprintf(stderr, "part_size: %d (%ld/%d)\n", part_size, f_size, n);

                buf_size = fread(buf, 1, part_size, ifp);

		if (buf_size <= 0) {
			break;
		}

                partition_path_arr[splited_n] = malloc(sizeof(char) * FILENAME_MAX);
                sprintf(partition_path_arr[splited_n], "./%d.part", file_no);
		file_no++;

                c_idx += buf_size;
                FILE * out_fp = fopen(partition_path_arr[splited_n], "w");
                splited_n ++;

                fwrite(buf, 1, buf_size, out_fp);
                fclose(out_fp);

		n = n-1;
		f_size -= part_size;

        } while(n > 0);

        fclose(ifp);
        return splited_n;
}
