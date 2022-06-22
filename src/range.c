#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>


#include "../include/range.h"
#include "../include/ddmin.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void
init_queue (struct queue_t * queue);
void
push_queue (struct queue_t * q, int rs, int begin);
struct node *
pop_queue (struct queue_t * q);

extern int file_no;
extern int fail_no;

struct queue_t * q;

struct pthread_data ** p_data_arr;

sem_t q_sem;
sem_t x_sem;
pthread_mutex_t start_mt;
pthread_mutex_t end_mt;
pthread_mutex_t find_mt;

FILE * null_fp;
FILE * in_fps[THREAD_N];
int in_fds[THREAD_N];
char * out_files[THREAD_N];
FILE * out_fps[THREAD_N];
int out_fds[THREAD_N];


//int begin;
int fail_stack[THREAD_N];

unsigned int total_fail_n = 0;
int fail_max = 16;
char ** fail_arr;
int fail_n = 0;

int existing;

int rs;
char * input_path;
int input_size;
char * program_path;
char * err_msg;

void * 
test_range (void *data) {
	
	int th_idx = (int) data;

	while (1) {

		// pop
		sem_wait(&q_sem);
		pthread_mutex_lock(&start_mt);
		struct node * n = pop_queue(q);
		pthread_mutex_unlock(&start_mt);
		
		// logic of range

		init_cursor(in_fds[th_idx], out_fds[th_idx]);
		read_and_write(in_fps[th_idx], out_fps[th_idx], n->begin); //prefix
		read_and_write(in_fps[th_idx], null_fp, rs); //rs
		read_and_write(in_fps[th_idx], out_fps[th_idx], input_size - (n->begin + rs)); //postfix
		if (fflush(out_fps[th_idx]) == -1) {
			perror("ERROR: flush");
			exit(1);
		}
		int bt;
		if ((bt = byte_count_file(out_files[th_idx])) > (input_size - rs)) {
			if (truncate(out_files[th_idx], input_size - rs) == -1) {
				perror("ERROR: subset truncate");
				exit(1);
			}
		}
		else if (bt < (input_size - rs)) {
			fprintf(stderr, "(th=%d)ERROR: few written. bt:%d, correct: %d \n", th_idx, bt, (input_size - rs));
			exit(1);
		}

		int e_code = test_buffer_overflow_thread(program_path, out_files[th_idx], err_msg, th_idx);
		if (file_no++ % 1000 == 0) {
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			fprintf(stderr, "(rs=%d)Test count:%d -  %d-%d-%d %d:%d:%d\n", rs, file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		}


		if (e_code == 1) {

			pthread_mutex_lock(&find_mt);
			int fail_idx = fail_n;
			fail_n++;
			pthread_mutex_unlock(&find_mt);

			if (fail_n >= (fail_max - THREAD_N)) {
				fail_max *= 2;
				fail_arr = realloc(fail_arr, sizeof(char*)*fail_max);
			}

			fail_arr[fail_idx] = out_files[th_idx];
			fclose(out_fps[th_idx]);
			
			out_files[th_idx] = malloc(sizeof(char) * 32);
			fail_stack[th_idx]++;
			sprintf(out_files[th_idx], "./thread%d.part", th_idx + THREAD_N*fail_stack[th_idx]);

			out_fps[th_idx] = fopen(out_files[th_idx], "wb");
			out_fds[th_idx] = fileno(out_fps[th_idx]);
		}


		// closing
		pthread_mutex_lock(&end_mt);
		existing--;
		//fprintf(stderr, "existing: %d \n", existing);
		if (existing == 0) {
			//fprintf(stderr, "push (x_sem) \n");
			sem_post(&x_sem); // end signal
		}
		pthread_mutex_unlock(&end_mt);
		free(n);

	}

	return NULL;
}


char*
_range (char * _input_path, long _input_size, int _rs) {


	// input settings
	input_path = _input_path;
	input_size = _input_size;
	
	for (int i = 0; i < THREAD_N; i++) {

                in_fps[i] = fopen(_input_path, "rb");
		if (in_fps[i] == NULL) {
			fprintf(stderr, "ERROR: fopen(input) \n");
				exit(1);
		}
                in_fds[i] = fileno(in_fps[i]);
        }

	for (int r_size = _rs; r_size > 0; r_size--) {

		fail_n = 0;
		rs = r_size;
		existing = input_size - r_size + 1;

		// fille queue
		for (int begin = 0 ; begin <= input_size - r_size; begin++) {
			pthread_mutex_lock(&start_mt);

			push_queue(q, r_size, begin);
			pthread_mutex_unlock(&start_mt);
			sem_post(&q_sem);
		}

		// wait signal
		sem_wait(&x_sem);

		if (fail_n > 0) {

			//char ** fail_arr;
			//int fail_n = 0;
			int select_idx = rand() % fail_n;
			fprintf(stderr, "select %d in [0, %d) \n", select_idx, fail_n);

			total_fail_n++;
			char * fail_path = malloc(sizeof(char) * 32);
			sprintf(fail_path, "%u.part", total_fail_n);

			if (rename(fail_arr[select_idx], fail_path) == -1) {
				perror("fail to rename n.part");
				exit(1);
			}

			for (int i = 0; i < THREAD_N; i++) {
				fclose(in_fps[i]);

				in_fps[i] = fopen(fail_path, "rb");
				in_fds[i] = fileno(in_fps[i]);
			}

			free(_input_path);

			fprintf(stderr, "last minimized: %s, %d\n", fail_path, input_size-rs);
			return _range(fail_path, (input_size-rs), MIN(rs, (input_size-rs-1)));

		}
		else {
		}
	}


	for (int i = 0; i < THREAD_N; i++) {
		fclose(in_fps[i]);
	}

	return input_path;

}
char *
range (char * _program_path, char * _input_path, char * _err_msg) {

	program_path = _program_path;
	err_msg = _err_msg;

	sem_init(&q_sem, 0, 0);
	sem_init(&x_sem, 0, 0);
	pthread_mutex_init(&start_mt, NULL);
	pthread_mutex_init(&end_mt, NULL);
	pthread_mutex_init(&find_mt, NULL);

	fail_arr = malloc(sizeof(char*)*fail_max);

	//fprintf(stderr, "1 input_path: %s \n", _input_path);
	long input_size = byte_count_file(_input_path);

	null_fp = fopen("/dev/null", "wb");

	pthread_t p_threads[THREAD_N];

	q = malloc(sizeof(struct queue_t));
	init_queue(q);
	for (int i = 0; i < THREAD_N; i++) {

		out_files[i] = malloc(sizeof(32));
                sprintf(out_files[i], "thread%d", i);
                out_fps[i] = fopen(out_files[i], "wb");
                out_fds[i] = fileno(out_fps[i]);

		pthread_create(&p_threads[i], NULL, test_range, (void *) i);
	}


	char * ret_path = _range(_input_path, input_size, input_size-1);


	sem_destroy(&q_sem);
	sem_destroy(&x_sem);
	pthread_mutex_destroy(&start_mt);
	pthread_mutex_destroy(&end_mt);
	pthread_mutex_destroy(&find_mt);

	for (int i = 0; i < THREAD_N; i++) {

		fclose(out_fps[i]);
	}
	fclose(null_fp);
	free(fail_arr);



	return ret_path;
}

void 
init_cursor(int in_fd, int out_fd) {
	
	if (lseek(in_fd, 0, SEEK_SET) == -1) {
		perror("lseek(in_fd)");
		exit(1);
	}
	if (lseek(out_fd, 0, SEEK_SET) == -1) {
		perror("lseek(out_fd)");
		exit(1);
	}
}

void
read_and_write(FILE * in_fp, FILE * out_fp, int n) {

	while (n > 0) {

		char buf[2048];
		int b_size;

		if (n > 2048) {
			b_size = fread(buf, 1, 2048, in_fp);
		}
		else {
			b_size = fread(buf, 1, n, in_fp);
		}
		//fprintf(stderr, "read %d (n=%d)\n", b_size, n);
		if (b_size != fwrite(buf, 1, b_size, out_fp)) {
			fprintf(stderr, "ERROR on write\n");
			exit(1);
		}
		//fprintf(stderr, "write %d (n=%d)\n", b_size, n);
		n -= b_size;
	}

}

char *
range_increasing (char * program_path, char * input_path, char * err_msg) {

	int rs = 1;
	int input_size;

	char * out_file = calloc(sizeof(char), 512);
	sprintf(out_file, "./%d.part", ++fail_no);

	FILE * in_fp = fopen(input_path, "rb");
	FILE * out_fp = fopen(out_file, "wb");
	int in_fd = fileno(in_fp);
	int out_fd = fileno(out_fp);

	while (rs < (input_size = byte_count_file(input_path))) {
		printf("rs: %d\n", rs);
	
		int begin = 0;
		while (begin <= ((input_size = byte_count_file(input_path)) - rs)) {

			init_cursor(in_fd, out_fd);
			read_and_write(in_fp, out_fp, begin); //prefix
			FILE * null_fp = fopen("/dev/null", "wb");
			read_and_write(in_fp, null_fp, rs); //rs
			fclose(null_fp);
			read_and_write(in_fp, out_fp, input_size - (begin + rs)); //postfix
			if (fflush(out_fp) == -1) {
				perror("ERROR: flush");
				exit(1);
			}
			int bt;
			if ((bt = byte_count_file(out_file)) > (input_size - rs)) {
				if (truncate(out_file, input_size - rs) == -1) {
					perror("ERROR: subset truncate");
					exit(1);
				}
			}
			else if (bt < (input_size - rs)) {
				fprintf(stderr, "ERROR: few written. bt:%d, correct: %d \n", bt, (input_size - rs));
				exit(1);
			}

			int e_code = test_buffer_overflow(program_path, out_file, err_msg);
			if (file_no % 1000 == 0) {
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				printf("Test count:%d -  %d-%d-%d %d:%d:%d\n", file_no, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			}
                        if (e_code == 1) {

				printf("last minimized: %s, %d\n", out_file, bt);

				fclose(in_fp);
				fclose(out_fp);
				free(input_path);
				input_path = strdup(out_file);
				in_fp = fopen(input_path, "rb");
				sprintf(out_file, "./%d.part", ++fail_no);
				out_fp = fopen(out_file, "wb");
				in_fd = fileno(in_fp);
				out_fd = fileno(out_fp);

				begin = MAX(begin - rs + 1, 0);

			}
			else {
				begin++;
			}
		}
		rs++;
	}
	return input_path;

}

void
init_queue (struct queue_t * queue) {

	queue->front = NULL;
	queue->rear = NULL;
	queue->count = 0;
}

void
push_queue (struct queue_t * q, int rs, int begin) {

	struct node * node = malloc(sizeof(struct node));
	node->begin = begin;
	node->rs = rs;
	node->next = NULL;

	if (q->count == 0) {
		q->front = node;
	}
	else {
		q->rear->next = node;
	}
	q->rear = node;
	q->count++;
}

struct node *
pop_queue (struct queue_t * q) {
	if (q->count == 0) {
		fprintf(stderr, "cannot pop_queue()\n");
		exit(1);
	}
	struct node * ptr = q->front;
	q->front = ptr->next;
	q->count--;
	return ptr;
}
