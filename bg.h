struct bg{
    char * command;
    pid_t pid;
    int r_s; // running = 1, stopped = 0
    int position;
};

void initialize_bground();
int put_into_bground(pid_t pid, char * command, int rs);
void bg_child();
int handle_jobs(char * comm);
int handle_sig(char * comm);
int handle_fg(char * comm);
int handle_bg(char * comm);