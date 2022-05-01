#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "./include/old_ddmin.h"
#include "./include/runner.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int file_no = 0;
int iter_no = 0;
char * minimized_fname = 0x0;

// return value: minimum input path
char *
ddmin (char * program_path, char * byte_seq_path, char * err_msg) {

	long seq_len;
	int n = 2;
	minimized_fname = strdup(byte_seq_path);
	while ((seq_len = byte_count_file(minimized_fname)) > 1) {

		fprintf(stderr, "len: %ld, n: %d, path: %s, file_no: %d\n", seq_len, n, minimized_fname, file_no);

		char ** partition_path_arr = malloc(sizeof(char *) * n);
		int splited_n = split_to_file(partition_path_arr, minimized_fname, n);

		int any_failed = 0;
		

		for (int i = 0; i < splited_n; i++) {
			char * part_path = partition_path_arr[i];

			if (access(part_path, F_OK ) != 0) {
				fprintf(stderr,"not exists part_path \n");
				exit(1);
			}

			//fprintf(stderr, "test %s with %s \n", program_path, part_path);
			int e_code = test_buffer_overflow(program_path, part_path, err_msg);
			if (e_code == 1) {
				
				minimized_fname = strdup(part_path);
				fprintf(stderr, "last minimized: %s\n", minimized_fname);

				//delete_files(partition_path_arr, splited_n);
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
				fprintf(stderr,"not exists cpart_path \n");
				exit(1);
			} 

			int e_code = test_buffer_overflow(program_path, cpart_path, err_msg);
			if (e_code == 1) {

				free(minimized_fname);
				minimized_fname = strdup(cpart_path);
				fprintf(stderr, "last minimized: %s\n", minimized_fname);

				free(cpart_path);
				//delete_files(partition_path_arr, splited_n);
				free_paths(partition_path_arr, splited_n);

				n = MAX(n-1, 2);
				any_failed = 1;

				break;
			} else {
				//remove(cpart_path);
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
			fprintf(stderr,"len: %ld, n: %d\n", seq_len, n);
			break;
		}
	}

	return minimized_fname; 
}



char *
complement_seq_files (int n, char ** seq_file_arr, int arr_len) {

	FILE ** fp_arr = malloc(sizeof(FILE *) * arr_len);

	for (int i = 0; i < arr_len; i++) {
		fp_arr[i] = fopen(seq_file_arr[i], "rb");
		if (fp_arr[i] == 0x0) {
			perror("complement_seq_files() fp_arr[i]");
			exit(1);
		}
	}

	char * compl = malloc(sizeof(char) * FILENAME_MAX);
	file_no++;
	sprintf(compl, "./%d.part", file_no);
	
	// merge sequence files	except for index-n
	FILE * out_fp = fopen(compl, "wb");

	if (out_fp == 0x0) {
		fprintf(stderr, "ERROR: fopen write %s\n", compl);
		perror("msg");
		exit(1);
	}

	for (int i = 0; i < arr_len; i++) {
		if (i == n) {
			continue;
		}

		int buf_size = 0;

        	do {
			char buf[2048];
			FILE * ifp = fp_arr[i];
                	buf_size = fread(buf, 1, 2048, ifp);
			if (buf_size < 1) {
				break;
			}
                	fwrite(buf, 1, buf_size, out_fp);
			fprintf(stderr, "cpart %d:%d %s\n", file_no,buf_size, buf);
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
test_buffer_overflow (char * program_path, char * input_seq_path, char * err_msg) {
	
	FILE * t_fp = fopen("./program.strerr", "wb");
	fclose(t_fp);
	runner_error_code error_code = runner(program_path, input_seq_path, "./program.strout", "./program.strerr");

        FILE * fp = fopen("./program.strerr", "rb");
	if (fp == 0x0) {
		perror("test_buffer_overflow()");
		exit(1);
	}

        char line[2048];
        while (fgets(line, 2048, fp) != 0x0) {
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

int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n) {

        FILE * ifp = fopen(char_seq_path, "rb");
	if (ifp == 0x0) {
		perror("split_to_file() ifp");
		exit(1);
	}
        int splited_n = 0;

	long f_size = byte_count_file(char_seq_path);

        char buf[2048];
        int buf_size = 0;
        int c_idx = 0;

	do {
		

		int part_size = ceilf((float)f_size / (float)n);

		partition_path_arr[splited_n] = malloc(sizeof(char) * FILENAME_MAX);
		file_no++;
                sprintf(partition_path_arr[splited_n], "./%d.part", file_no);

                c_idx += buf_size;
                FILE * out_fp = fopen(partition_path_arr[splited_n], "wb");
		if (out_fp == 0x0) {
			perror("split_to_file() out_fp");
			exit(1);
		}
                splited_n ++;

		f_size -= part_size;
		while (part_size > 0) {

			if (part_size < 2048) {
				buf_size = fread(buf, 1, part_size, ifp);
			}
			else {
				buf_size = fread(buf, 1, 2048, ifp);
			}

			fprintf(stderr, "part %d:%d %s\n", file_no,buf_size, buf);	
			fwrite(buf, 1, buf_size, out_fp);
			part_size -= buf_size;
		}
		
		fclose(out_fp);
		n = n-1;

	} while (n > 0);

        fclose(ifp);

        return splited_n;
}
