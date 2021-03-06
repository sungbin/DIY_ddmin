#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/ddmin.h"
#include "../include/runner.h"
#include "../include/range.h"

char *
read_file (char * input_path);

extern int file_no;
extern int fail_no;

//extern int * checked_arr;

int
main (int argc, char * argv[]) {
	
	// argv[1] = target exe file
	// argv[2] = crash input

	srand(time(NULL));
	if (argc < 4) {
		printf("ERROR: input paprameters must be argv[0]: jsondump-path, argv[1]: crash-json-path, argv[2]: [ddmin, mymin], argv[3]: error-msg\n");
		exit(1);
	}

	struct timeval  tv;
	double start, end;
	double res;

	gettimeofday(&tv, NULL);
	start = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

	time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(stderr, "START:  %d-%d-%d %d:%d:%d\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	char * result;
	if(argv[3][0]=='d') {
		result = ddmin(argv[1], argv[2], argv[4]);
		printf("The times of execute %d, iteration: %d \n", file_no-1, fail_no);
		printf("applied ddmin result: %s\n", result);
	}
	else if (strcmp(argv[3], "range_increasing") == 0) {
		result = range_increasing(argv[1], strdup(argv[2]), argv[4]);
		printf("The times of execute %d, iteration: %d \n", file_no, fail_no);
		printf("applied range-decending result: %s\n", result);
	}
	else if (argv[3][0]=='r') {
		result = range(argv[1], strdup(argv[2]), argv[4]);
		printf("The times of execute %d, iteration: %d \n", file_no, fail_no);
		printf("applied range result: %s\n", result);
	}

	t = time(NULL);
        tm = *localtime(&t);
        fprintf(stderr, "END:  %d-%d-%d %d:%d:%d\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	gettimeofday(&tv, NULL);
	end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	res = (end - start)/1000;
	fprintf(stderr, "thread(%d): %f \n", THREAD_N, res);	

	if (result != argv[2]) {
		free(result);
	}
	return 0;
}

