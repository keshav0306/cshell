char* permish(struct stat sb, char * path);

int ls_l(char * dir, int dir_len, char * name, int flag);

int ls_main(char * dir, int a_flag, int l_flag);

int handle_ls(char * comm, char * init_path);

int print_for_tab(char * file_name, char * out_name, int * file_or_dir);