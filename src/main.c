#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ddmin.h"
#include "../include/mymin.h"
#include "../include/runner.h"

char *
read_file (char * input_path);

int
main (int argc, char * argv[]) {
	
	// argv[1] = jsondump
	// argv[2] = crash json

	if (argc < 3) {
		printf("ERROR: input paprameters must be argv[0]: jsondump-path, argv[1]: crash-json-path, argv[2]: [ddmin, mymin]\n");
		exit(1);
	}

	char * result;
	if(argv[3][0]=='d') {
		result = ddmin(argv[1], argv[2]);
	}
	else if(argv[3][0]=='m') {
		result = mymin(argv[1], argv[2]);
	}

	printf("applied ddmin result: %s\n", result);

	free(result);
	return 0;
}

