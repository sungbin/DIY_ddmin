#include <stdio.h>
#include <stdlib.h>

#include "../../../include/ddmin.h"
#include "../../../include/range.h"

int
main (int argc, char * argv[]) {

	// ddmin test
	char program_path[512] = "./sapi/fuzzer/php-fuzz-tracing-jit";
	char f_name[256] = "../inputs";
	char err_msg[256] = "zend_unclean_zval_ptr_dtor";
	ddmin_dir(program_path, f_name, err_msg, "result_42859_ddmin"); 
	
	return 0;
}
