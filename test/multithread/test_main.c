#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "./include/range.h"
#include "../../include/ddmin.h"

int
main (int argc, char * argv[]) {

	char * target_path = argv[1];
	char * input_path = argv[2];

	if (access(target_path, F_OK)) {
                fprintf(stderr, "no program path: %s\n", target_path);
                exit(1);
        }

        if (access(input_path, F_OK)) {
                fprintf(stderr, "no input path: %s\n", input_path);
                exit(1);
        }	
	printf("target: %s, input: %s \n", target_path, input_path);

	long f_size = byte_count_file(input_path);
	int rs = 300;
	int max_range_n = (int)ceilf(((f_size - rs + 1) / (float)THREAD_N));

	int ** parts = malloc(sizeof(int**)*THREAD_N);
	for (int i = 0; i < THREAD_N; i++) {
		parts[i] = malloc(sizeof(int*)*max_range_n);
		for (int j = 0; j < max_range_n; j++) {
			parts[i][j] = -1;
		}
	}


	ranges(parts, f_size, rs);
	//test_ranges(parts, THREAD_N, max_range_n, rs);

	/* ## thread start ## */
	run_threads(parts, test_range, max_range_n, rs);

	for (int i = 0; i < THREAD_N; i++) {
		free(parts[i]);
	}
	free(parts);

	return 0;
}
void test_ranges (int ** parts, int max_range_n, int rs) {

	for (int i = 0; i < THREAD_N; i++) {
		printf("# THREAD NUMBER: %d \n", i);
		for (int j = 0; j < max_range_n; j++) {
			if (parts[i][j] == -1) {
				continue;
			}
			int start = parts[i][j];
			int end = start+rs;

			printf("(%d, %d) ", start, end);
		}
		printf("\n");
	}
	
}
