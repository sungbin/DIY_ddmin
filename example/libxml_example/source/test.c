#include <stdio.h>
#include <stdlib.h>

#include "../../../include/ddmin.h"


int
ddmin_buffer_overflow_test (char * xmllint_path);

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
