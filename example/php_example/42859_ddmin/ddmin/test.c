#include <stdio.h>
#include <stdlib.h>

#include "../../../../include/ddmin.h"
#include "../../../../include/range.h"

int
main (int argc, char * argv[]) {

	// ddmin test
	char program_path[512] = "./sapi/fuzzer/php-fuzz-tracing-jit";
	char input_dir[256] = "../ddmin/inputs";
	char err_msg[256] = "zend_unclean_zval_ptr_dtor";

	ddmin_dir(program_path, input_dir, err_msg, "ddmin"); 
	
	return 0;
}
