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
		printf("argv[0]: jsondump-path, argv[1]: crash-json-path\n");
	}

	char * json = read_file(argv[1]); //TODO: read as string (not hexa)
	printf("origin: %s\n", json);
/*
	char * result = ddmin(argv[0], json);

	printf("%s\n",result);

	free(result);
*/
	return 0;
}

char *
read_file (char * input_path) {

	int c_size = 512;
	char * out_buf = malloc(sizeof(char) * c_size);
	long f_size = 0;

	FILE * input_fp = fopen(input_path, "r");

	while ( 1 ) {

                char in_buf[512];
                int buf_size = fread(in_buf, 1, 512, input_fp);
		f_size = f_size + buf_size;

		if (f_size > c_size) {
			c_size = c_size + 512;
			out_buf = realloc(out_buf, c_size);
		}

		for (int _idx = 0; _idx < buf_size; _idx = _idx+1) {

                        sprintf(out_buf+_idx, "%02x", in_buf[_idx]);

                }

//		strcat(out_buf, in_buf);

                if (buf_size < 512) {
                        break;
                }
        }
	fclose(input_fp);
	return out_buf;
}
