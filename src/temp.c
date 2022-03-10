#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n);

int
main (int argc, char * argv[]) {

	char * input_path = "../crash.json";

	int n = 20;

	char ** partition_path_arr = malloc(sizeof(char *) * 512);
	int splited_n = split_to_file(partition_path_arr, input_path, n);
	
	for (int i = 0; i < splited_n; i++) {

		printf("%s \n", partition_path_arr[i]);
		free(partition_path_arr[i]);
	}

	free(partition_path_arr);
	return 0;
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
