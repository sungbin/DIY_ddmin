#define THREAD_N 8

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

void 
test_ranges (int ** parts, int max_range_n, int rs);

void * 
test_range (void *data);

void
ranges (int ** parts, long f_size, int rs);

void
run_threads (int ** parts, void *test_range_func, int max_range_n, int rs, char * program_path, char * mmap_addr, long f_size, char * err_msg);
