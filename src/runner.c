#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "../include/runner.h"

runner_error_code
get_error (enum E_Type type, int exit_code);

pid_t c_pid;

void
kill_child(int sig) {

	kill(c_pid, SIGKILL);
}

runner_error_code
runner (char* target_path, char* input_path, char *output_path, char *output_err_path)
{

	signal(SIGALRM, kill_child);

	c_pid = fork();
        if (c_pid < 0) { 
		runner_error_code error_code = get_error(E_FORK, 0);
                return error_code;
        }

        /* Child process */
        if (c_pid == 0) { 

		//putenv("ASAN_OPTIONS=detect_leaks=0:halt_on_error=1");

		int input_fd = open(input_path, O_RDONLY);
                int out_fd = open(output_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
                int out_err_fd = open(output_err_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
                
		dup2(input_fd, STDIN_FILENO);
                dup2(out_fd, STDOUT_FILENO);
                dup2(out_err_fd, STDERR_FILENO);
                
		execl(target_path, target_path, NULL);
		perror("execl():");
        }
        
        /* Parent process */
	alarm(3);
	int status;
	int start = ((int) clock()) / CLOCKS_PER_SEC;
	waitpid(c_pid, &status, 0);
	int end = ((int) clock()) / CLOCKS_PER_SEC;

	int exit_stated = WEXITSTATUS(status);
	if ((end - start) >= 3) {
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
