#include <stdio.h>
#include <stdlib.h>



//int
//ddmin_buffer_overflow_test ();



int
main (int argc, char * argv[]) {


	
	
	// test 2
	/*
	char f_name[256] = "../fuzzer-test-suite/libxml2-v2.9.2/crash-d8960e21ca40ea5dc60ad655000842376d4178a1";
	char program_path[512] = "../libxml2/libxml2-v2.9.2-fsanitize_fuzzer";
	char err_msg[256] = "AddressSanitizer: heap-buffer-overflow";
	*/

	// test 3
	/*
	char f_name[256] = "../fuzzer-test-suite/libxml2-v2.9.2/leak-bdbb2857b7a086f003db1c418e1d124181341fb1";
	char program_path[512] = "../libxml2/libxml2-v2.9.2-fsanitize_fuzzer";
	char err_msg[256] = "LeakSanitizer: detected memory leaks";
	*/
	
	// test 5
	char f_name[256] = "../fuzzer-test-suite/sqlite-2016-11-14/leak-b0276985af5aa23c98d9abf33856ce069ef600e2";
	char program_path[512] = "../sqlite/sqlite-2016-11-14-fsanitize_fuzzer";
	char err_msg[256] = "LeakSanitizer: detected memory leaks";
	/*
	*/

	//test 7
	/*
	char f_name[256] = "../fuzzer-test-suite/sqlite-2016-11-14/crash-1066e42866aad3a04e6851dc494ad54bc31b9f78";
	char program_path[512] = "../sqlite/sqlite-2016-11-14-fsanitize_fuzzer";
	char err_msg[256] = "AddressSanitizer: heap-use-after-free";
	*/

	//#include "../../../include/ddmin.h"
	//char * result = ddmin(program_path, f_name, err_msg);


	#include "../../../include/range.h"
	char * result = range(program_path, f_name, 50, err_msg);

	printf("result: %s\n", result);

	free(result);

	
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
