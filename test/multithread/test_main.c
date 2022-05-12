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
	char * err_msg = "dump example/jsondump.c:44";

	if (access(target_path, F_OK)) {
                fprintf(stderr, "no program path: %s\n", target_path);
                exit(1);
        }

        if (access(input_path, F_OK)) {
                fprintf(stderr, "no input path: %s\n", input_path);
                exit(1);
        }	
	
	printf("target: %s, input: %s \n", target_path, input_path);

	/*
        char ** ret_list = malloc(sizeof(char*) * THREAD_N);
        int ret_cnt = range_thread(ret_list, target_path, input_path, 1, err_msg);

	free(ret_list);
	*/	

	char * minimized = range(target_path, input_path, err_msg);
	printf("last_minimized: %s \n", minimized);

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

