#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "../include/ddmin.h"
#include "../include/range.h"
#include "../include/runner.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


extern int file_no;
extern int iter_no;
extern char * minimized_fname;

char *
range (char * program_path, char * char_seq_path) {


}

int
test_buffer_overflow_range (char * program_path, char * input_seq_path) {

}

int
split_to_file_range (char ** partition_path_arr, char * char_seq_path, int n) {



}
