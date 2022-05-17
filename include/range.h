#define THREAD_N 8

/*
struct pthread_data {
	int * part;
	int max_range_n;
	int thread_n;
	int rs;
	char * program_path;
	char * mmap_addr;
	char * err_msg;
	long f_size;
	int out_idx;
};
*/

struct range_data {

	char * checked_arr;
	int rs;
	char * program_path;
	char * mmap_addr;
	long f_size;
	char * err_msg;
};

struct pthread_data {

	struct range_data * range_data;
	int thread_n;
	int out_idx;
};

/*
struct pthread_result {
	char * file_path;
	int start;
};
*/

void 
test_ranges (int ** parts, int max_range_n, int rs);

void * 
test_range (void *data);

int
run_threads (char ** ret_list, void *test_range_func, int rs, char * program_path, char * mmap_addr, long f_size, char * err_msg);

int
range_thread (char ** ret_list, char * program_path, char * input_path, int rs, char * err_msg);

char*
range (char * program_path, char * input_path, char * err_msg);

char *
range_dir (char * program_path, char * input_dir, char * input_name, int rs, char * err_msg, char * exe_dir);

char *
range_increasing (char * program_path, char * input_path, char * err_msg);

char *
range_increasing_dir (char * program_path, char * input_dir, char * input_name, char * err_msg, char * exe_dir);
