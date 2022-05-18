#define THREAD_N 8

struct pthread_data {
	char * program_path;
	char * input_path;
	long input_size;
        int rs;
        char * err_msg;
	int thread_idx;
};

void 
test_ranges (int ** parts, int max_range_n, int rs);

void * 
test_range (void *data);

int
run_threads (char ** ret_list, void *test_range_func, int rs, char * program_path, char * mmap_addr, long f_size, char * err_msg);

int
range_thread (char ** ret_list, char * program_path, char * input_path, int rs, char * err_msg);

char*
_range (char * program_path, char * input_path, char * err_msg, long input_size, int rs);

char*
range (char * program_path, char * input_path, char * err_msg);

char *
range_dir (char * program_path, char * input_dir, char * input_name, int rs, char * err_msg, char * exe_dir);

char *
range_increasing (char * program_path, char * input_path, char * err_msg);

char *
range_increasing_dir (char * program_path, char * input_dir, char * input_name, char * err_msg, char * exe_dir);

void 
init_cursor(int in_fd, int out_fd);

void
read_and_write(FILE * in_fp, FILE * out_fp, int n);
