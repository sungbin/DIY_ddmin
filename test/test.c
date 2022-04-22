#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/ddmin.h"

int p_no = 0;
int c_no = 0;

int 
ddmin_partitions_test ();
int 
_ddmin_partitions_test (char * file, int n);

int
ddmin_buffer_overflow_test (char * jsonump_path);

int
ddmin_result_test (char * jsonump_path);

int
main (int argc, char * argv[]) {

	if (argc < 2) {
		printf("argv[1]: ./jsondump");
		exit(1);
	}

	int fail = 0;

	// test1
	//fail = ddmin_partitions_test();
	if (fail) {
		printf("failed: ddmin_partitions()\n");
	}
	else {
		printf("# PASS split_to_file() in %d test cases\n", p_no);
		printf("# PASS complement_seq_files() in %d test cases\n", c_no);
	}
	
	// test2
	fail = ddmin_buffer_overflow_test(argv[1]);
	if (fail) {
		printf("failed: ddmin_buffer_overflow()\n");
	}
	else {
		printf("# PASS ddmin_buffer_overflow() in 3 test cases\n");
	}

	fail = ddmin_result_test(argv[1]);
	if (fail) {
		printf("failed: ddmin_result_test \n");
	}
	else {
		printf("# PASS ddmin_buffer_overflow() in 6 test cases\n");
	}
}

int
ddmin_partitions_test () {

	char * file1_name = "./input/90_char";
	char * file2_name = "./input/100_char";
	char * file3_name = "./input/5000_char";

	int ret = 0;

	for (int i = 4; i < 89; i++) {
		ret |= _ddmin_partitions_test(file1_name, i);
		ret |= _ddmin_partitions_test(file2_name, i);
	}

	for (int i = 90; i < 120; i++) {
		ret |= _ddmin_partitions_test(file3_name, i);
	}
	for (int i = 200; i < 230; i++) {
		ret |= _ddmin_partitions_test(file3_name, i);
	}
	//
	//Test divide ./5000_char 5000 chars to 514 partitions
	//ret |= _ddmin_partitions_test(file3_name, 514);
	for (int i = 550; i < 560; i++) {
		ret |= _ddmin_partitions_test(file3_name, i);
	}

	return ret;
}

int 
_ddmin_partitions_test (char * file, int n) {

	p_no++;

	long f_size = byte_count_file(file);
	int part_size = ceil(f_size / n) + 1;

//	fprintf(stdout, "# Test divide %s %ld chars to %d partitions\n", file, f_size, n);

	char** partitions = malloc(sizeof(char*)*n);
	for (int i = 0; i < n; i ++) {
		partitions[i] = malloc(sizeof(char)*part_size);
	}
	int part_n = split_to_file(partitions, file, n);
	
	long sum_size = 0;
	for (int i = 0; i < part_n; i++) {
		char * path = partitions[i];
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
		fprintf(stderr, "sum_size: %ld, f_size: %ld\n", sum_size, f_size);
		return 1;
	}

	// Test complement
	
	for (int i = 0; i < part_n; i++) {
		c_no++;
		char * path = partitions[i];
		long s_size = byte_count_file(path);
	
		char * com_path = complement_seq_files(i, partitions, part_n);
		long c_size = byte_count_file(com_path);

		if (f_size != s_size + c_size) {
			
			fprintf(stdout, "# Test Fail to divide %s %ld chars to %d partitions\n", file, f_size, n);
			fprintf(stderr, "f_size: %ld, s_size: %ld, c_size: %ld\n", f_size, s_size , c_size);
			return 1;
		}

		free(com_path);
	}


	delete_files(partitions, part_n);
	free_paths(partitions, part_n);

	return 0;
}

int
ddmin_buffer_overflow_test (char * jsonump_path) {

	int ret = 0;

	if (!test_buffer_overflow(jsonump_path, "./input/crash.json")) {
		ret |= 1;
	}

	if (test_buffer_overflow(jsonump_path, "../example/library.json")) {
		ret |= 1;
	}
	if (!test_buffer_overflow(jsonump_path, "../example/mymin_result/21")) {
		ret |= 1;
	}

	return ret;
}

int
ddmin_result_test (char * jsonump_path) {

	char * in_file;
       	char * minimized_path;

	in_file = "./input/f_6";
	minimized_path = ddmin(jsonump_path, in_file);
	if (byte_count_file(minimized_path) != 6l) {
		fprintf(stdout, "# Test Fail to ddmin on %s\n", in_file);
		return 1;
	}
	free(minimized_path);

	in_file = "./input/f_12_1";
	minimized_path = ddmin(jsonump_path, in_file);
	if (byte_count_file(minimized_path) != 6l) {
		fprintf(stdout, "# Test Fail to ddmin on %s\n", in_file);
		return 1;
	}
	free(minimized_path);
	
	in_file = "./input/f_12_2";
	minimized_path = ddmin(jsonump_path, in_file);
	if (byte_count_file(minimized_path) != 6l) {
		fprintf(stdout, "# Test Fail to ddmin on %s\n", in_file);
		return 1;
	}
	free(minimized_path);

	in_file = "./input/f_7";
	minimized_path = ddmin(jsonump_path, in_file);
	if (byte_count_file(minimized_path) != 6l) {
		fprintf(stdout, "# Test Fail to ddmin on %s\n", in_file);
		return 1;
	}
	free(minimized_path);

	in_file = "./input/f_11";
	minimized_path = ddmin(jsonump_path, in_file);
	if (byte_count_file(minimized_path) != 6l) {
		fprintf(stdout, "# Test Fail to ddmin on %s\n", in_file);
		return 1;
	}
	free(minimized_path);

	in_file = "./input/f_8";
	minimized_path = ddmin(jsonump_path, in_file);
	if (byte_count_file(minimized_path) != 8l) {
		fprintf(stdout, "# Test Fail to ddmin on %s\n", in_file);
		return 1;
	}
	free(minimized_path);



	return 0;
}
