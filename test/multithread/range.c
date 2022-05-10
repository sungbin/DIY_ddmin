#include <math.h>

#include "./include/range.h"

void
ranges (int *** parts, long f_size, int thread_n, int rs) {

	int thread_idx = 0;
	int idx = 0;
	int max_idx = (f_size - rs + 1);

	do {
		int range_cnt = ceilf((max_idx / (float)(thread_n - thread_idx)));

		for (int i = 0; i < range_cnt; i++) {

			int start = i + idx;
			int end = start + rs;

			//fprintf(stderr, "f_size: %ld, rs: %d, thread_n: %d, thread_idx: %d, max_range_n: %d max_idx: %d \n", f_size, rs, thread_n, thread_idx, max_range_n, max_idx);
			//fprintf(stderr, "range_cnt: %d, start: %d, end: %d \n\n", range_cnt, start, end);
		
			
			parts[thread_idx][i][0] = start;
			parts[thread_idx][i][1] = end;
		}
		
		idx += range_cnt;

		max_idx -= range_cnt;
		thread_idx++;

	} while(thread_idx < thread_n);

}
