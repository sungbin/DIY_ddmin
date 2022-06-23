char *
ddmin (char * program_path, char * char_seq_path, char * err_msg);

char *
ddmin_dir (char * program_path, char * input_dir, char * err_msg, char * exe_dir);

int
test_buffer_overflow (char * program_path, char * input_seq_path, char * err_msg);

long
byte_count_file (char * path);

void
copy (const char *src, const char *dst);

int
test_buffer_overflow_thread (char * program_path, char * input_seq_path, char * err_msg, int out_idx);
