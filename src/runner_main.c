#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include "../include/runner.h"

int 
main (int argc, char* argv[])
{

	char * target_path = "../jsondump";
	char * input_path = "../crash.json";
	//char * input_path = "../library.json";
	char * output_path = "./runner_result.txt";
	char * output_err_path = "./runner_result.txt.err";

	// argv[1]: target_path;
	// argv[2]: input_path;
	// argv[3]: output_path;
	// argv[4]: output_err_path;
	
	runner_error_code error_code = runner(target_path, input_path, output_path, output_err_path);

	printf("%d %d\n", error_code.type, error_code.exit_code);

	return 0;
}
