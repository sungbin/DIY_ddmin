#include <stdio.h>
#include <stdlib.h>

#include "../../../include/ddmin.h"


int
ddmin_buffer_overflow_test (char * xmllint_path);

int
ddmin_partitions_test (char * xmllint_path);

int
main (int argc, char * argv[]) {

	if (argc < 2) {
		printf("argv[1]: ./xmllint");
		exit(1);
	}
	

	int fail;

	fail = ddmin_buffer_overflow_test(argv[1]);
	if (fail) {
		printf("failed: ddmin_buffer_overflow()\n");
	}
	else {
		printf("# PASS ddmin_buffer_overflow() in 1 test cases\n");
	}

	fail = ddmin_partitions_test(argv[1]);
	if (fail) {
		printf("failed: partition test()\n");
	}
	else {
		printf("# PASS partition test in 1 test cases\n");
	}



	return 0;
}


int
ddmin_buffer_overflow_test (char * xmllint_path) {

	int ret = 0;

	ret |= !test_buffer_overflow(xmllint_path, "./poc");
	if (ret) {
		fprintf(stderr, "return: %d \n", test_buffer_overflow(xmllint_path, "./poc"));
	}

	return ret;
}

int
ddmin_partitions_test (char * xmllint_path) {

	char name[256] = "../poc";
	char ** partition_path_arr;
	int splited_n;


	for (int n = 2; n <= 256; n*=2) {

		if (n == 256) {
			n = 143;
		}

		partition_path_arr = malloc(sizeof(char *) * n);
		splited_n = split_to_file(partition_path_arr, name, n);

		if ( n != splited_n) {
			fprintf(stderr,"error1\n");
			exit(1);
		}


		for (int i = 0; i < n; i++) {
			char * part = partition_path_arr[i];
			char * compl = complement_seq_files(i, partition_path_arr, n);

			int c1 = test_buffer_overflow(xmllint_path, part);
			int c2 = test_buffer_overflow(xmllint_path, compl);

			if (c1) {
				fprintf(stderr, "fail-part: %s\n", part);
			}
			if (c2) {
				fprintf(stderr, "fail-compl: %s\n", compl);
			}

		}

		free(partition_path_arr);
	}


	

}
