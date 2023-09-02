#include "include.h"
#include "bg.h"
#include "echo.h"
#include "pwd.h"
#include "ls.h"
#include "discover.h"
#include <ctype.h>
#include "pinfo.h"
#include "cd.h"
#include "command.h"
#include <termios.h>
#include "token.h"

extern pid_t curr_fg;
int t2 = 0;
int t1;
int num_builtin_command;
extern int global_fg_bg_flag = 0;
int stdo_dup, stdi_dup;
#ifndef __APPLE__
char * built_in_command[11] = {"echo", "pwd", "cd", "discover", "history", "ls", "pinfo", "jobs", "fg", "bg" ,"sig"};
num_builtin_comm = 11;
#else
char * built_in_command[10] = {"echo", "pwd", "cd", "discover", "history", "ls","jobs", "fg", "bg", "sig"};
num_builtin_command = 10;
#endif

int is_builtin_command(char * command){
    for(int i=0;i<num_builtin_command;i++){
        if(strcmp(command, built_in_command[i]) == 0){
            return 1;
        }
    }
    return 0;
}

struct termios orig_termios;

void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

void control_c(){
    if(curr_fg)
    kill(curr_fg, SIGKILL);
}

void control_z(){
    
}

int main(){

    initialize_bground();
    stdo_dup = dup(1);
    stdi_dup = dup(0);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGCHLD, bg_child);
    signal(SIGINT, control_c);
    signal(SIGTSTP, control_z);

    char name[SYS_NAME_LEN];
    struct passwd * pass = getpwuid(getuid());
    gethostname(name, SYS_NAME_LEN);

    char path[PATH_LEN] = "~";
    char old_path[PATH_LEN];
    getcwd(old_path,PATH_LEN);
    char init_path[PATH_LEN];
    getcwd(init_path, PATH_LEN);

    int size = strlen(init_path);
    char buff[20][COMMAND_LEN];
    int num_history = 0;
    char arrey[COMMAND_LEN];
    char his_path[PATH_LEN];
    strcpy(his_path, init_path);
    strcat(his_path,"/history.txt");

    int fds[2];
    if(pipe(fds)==-1){
        printf("what is the fds?!\n");
    }

    while(1){
        dup2(stdi_dup, 0);
        dup2(stdo_dup, 1);
        char arr[COMMAND_LEN]; 
        char c;
        for(int i=0;i<COMMAND_LEN;i++){
            arr[i] = '\0';
        }
        int pt = 0;
prompt:
        if(t2 < 1){
            printf("<%s@%s:%s> ", pass->pw_name, name, path);
        }
        else{
            printf("<%s@%s:%s%s%d%s> ", pass->pw_name, name, path, "took ", t2,"s");
        }
        t2 = 0;

        printf("%s", arr);
        
        setbuf(stdout, NULL);
        enableRawMode();
        
        while (read(STDIN_FILENO, &c, 1) == 1) {
            if (iscntrl(c)) {

                if (c == 10){ 
                    break;
                }
                else if (c == 127) { // backspace
                    if (pt > 0) {
                        if (arr[pt-1] == 9) {
                            for (int i = 0; i < 7; i++) {
                                printf("\b");
                            }
                        }
                        arr[--pt] = '\0';
                        printf("\b \b");
                    }
                }
                else if(c == 4){
                    exit(0);
                }
                else if (c == 9) { // TAB character
                if(arr[pt-1] == 32){
                    arr[pt++] = c;
                    for (int i = 0; i < 8; i++) { // TABS should be 8 spaces
                        printf(" ");
                    }
                }
                else{
                    int pos = 1;
                    char file_name[MAXNAMLEN];
                    char out_name[MAXNAMLEN];
                    int file_dir;
                    int * file_or_dir = &file_dir;
                    while(pt-pos >= 0 && arr[pt-pos] != 32){
                        pos++;
                    }
                    pos--;
                    for(int i=0;i<pos;i++){
                        file_name[i] = arr[pt-pos+i];
                    }
                    file_name[pos] = '\0';
                    int possible = print_for_tab(file_name, out_name, file_or_dir);
                    if(possible > 1){
                        goto prompt;
                    }
                    else if(possible == 1){
                        int out_len = strlen(out_name);
                        int dpt = pt;
                        for(int i=0;i<out_len-pos;i++){
                            arr[dpt + i] = out_name[i + pos];
                            pt++;
                            printf("%c", out_name[i + pos]);
                        }
                        if(*file_or_dir == 0){
                            arr[pt++] = 32;
                            printf(" ");
                        }
                        else{
                            printf("/");
                            arr[pt++] = '/';
                        }
                    }
                }
                }
                else if (c == 4) {
                    exit(0);
                } 
                else {
                    printf("%d\n", c);
                }

            } else {
                arr[pt++] = c;
                printf("%c", c);
            }
        }
        
        disableRawMode();
        printf("\n");
        fflush(NULL);
        FILE * fp = fopen(his_path, "r");
        fflush(NULL);
        num_history = 0;
        int his_flag = 1;
        fseek(fp,0,SEEK_SET);
        while(fgets(arrey,COMMAND_LEN, fp) != NULL){
            if(num_history >= 19){
                break;
            }
            if(num_history == 0){
                if(strcmp(arrey, arr) == 0){
                    his_flag = 0;
                }
            }
            if(his_flag){
                strcpy(buff[++num_history], arrey);
            }
            else{
                strcpy(buff[num_history++],arrey);
            }
        }
        if(his_flag){
            strcpy(buff[0], arr);
        }
        int history_count;
        if(his_flag){
            history_count = num_history + 1;
        }
        else{
            history_count = num_history;
        }
        FILE * fp2 = fopen(his_path, "w");
        fseek(fp2,0,SEEK_SET);
        for(int i=0;i<history_count;i++){
            fprintf(fp2,"%s",buff[i]);
        }
        fflush(NULL);
        char * kot = NULL;
        char * tok = strtok_r(arr,";",&kot);
        if(tok == NULL){
            continue;
        }
        while(tok != NULL){
            int bg = 1;
            char * and = NULL;
            int and_flag = 1;
            if(tok[strlen(tok)-1] == '&'){
            and_flag = 0;
            }
            char * and_comm = strtok_r(tok, "&", &and);
            if(and  == NULL || (strlen(and) == 0 && and_flag)){
                bg = 0;
            }
            while(and_comm != NULL){
                int stdio_save[2];
                dup2(stdi_dup, 0);
                dup2(stdo_dup, 1);
                save_the_stdio(stdio_save);
                char * pipe_str = NULL;
                char * num_pipe_str;
                int num_pipes = 0;
                char pipe_str_comm[COMMAND_LEN];
                strcpy(pipe_str_comm,and_comm);
                char * pipe_comm = strtok_r(and_comm, "|", &pipe_str);
                char * pipe_count_str = strtok_r(pipe_str_comm, "|", &num_pipe_str);

                while(pipe_count_str!=NULL){
                    pipe_count_str = strtok_r(NULL, "|", &num_pipe_str);
                    num_pipes++;
                }
                num_pipes--;
                int pipe_count = 0;
                while(pipe_comm!=NULL){

                    char comm_copy[COMMAND_LEN];
                    strcpy(comm_copy, pipe_comm);
                    char * comm = NULL; 
                    char * command = strtok_r(pipe_comm, " \t",&comm);   // comm contains the rest of the string

                    if(num_pipes && is_builtin_command(command)){
                        if(pipe_count != num_pipes){
                        dup2(fds[1], 1);
                        }
                        else{
                        dup2(stdo_dup, 1);
                        }
                        close(fds[1]);
                    }

                    if(!command){
                        goto end;
                    }
                    if(strcmp(command, "echo") == 0){
                        int echo = handle_echo(comm);
                        if(num_pipes){
                                dup2(fds[0], 0);
                                close(fds[0]);
                                // close(1);
                                if(pipe(fds)==-1){
                                }
                                close(1);
                            }
                    }
                    else if(strcmp(command, "q") == 0){
                        return 0;
                    }
                    else if(strcmp(command, "pwd") == 0){
                        int pwd = handle_pwd(comm);
                        if(num_pipes){
                                dup2(fds[0], 0);
                                close(fds[0]);
                                if(pipe(fds)==-1){
                                }
                                close(1);
                            }
                    }
                    else if(strcmp(command, "cd") == 0){
                        int cd = handle_cd(comm,path,init_path,old_path,size);
                        if(num_pipes){
                                dup2(fds[0], 0);
                                close(fds[0]);
                                if(pipe(fds)==-1){
                                }
                                close(1);
                            }
                    }
                    else if(strcmp(command, "ls") == 0){
                        int ls = handle_ls(comm, init_path);
                        if(num_pipes){
                                dup2(fds[0], 0);
                                close(fds[0]);
                                if(pipe(fds)==-1){
                                }
                                close(1);
                            }
                    }
                    else if(strcmp(command, "discover") == 0){
                        int discover = handle_discover(&comm, init_path);
                        if(num_pipes){
                                dup2(fds[0], 0);
                                close(fds[0]);
                                if(pipe(fds)==-1){
                                }
                                close(1);
                            }
                    }
                    else if(strcmp(command, "history") == 0){
                        INITIALIZE_COMMAND("history")
                        if(num_arg != 1){
                            FREE_COMMAND
                            printf("Invalid Syntax\n");
                            goto end;
                        }
                        int smaller = (history_count > 9) ? 9 : history_count;
                        for(int i=smaller;i>=0;i--){
                            printf("%s", buff[i]);
                        }
                        FREE_COMMAND
                    }
                    else if(strcmp(command, "jobs") == 0){
                        int jobs = handle_jobs(comm);
                        if(!jobs){
                            goto end;
                        }
                    }
                    else if(strcmp(command, "sig") == 0){
                        int sig = handle_sig(comm);
                        if(!sig){
                            goto end;
                        }
                    }
                    else if(strcmp(command, "fg") == 0){
                        int fg = handle_fg(comm);
                        if(!fg){
                            goto end;
                        }
                    }
                    else if(strcmp(command, "bg") == 0){
                        int bg = handle_bg(comm);
                        if(!bg){
                            goto end;
                        }
                    }
                    #ifndef __APPLE__
                    else if(strcmp(command, "pinfo") == 0){
                        int pinfo = handle_pinfo(&comm);
                        if(!pinfo){
                            goto end;
                        }
                    }
                    #endif
                    else{
                        char * arg_list[MAX_ARGS_FOR_SYS_PROCESS];
                        char out_file[FILENAME_MAX];
                        char in_file[FILENAME_MAX];
                        int app;
                        int * append = &app;
                        int num_arg = io_redirect(comm, command, arg_list, out_file, in_file, append);
                        pid_t pid = fork();

                        if(pid == 0){
                            setpgid(0, 0);
                            if(num_pipes){
                                if(pipe_count != num_pipes){
                                dup2(fds[1], 1);
                                }
                                else{
                                dup2(stdo_dup, 1);
                                }
                                close(fds[0]);
                                close(fds[1]);
                            }
                            int ret = make_files_the_stds(out_file, in_file, append);
                            if(!ret){
                                goto end;
                            }
                            ret = execvp(command, arg_list);
                            if(ret == -1){
                                printf("No such command!\n");
                                close(1);
                            }
                            return 0;
                        }
                        else{
                            setpgid(pid, pid);
                            if(num_pipes){                  // if any process has a write end open then read for others will not terminate
                            dup2(fds[0], 0);                // by default changing the stdin for the next command in the pipe
                            close(fds[0]);
                            close(fds[1]);
                            // close(1);
                            dup2(0, 1);
                            if(pipe(fds)==-1){
                                printf("what is the fds?!\n");
                            }
                            close(1);
                            }
                            if(!bg){
                                
                                tcsetpgrp(STDIN_FILENO, pid);
                                t1 = time(NULL);
                                curr_fg = pid;
                                int status;
                                waitpid(pid,&status,WUNTRACED);
                                tcsetpgrp(STDIN_FILENO, getpid());
                                // printf("In main\n");
                                curr_fg = 0;
                                if(WIFSTOPPED(status)){
                                    global_fg_bg_flag = 1;
                                int position = put_into_bground(pid, comm_copy,0);
                                }
                                t2 = time(NULL) - t1;
                            }
                            else{
                                int position = put_into_bground(pid, comm_copy,1);
                                printf("[%d] %d\n", position, pid);
                            }
                        }
                        free_the_args(arg_list, num_arg);
                        goto norm_end;
                    }
                    end:
                    
                    norm_end:
                    pipe_comm = strtok_r(NULL, "|", &pipe_str);
                    // dup2(fds[0], 0);
                    // close(fds[0]);
                    pipe_count++;
                }
                restore_the_stdio(stdio_save);

                and_flag = 1;
            	if(and && and[strlen(and)-1] == '&'){
            	and_flag = 0;
            	}
                and_comm = strtok_r(NULL, "&", &and);
                if(and  == NULL || (strlen(and) == 0 && and_flag)){
                bg = 0;
            }
            }
            tok = strtok_r(NULL, ";",&kot);
        }
     }
}