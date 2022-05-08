#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <time.h>

#include "./include/runner.h"

runner_error_code
get_error (enum E_Type type, int exit_code);

runner_error_code
runner (char* target_path, char* input_path, char *output_path, char *output_err_path)
{
	pid_t pid = fork();
        if (pid < 0) { 
		runner_error_code error_code = get_error(E_FORK, 0);
                return error_code;
        }

        /* Child process */
        if (pid == 0) { 

                int out_fd = open(output_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
                int out_err_fd = open(output_err_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
                
                dup2(out_fd, STDOUT_FILENO);
                dup2(out_err_fd, STDERR_FILENO);
                
                execl(target_path, target_path, input_path, "-runs=1", NULL);
                _exit(1);
        }
        
        /* Parent process */
        int status = 0;
        int start, end;
        start = ((int)clock()) / CLOCKS_PER_SEC;
        end = ((int)clock()) / CLOCKS_PER_SEC;

        while ((end - start) < 10) {
		int w = waitpid(pid, &status, WNOHANG);
		if (w != 0) {
			break;
		}
                end = ((int)clock()) / CLOCKS_PER_SEC;
	}

	int exit_stated = WEXITSTATUS(status);
	if ((end - start) >= 4) {
		// Time out Kill
		kill(pid, SIGKILL);
		wait(&status);
		runner_error_code error_code = get_error(E_TIMEOUT_KILL, exit_stated);
		return error_code;
	}
	else {
		// normally exit
		runner_error_code error_code = get_error(NO_ERROR, exit_stated);
		return error_code;
	}

	/* cannot reach this area */
}

runner_error_code
get_error (enum E_Type type, int exit_code) {
	runner_error_code _code;
	_code.type = type;
	_code.exit_code = exit_code;

	return _code;
}
