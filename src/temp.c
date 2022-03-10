#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

char **
split_to_file (char * char_seq_path, int n);

main (int argc, char * argv[]) {

	char * path = "../crash.json"

	int start = 5;
	int end = 9;

	char ** splited_file_list = split_to_file(path, n);
	
	

	return 0;
}


char **
split_to_file (char * char_seq_path, int n) {

	FILE * ifp = fopen(char_seq_path, "r");

	if (ifp == NULL) {
		fprintf(stderr, "ERROR: open the character sequence file\n");
		exit(1);
	}

	int allocated_file_n = 512;	
	char ** first_partition_path = malloc(sizeof(char *) * allocated_file_n);
	int first_part_size = 0;

	int buf_size = 0;
        int seq_len = 0;

	do {

		char * buf = malloc(sizeof(char) * n);
		buf_size = fread(buf, 1, part_size, ifp); //TODO: file-write

		char * out_f_name = malloc(sizeof(char) * FILENAME_MAX);
		sprintf(out_f_name, "./%d-%d.txt", seq_len, seq_len+buf_size);
		FILE * out_fp = fopen(out_f_name, "w");
		
		fwrite(buf, 1, buf_size, out_fp);
		fclose(out_fp);

		if (first_part_size >= allocated_file_n) {
			allocated_file_n *= 2;
			realloc(first_partition_path, sizeof(char *) * allocated_file_n);
		}
		first_partitions[first_part_size] = strdup(out_f_name);

		seq_len += buf_size;
		first_part_size++;

		free(out_f_name);

	} while (buf_size  == n);
	fclose(ifp);

        int each_size = ceil(seq_len / n);

	FILE ** first_part_files = malloc(sizeof(FILE *) * first_part_size);
	for (int i = 0; i < first_part_size; i++) {
		first_part_files[i] = fopen(first_partition_path[i], "r");
	}

	int seq_idx = 0;
	int second_p_size

	for (int p_idx = 0; p_idx < first_part_size; ) {

		char * f_name[FILENAME_MAX];
		sprintf(f_name, "./%d-.txt", seq_idx);
		FILE * sub_f = fopen(f_name, "w");

		char * buf = malloc(sizeof(char) * each_size);
		int buf_size = fread(buf, 1, each_size, first_part_files[p_idx]); 

		if (buf_size > 0) {

			if (buf_size < each_size) {
				fclose(first_part_files[p_idx]);
				free(first_part_files[p_idx]);
			}
			
			char out_f_name[FILENAME_MAX];
			sprintf(out_f_name, "./%d-.txt", seq_idx);
			FILE * out_fp = fopen(out_f_name, "w");
			fwrite(buf, 1, buf_size, out_fp);

			seq_idx += buf_size;
			int lack_partition = each_size - buf_size;
			
			p_idx++;
			while (lack_partition > 0) {

				buf_size = fread(buf, 1, lack_partition, first_part_files[p_idx]);
				lack_partition = lack_partition - buf_size;
				
				fwrite(buf, 1, lack_partition, out_fp);
				seq_idx += buf_size;

				if (lack_partition >= 0) {
					fclose(first_part_files[p_idx]);
					p_idx++;
				}
			}

			fclose(out_fp);
			free(out_fp);
			
		}
		else if (buf_size > each_size) {

			char out_f_name[FILENAME_MAX];
			sprintf(out_f_name, "./%d-.txt", seq_idx);
			FILE * out_fp = fopen(out_f_name, "w");
			fwrite(buf, 1, buf_size, out_fp);

			fclose(out_fp);
			free(out_fp);

			seq_idx += buf_size;
			int over_partition = buf_size - each_size;

			sprintf(out_f_name, "./%d-.txt", seq_idx);
			out_fp = fopen(out_f_name, "w");

			while (over_partition > 0) {

				buf_size = fread(buf, 1, over_partition, first_part_files[p_idx]);
				over_partition = over_partition - buf_size;

				fwrite(buf, 1, buf_size, out_fp);
				
				if (over_partition >= 0) {

					

				}
			}

		}
		else {
			/* un-reached area */
			exit(1);
		}
		

		fwrite(first_partitions[fp_idx], 1, strlen(first_partitions[fp_idx], ))
		first_partitions[fp_idx]

		free(buf);
		fclose(sub_f);
		free(f_name)
		free(sub_f);

	}
















		for (int i = 0; i < first_part_size; i ++) {

			char * buf = malloc(sizeof(char) * each_size);
			int size = strlen
			strncpy(buf, first_partitions[i], n);

			

		}

	}





	free(ifp);

        return part_n;

}
