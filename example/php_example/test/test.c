#include <stdio.h>
#include <stdlib.h>

#include "../../../include/ddmin.h"
#include "../../../include/range.h"

//int
//ddmin_buffer_overflow_test ();



int
main (int argc, char * argv[]) {


	// ddmin test
	/*
	char program_path[512] = "./sapi/fuzzer/php-fuzz-tracing-jit";
	char f_name[256] = "../inputs";
	char err_msg[256] = "zend_unclean_zval_ptr_dtor";
	ddmin_dir(program_path, f_name, err_msg); 
	*/

	char program_path[512] = "./sapi/fuzzer/php-fuzz-tracing-jit";
	char input_dir[256] = "../inputs";
	char f_name[256] = "test_input";
	char err_msg[256] = "zend_unclean_zval_ptr_dtor";
	range_dir(program_path, input_dir, f_name, (int) byte_count_file("../test_input")- 1000, err_msg); 



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
