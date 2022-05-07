char *
ddmin (char * program_path, char * char_seq_path, char * err_msg);

char *
ddmin_dir (char * program_path, char * input_dir, char * err_msg, char * exe_dir);

int
test_buffer_overflow (char * program_path, char * input_seq_path, char * err_msg);

long
byte_count_file (char * path);

int
copy (const char *src, const char *dst);
