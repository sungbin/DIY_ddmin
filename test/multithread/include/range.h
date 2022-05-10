struct pthread_data {
	int ** part;
	int max_range_n;
	int thread_n;
};

void test_ranges (int *** parts, int thread_n, int max_range_n);
void * test_range (void *data);

void
ranges (int *** parts, long f_size, int thread_n, int rs);
