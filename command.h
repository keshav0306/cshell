int io_redirect(char * comm, char * command, char * arg_list[], char * out_file, char * in_file, int * append);

int make_files_the_stds(char * out_file, char * in_file, int *append);

void free_the_args(char * arg_list[], int num_arg);

void restore_the_stdio(int * stdio);

void save_the_stdio(int * stdio);