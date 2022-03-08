#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../include/ddmin.h"
#include "../include/runner.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int
split (partition * partitions, char * char_seq, int n);

char *
complement_seq(partition * partitions, int part_index, int partition_n);

int
test_buffer_overflow(char * program_path, char * input_seq);

char *
ddmin (char * program_path, char * char_seq) {

	int seq_len, n = 2;
	char * sequnce = strdup(char_seq);

	while ((seq_len = strlen(sequnce)) > 1) {

		printf("seq: %s\n", char_seq);

		partition * partitions;
		partitions = malloc(sizeof(partition) * (seq_len/n + 1));
		int partition_n = split(partitions, sequnce, n);

		int any_failed = 0;
		for (int i = 0; i < partition_n; i++) {
			partition p = partitions[i];

			if (! test_buffer_overflow(program_path, p.seq)) {
				sprintf(sequnce, "%s", p.seq);
				n = 2;
				any_failed = 1;
				break;
			}
		}
		if (any_failed) {
			free(partitions);
			continue;
		}

		// any_failed = 0
		for (int i = 0; i < partition_n; i++) {
			char * cp_seq = complement_seq(partitions, i, partition_n);

			if (! test_buffer_overflow(program_path, cp_seq)) {
				sprintf(sequnce, "%s", cp_seq);
				n = MAX(n-1, 2);
				any_failed = 1;
				break;
			}
		}
		if (any_failed) {
			free(partitions);
			continue;
		}

		if (seq_len < n) {
			free(partitions);
			n = MIN(n*2, seq_len);
		}
		else {
			free(partitions);
			break;
		}
	}
	return sequnce;
}


int
split (partition * partitions, char * char_seq, int n) {

	int part_idx = 0;
	int seq_len = strlen(char_seq);	
	int part_size = ceil(seq_len / n);
	int current_idx = 0;
	int part_n = 0;

	for (int i = 0; i < n; i++) {
		int start_part = current_idx + i * part_size;
		int end_part = MIN(current_idx + (i+1)*part_size, seq_len);

		char * _seq = malloc(sizeof(char) * end_part-start_part);
		strncpy(_seq, (char_seq+start_part), end_part-start_part);
		partition p = { .start=start_part, .end=end_part, .seq = _seq };
		partitions[part_idx] = p;
		
		current_idx = end_part;
		part_n++;
	}
	
	return part_n;
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

int
test_buffer_overflow(char * program_path, char * input_seq) {

	// 0. write input as fie
	char * input_path = "./input.test";
	char * output_path = "./output.text";
	char * output_err_path = "./output_err.text";
	FILE * input_fw = fopen(input_path, "w");
	fwrite(input_seq, 1, strlen(input_seq), input_fw);
	fclose(input_fw);

	// 1. run and print error as file

	//runner (char * target_path, char * input_path, char * output_path, char * output_err_path)	
	runner(program_path, input_path, output_path, output_err_path);
	
	// 2. if size(error_file) > 0, assume buffer overflow

	FILE * output_err_fr = fopen(output_err_path, "r");
	unsigned char in_buf[512];
	int buf_size = fread(in_buf, 1, 512, output_err_fr);
	fclose(output_err_fr);

	// collect garbage files
	remove(input_path);
	remove(output_path);
	remove(output_err_path);

	// if err message exist, assume buffer overflow
	if (buf_size > 0) {
		return 1;
	} 
	else {
		return 0;
	}
}
