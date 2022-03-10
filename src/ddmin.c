#include <string.h>
#include <stdlib.h>
// #include <stdio.h>
#include <math.h>

#include "../include/ddmin.h"
#include "../include/runner.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int
split (partition * partitions, char * char_seq, int n);

int
test_buffer_overflow(char * program_path, char * input_seq_path);

int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n);

// return value: minimum input path
char *
ddmin (char * program_path, char * char_seq_path) {

	int seq_len, n = 2;
	char * sequence = path_to_str(char_seq_path);

	while ((seq_len = strlen(sequnce)) > 1) {

		char ** partition_path_arr = malloc(sizeof(char *) * 512);
		int splited_n = split_to_file(partition_path_arr, input_path, n);

		int any_failed = 0;
		for (int i = 0; i < splited_n; i++) {
			
			char * part_path = partition_path_arr[i];

			if (! test_buffer_overflow(program_path, part_path)) {
				sequance = path_to_str(part_path);
				n = 2;
				any_failed = 1;

				break;
			}
		}
		if (any_failed) {
			//TODO: free(partition_path) for each partition_path_arr
			continue;
		}

		// any_failed = 0
		for (int i = 0; i < partition_n; i++) {

			char * cpart_path = complement_seq(partition_path_arr, splited_n, i);

			if (! test_buffer_overflow(program_path, cpart_path)) {
				sequance = path_to_str(cpart_path);
				n = MAX(n-1, 2);
				any_failed = 1;

				break;
			}
		}
		if (any_failed) {
			//TODO: free(partition_path) for each partition_path_arr
			continue;
		}

		//TODO: free(partition_path) for each partition_path_arr
		if (seq_len < n) {
			n = MIN(n*2, seq_len);
		}
		else {
			break;
		}
	}

	//TODO: free sequence

	return sequnce; //TODO: save sequence as file, and return the path.
}


int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n) {

	FILE * ifp = fopen(char_seq_path, "r");
	int splited_n = 0;
	int allocated_file_n = 512;	

	struct stat _stat;
	fstat(ifp, &_stat);
	long f_size = (long) _stat.st_size;
	int part_size = ceil(f_size / n);

	char buf[512];
	int buf_size = 0;
	int c_idx = 0;

	do {

		buf_size = fread(buf, 1, part_size, ifp);

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
complement_seq(partition * partitions, int part_index, int partition_n) {

	int part_len = strlen(partitions[part_index].seq);
	char * compl = malloc(sizeof(char) * (partition_n - part_len));
	
	for (int i = 0; i < partition_n; i++) {

		if (i == part_index) {
			continue;
		}

		strncat(compl, partitions[i].seq, (partitions[i].end - partitions[i].start));
	}	
	
	return compl;
}
