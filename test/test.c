#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../../include/ddmin.h"

int 
ddmin_partitions_test ();
int 
_ddmin_partitions_test (char * file, int n);

int
ddmin_buffer_overflow_test (char * jsonump_path);

int
main (int argc, char * argv[]) {

	if (argc < 2) {
		printf("argv[1]: ./jsondump");
		exit(1);
	}

	int fail = 0;

	// test1
	fail = ddmin_partitions_test();
	if (fail) {
		printf("failed: ddmin_partitions()\n");
	}
	else {
		printf("# PASS ddmin_partitions() in 88 test cases\n");
	}
	
	// test2
	fail = ddmin_buffer_overflow_test(argv[1]);
	if (fail) {
		printf("failed: ddmin_buffer_overflow()\n");
	}

}

int
ddmin_partitions_test () {

	char * file1_name = "./90_char";
	char * file2_name = "./100_char";
	char * file3_name = "./5000_char";

	int ret = 0;

	for (int i = 4; i < 89; i++) {
		ret |= _ddmin_partitions_test(file1_name, i);
		ret |= _ddmin_partitions_test(file2_name, i);
	}

	for (int i = 90; i < 250; i++) {
		ret |= _ddmin_partitions_test(file3_name, i);
	}
	/*
	*/

	//Test divide ./5000_char 5000 chars to 514 partitions
	//ret |= _ddmin_partitions_test(file3_name, 514);
	for (int i = 500; i < 700; i++) {
		ret |= _ddmin_partitions_test(file3_name, i);
	}
	/*
	*/

	return ret;
}

int 
_ddmin_partitions_test (char * file, int n) {

	long f_size = byte_count_file(file);
	int part_size = ceil(f_size / n) + 1;

	fprintf(stdout, "# Test divide %s %ld chars to %d partitions\n", file, f_size, n);

	char** partitions = malloc(sizeof(char*)*n);
	for (int i = 0; i < n; i ++) {
		partitions[i] = malloc(sizeof(char)*part_size);
	}
	int part_n = split_to_file(partitions, file, n);
	
	long sum_size = 0;
	for (int i = 0; i < part_n; i++) {
		char * path = partitions[i];
		//fprintf(stderr, "path: %s\n", path);

		long size = byte_count_file(path);
		sum_size += size;
	}
	if (part_n != n) {
		fprintf(stdout, "# Test divide %s %ld chars to %d partitions\n", file, f_size, n);
		fprintf(stdout, "fail to part %s when n is %d\n", file, n);
		fprintf(stderr, "part_n: %d, n: %d\n", part_n, n);
		return 1;
	}
	if (sum_size != f_size) {
		fprintf(stdout, "# Test divide %s %ld chars to %d partitions\n", file, f_size, n);
		fprintf(stdout, "fail to part %s when n is %d\n", file, n);
		fprintf(stdout, "sum_size: %ld, f_size: %ld\n", sum_size, f_size);
		return 1;
	}

	delete_files(partitions, part_n);
	free_paths(partitions, part_n);

	return 0;
}

int
ddmin_buffer_overflow_test (char * jsonump_path) {

	int ret = 0;

	if (!test_buffer_overflow(jsonump_path, "../crash.json")) {
		ret |= 1;
	}

	if (test_buffer_overflow(jsonump_path, "../library.json")) {
		ret |= 1;
	}
	if (!test_buffer_overflow(jsonump_path, "../mymin_result/21")) {
		ret |= 1;
	}

	return ret;
}
