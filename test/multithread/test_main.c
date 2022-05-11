#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/errno.h>

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

	char * err_msg = "dump example/jsondump.c:44";
	long f_size = byte_count_file(input_path);
	int fd;
	char * mmap_addr;


	if((fd = open(input_path, O_RDONLY)) == -1) {
		perror("ERROR: open fd");
		exit(1);
	}

	mmap_addr = mmap(0, f_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mmap_addr == MAP_FAILED) {
		perror("mmap error");
		exit(1);
	}

	fprintf(stderr, "f_size: %ld \n", f_size);
	fprintf(stderr, "mmap_addr: %s \n", mmap_addr);

	int rs = 370;
	int max_range_n = (int)ceilf(((f_size - rs + 1) / (float)THREAD_N));

	int ** parts = malloc(sizeof(int**)*THREAD_N);
	for (int i = 0; i < THREAD_N; i++) {
		parts[i] = malloc(sizeof(int*)*max_range_n);
		for (int j = 0; j < max_range_n; j++) {
			parts[i][j] = -1;
		}
	}

	ranges(parts, f_size, rs);
	//test_ranges(parts, max_range_n, rs);

	/* ## thread start ## */
	run_threads(parts, test_range, max_range_n, rs, target_path, mmap_addr, f_size, err_msg);

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
