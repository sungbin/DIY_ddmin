#define THREAD_N 12

struct pthread_data {
	/*global*/
	/*
	char * program_path;
	char * input_path;
	long input_size;
        char * err_msg;
	int finded_n;
	
	char * out_file;
	FILE * in_fp;
	FILE * out_fp;
	FILE * null_fp;
	int in_fd;
	int out_fd;
	*/

	/*thread input*/
	int thread_idx;

	/*queue*/
	/*
	int begin;
        int rs;
	*/

};

struct node {

	int rs;
	int begin;
	struct node * next;
};

struct queue_t {
	unsigned int count; 
	struct node * front;
	struct node * rear;
};


void 
test_ranges (int ** parts, int max_range_n, int rs);

void * 
test_range (void *data);

int
run_threads (char ** ret_list, void *test_range_func, int rs, char * program_path, char * mmap_addr, long f_size, char * err_msg);

char*
_range (char * _input_path, long _input_size, int _rs);

char*
range (char * _program_path, char * _input_path, char * _err_msg);

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
