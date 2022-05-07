#include <stdio.h>
#include <stdlib.h>

#include "../../../include/ddmin.h"
#include "../../../include/range.h"


int
main (int argc, char * argv[]) {

	char program_path[512] = "./sapi/fuzzer/php-fuzz-tracing-jit";
	char input_dir[256] = "../42859_range/inputs";
	char f_name[256] = "test_input";
	char err_msg[256] = "zend_unclean_zval_ptr_dtor";

	range_dir(program_path, input_dir, f_name, ((int)byte_count_file("../42859_range/test_input"))-1, err_msg, "42859_range"); 


	
	return 0;
}
