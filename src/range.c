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

pthread_cond_t cond;


//int begin;

int fail_max = 16;
char ** fail_arr;
int fail_n = 0;

int existing;

int cur_rs;

void * 
test_range (void *data) {
	
	struct pthread_data * d = (struct pthread_data*) data;

	while (1) {

		sem_wait(&q_sem);
		pthread_mutex_lock(&start_mt);
		struct node * n = pop_queue(q);
		pthread_mutex_unlock(&start_mt);
		
		pthread_mutex_lock(&end_mt);
		existing--;
		if (existing == 0 && q->count == 0) {
			fprintf(stderr, "signal (size=%d, rs=%d, begin=%d) \n", q->count, cur_rs, n->begin);
			sem_post(&x_sem);
		}
		pthread_mutex_unlock(&end_mt);
		free(n);

	}

	return NULL;
}


char*
_range (char * program_path, char * input_path, char * err_msg, long input_size, int rs) {

	fprintf(stderr, "last minimized: %s, %ld\n", input_path, input_size);


	fail_n = 0;

	for (int _rs = rs; _rs > 0; _rs--) {
	//for (int _rs = rs; _rs > 370; _rs--) {

		cur_rs = _rs;
		existing = input_size - _rs + 1;

		// fille queue
		for (int begin = 0 ; begin <= input_size - _rs; begin++) {
			//fprintf(stderr, "push (rs=%d, b=%d) \n", _rs, begin);
			pthread_mutex_lock(&start_mt);
			push_queue(q, rs, begin);
			pthread_mutex_unlock(&start_mt);
			sem_post(&q_sem);
		}


		fprintf(stderr, "wait (size=%d, rs=%d) \n", q->count, cur_rs);
		sem_wait(&x_sem);
		/*
		pthread_mutex_lock(&end_mt);
		while (q->count > 0) {
			fprintf(stderr, "wait (size=%d, rs=%d) \n", q->count, cur_rs);
			pthread_cond_wait(&cond, &end_mt);
		}
		pthread_mutex_unlock(&end_mt);
		*/

		// TODO: while num_sem(&q_sem) != 0 {}

		/*
		if (fail_n > 0) {

			//char ** fail_arr;
			//int fail_n = 0;
			int select_idx = rand() % fail_n;
			printf("select %d in [0, %d) \n", select_idx, fail_n);
			char * fail_path = fail_arr[select_idx];
			for (int i = 0; i < fail_n; i++) {
				if (i != select_idx) {
					free(fail_arr[i]);
				}
			}
			for (int i = 0; i < THREAD_N; i++) {
				fclose(p_data_arr[i]->in_fp);
				fclose(p_data_arr[i]->out_fp);
			}

			return _range(program_path, fail_path, err_msg, (input_size-cur_rs), MIN(cur_rs, (input_size-cur_rs-1)));

		}
		*/
	}

	/*
	for (int i = 0; i < THREAD_N; i++) {
		fclose(p_data_arr[i]->in_fp);
		fclose(p_data_arr[i]->out_fp);
	}
	*/
	return input_path;

}
char *
range (char * program_path, char * input_path, char * err_msg) {

	sem_init(&q_sem, 0, 0);
	sem_init(&x_sem, 0, 0);
	pthread_mutex_init(&start_mt, NULL);
	pthread_mutex_init(&end_mt, NULL);
	pthread_cond_init(&cond, NULL);

	fail_arr = malloc(sizeof(char*)*fail_max);
	long input_size = byte_count_file(input_path);
	p_data_arr = malloc(sizeof(struct pthread_data*) * THREAD_N);

	pthread_t p_threads[THREAD_N];

	q = malloc(sizeof(struct queue_t));
	init_queue(q);
	for (int i = 0; i < THREAD_N; i++) {

		p_data_arr[i] = malloc(sizeof(struct pthread_data));
		p_data_arr[i]->thread_idx = i;
		p_data_arr[i]->null_fp = fopen("/dev/null", "wb");
		p_data_arr[i]->out_file = malloc(sizeof(char)*16);
		//sprintf(p_data_arr[i]->out_file, "./thread%d.part", (i+THREAD_N*p_data_arr[i]->finded_n));
		sprintf(p_data_arr[i]->out_file, "./thread%d.part", (i+THREAD_N));
		p_data_arr[i]->in_fp = fopen(input_path, "rb");
		p_data_arr[i]->out_fp = fopen(p_data_arr[i]->out_file, "wb");
		p_data_arr[i]->in_fd = fileno(p_data_arr[i]->in_fp);
		p_data_arr[i]->out_fd = fileno(p_data_arr[i]->out_fp);
		
		pthread_create(&p_threads[i], NULL, test_range, (void *)p_data_arr[i]);
	}


	char * ret_path = _range(program_path, input_path, err_msg, input_size, input_size-1);

	sem_destroy(&q_sem);
	pthread_mutex_destroy(&start_mt);
	pthread_mutex_destroy(&end_mt);
	pthread_cond_destroy(&cond);

	for (int i = 0; i < THREAD_N; i++) {

		fclose(p_data_arr[i]->null_fp);
		fclose(p_data_arr[i]->in_fp);
		fclose(p_data_arr[i]->out_fp);
		free(p_data_arr[i]);
	}
	free(p_data_arr);
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
