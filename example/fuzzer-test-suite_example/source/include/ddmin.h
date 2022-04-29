char *
ddmin (char * program_path, char * char_seq_path, char * err_msg);

int
test_buffer_overflow (char * program_path, char * input_seq_path, char * err_msg);

long
byte_count_file (char * path);

int
split_to_file (char ** partition_path_arr, char * char_seq_path, int n);

void
free_paths (char ** path_arr, int len);

char *
complement_seq_files (int n, char ** seq_file_arr, int arr_len);

void
delete_files (char ** path_arr, int arr_len);

