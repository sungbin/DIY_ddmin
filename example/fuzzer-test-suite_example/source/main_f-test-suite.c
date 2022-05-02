#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./include/ddmin.h"
#include "./include/runner.h"
#include "./include/range.h"

char *
read_file (char * input_path);

extern int file_no;
extern int iter_no;

int
main (int argc, char * argv[]) {
	
	// argv[1] = jsondump
	// argv[2] = crash json

	if (argc < 4) {
		printf("ERROR: input paprameters must be argv[0]: <executable_file>, argv[1]: crash-file-path, argv[2]: [ddmin, range] argv[3]: <fail-error-maessage>\n");
		exit(1);
	}

	char * result;
	if(argv[3][0]=='d') {
		result = ddmin(argv[1], argv[2], argv[4]);
		printf("The times of execute %d, iteration: %d \n", file_no-1, iter_no);
		printf("applied ddmin result: %s\n", result);
	}
	else if(argv[3][0]=='r') {
		result = range(argv[1], argv[2], argv[4]);
		printf("The times of execute %d, iteration: %d \n", file_no, iter_no);
		printf("applied range result: %s\n", result);
	}

	if (result != argv[2]) {
		free(result);
	}
	return 0;
}
