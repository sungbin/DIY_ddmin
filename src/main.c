#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ddmin.h"
#include "../include/runner.h"

char *
read_file (char * input_path);

int
main (int argc, char * argv[]) {
	
	// argv[0] = jsondump
	// argv[1] = crash json

	if (argc < 2) {
		printf("ERROR: input paprameters must be argv[0]: jsondump-path, argv[1]: crash-json-path\n");
		exit(1);
	}

	char * result = ddmin(argv[0], argv[1]);

	printf("applied ddmin to %s\n",result);

	free(result);
	return 0;
}

