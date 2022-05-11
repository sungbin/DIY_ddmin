#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../include/ddmin.h"
#include "../../../include/runner.h"
#include "../../../include/range.h"

char *
read_file (char * input_path);

extern int file_no;
extern int iter_no;

int
main (int argc, char * argv[]) {
	
	/* ddmin_dir()
	 *
	 * argv[1]: ddmin
	 * argv[2]: program_path
	 * argv[3]: input_dir
	 * argv[4]: err_msg
	 * argv[5]: (exe_dir)
	 *
	 * */

	/* range_dir(), range_increasing_dir()
	 *
	 * argv[1]: range
	 * argv[2]: program_path
	 * argv[3]: input_dir
	 * argv[4]: input_name
	 * argv[5]: err_msg
	 * argv[6]: (exe_dir)
	 *
	 * */

	if (argc < 5) {
		printf("ERROR: input paprameters must be argv[0]: jsondump-path, argv[1]: crash-json-path, argv[2]: [ddmin, mymin], argv[3]: error-msg, argv[4]: exe_dir\n");
		exit(1);
	}

	char * result;
	if(argv[1][0]=='d') {
		result = ddmin_dir(argv[1], argv[2], argv[3], argv[4], argv[5]);
		printf("The times of execute %d, iteration: %d \n", file_no-1, iter_no);
		printf("applied ddmin result: %s\n", result);
	}
	else if (strcmp(argv[1], "range_increasing") == 0) {
		result = range_increasing_dir(argv[2], strdup(argv[3]), argv[4], argv[5], 1, argv[6]);
		printf("The times of execute %d, iteration: %d \n", file_no, iter_no);
		printf("applied range-decending result: %s\n", result);
	}
	else if (argv[1][0]=='r') {
		char input_path[256];
		sprintf(input_path, "%s/%s", argv[3], argv[4]);
		result = range_dir(argv[2], strdup(argv[3]), strv[4], argv[5], (int)(byte_count_file(input_path)-1),argv[6], argv[5]);
		printf("The times of execute %d, iteration: %d \n", file_no, iter_no);
		printf("applied range result: %s\n", result);
	}

	if (result != argv[2]) {
		free(result);
	}
	return 0;
}

