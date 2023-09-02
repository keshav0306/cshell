#include "include.h"

int io_redirect(char * comm, char * command, char * arg_list[], char * out_file, char * in_file, int * append){

    arg_list[0] = command;
    int num_arg = 1;
    int input_f = 0;
    int output_f = 0;
    int output_fa = 0;
    out_file[0] = 0;
    in_file[0] = 0;
    *append = 0;
    char arg[COMMAND_LEN];
    int head = 0;
    int begin_arg = 0;
    int next_be = -1;
    int comm_size;
    if(comm != NULL){
        comm_size = strlen(comm);
    }
    else{
        arg_list[num_arg] = NULL;
        return num_arg;
    }

    for(int i=0;i < comm_size;i++){
        if(comm[i] == ' ' || comm[i] == '\t'){
            if(begin_arg){
                arg[head] = 0;
                if(next_be == 0){
                    strcpy(in_file, arg);
                    next_be = -1;
                }
                else if(next_be == 1){
                    strcpy(out_file, arg);
                    next_be = -1;
                }
                else{
                arg_list[num_arg] = (char * )malloc(sizeof(char) * (strlen(arg)+1));
                strcpy(arg_list[num_arg],arg);
                num_arg++;
                }
            }
            head = 0;
            begin_arg = 0;
            continue;
        }
        else if(comm[i] == '<'){
            
            if(begin_arg){
                arg[head] = 0;
                if(next_be == 0){
                    strcpy(in_file, arg);
                    next_be = -1;
                }
                else if(next_be == 1){
                    strcpy(out_file, arg);
                    next_be = -1;
                }
                else{
                arg_list[num_arg] = (char * )malloc(sizeof(char) * (strlen(arg)+1));
                strcpy(arg_list[num_arg],arg);
                num_arg++;
                }
            }

            next_be = 0;
            head = 0;
            begin_arg = 0;
            input_f = 1;
        }
        else if(comm[i] == '>'){
            if(begin_arg){
                arg[head] = 0;
                if(next_be == 0){
                    strcpy(in_file, arg);
                    next_be = -1;
                }
                else if(next_be == 1){
                    strcpy(out_file, arg);
                    next_be = -1;
                }
                else{
                arg_list[num_arg] = (char * )malloc(sizeof(char) * (strlen(arg)+1));
                strcpy(arg_list[num_arg],arg);
                num_arg++;
                }
            }

            if(comm[i+1] == '>'){
                next_be = 1;
                *append = 1;
                output_fa = 1;
            }
            else{
                next_be = 1;
                output_f = 1;
            }
            
            head = 0;
            begin_arg = 0;
        }
        else{
            if(!begin_arg){
                begin_arg = 1;
            }
            arg[head++] = comm[i];
            if(i == comm_size - 1){
                arg[head] = 0;
                if(next_be == 0){
                    strcpy(in_file, arg);
                }
                else if(next_be == 1){
                    strcpy(out_file, arg);
                }
                else{
                arg_list[num_arg] = (char * )malloc(sizeof(char) * (strlen(arg)+1));
                strcpy(arg_list[num_arg],arg);
                num_arg++;
                }
            }
        }
    }
    arg_list[num_arg] = NULL;
    // for(int i=0;i<num_arg;i++){
    //     printf("%s\n", arg_list[i]);
    // }
    // printf("\n\n");
    // printf("%s %s\n", out_file, in_file);
    return num_arg;
}


// fd dup call
// return 0 on error

int make_files_the_stds(char * out_file, char * in_file, int *append){
    int out, in;
    if(in_file[0] != 0){
        in = open(in_file,O_RDONLY);
        if(in == -1){
            printf("No such input file exists\n");
            return 0;
        }
        dup2(in,0);
        close(in);
    }
    if(out_file[0] != 0){
        if(*append){
        out = open(out_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
        }
        else{
        out = open(out_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        }
        dup2(out,1);
        close(out);
    }
    return 1;
}

void free_the_args(char * arg_list[], int num_arg){
    for(int i=1;i<num_arg-1;i++){
        free(arg_list[i]);
    }
}

void restore_the_stdio(int * stdio){
    dup2(stdio[0], 0);
    dup2(stdio[1], 1);
    close(stdio[0]);
    close(stdio[1]);
}

void save_the_stdio(int * stdio){
    stdio[0] = dup(0);
    stdio[1] = dup(1);
}