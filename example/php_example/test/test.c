#include <stdio.h>
#include <stdlib.h>

#include "../../../include/ddmin.h"
#include "../../../include/range.h"

//int
//ddmin_buffer_overflow_test ();



int
main (int argc, char * argv[]) {


	
	
	// test 5
/*
	./sapi/fuzzer/php-fuzz-tracing-jit ../inputs --runs=1
*/
	char f_name[256] = "../inputs";
	char program_path[512] = "./sapi/fuzzer/php-fuzz-tracing-jit";
	char err_msg[256] = "AddressSanitizer: heap-use-after-free";

	//char * result = range(program_path, f_name, 50, err_msg);
	//int ret = test_buffer_overflow(program_path, f_name, err_msg);

	ddmin(program_path, f_name, err_msg); 


	//printf("result: %d\n", ret);

	
	return 0;
}

/*
int
ddmin_buffer_overflow_test () {


	int ret = 0;

	ret = test_buffer_overflow(program_path, f_name, err_msg);

	if (ret) {
		fprintf(stderr,"test fail: %s, %s\n", program_path, f_name);
	}
	else {
		fprintf(stderr,"test pass: %s, %s\n", program_path, f_name);
	}

	return ret;
}
*/
