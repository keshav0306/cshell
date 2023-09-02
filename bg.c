#include "include.h"
#include "bg.h"

extern int global_fg_bg_flag;
struct bg ** bground;
pid_t curr_fg = 0;

int bg_compare(const void* a, const void* b){
    char * str1 = (char*)a;
    char * str2 = (char*)b;
    return strcmp(str1,str2);
}

void initialize_bground(){
    bground = (struct bg ** )malloc(sizeof(struct bg * ) * MAX_BG_PROCESSES);
    for(int i=0;i<MAX_BG_PROCESSES;i++){
        bground[i] = NULL;
    }
}

int put_into_bground(pid_t pid, char * command, int rs){
    for(int i=0;i<MAX_BG_PROCESSES;i++){
        if(bground[i] == NULL){
            bground[i] = (struct bg *)malloc(sizeof(struct bg));
            bground[i]->command = (char *)malloc(sizeof(char) * (strlen(command) + 1));
            strcpy(bground[i]->command, command);
            bground[i]->pid = pid;
            bground[i]->r_s = rs;
            bground[i]->position = i + 1;
            return i + 1;
        }
    }
}

void bg_child(){
    // printf("I am here\n");
    pid_t pid;
    int status;
    if(global_fg_bg_flag){
        global_fg_bg_flag = 0;
        goto end;
    }
    ((pid = waitpid(-1, &status, WNOHANG)));
    // printf("%d\n", pid);
    
    if(pid == curr_fg){
        // printf("was running in fg and in bg_child\n");
        goto end;
    }

    for(int i=0;i<MAX_BG_PROCESSES;i++){
        if(bground[i] == NULL){
            continue;
        }
        else{
            if(bground[i]->pid == pid){
                fprintf(stdout,"%s with pid %d exited %s\n", bground[i]->command, pid, WIFEXITED(status) ? "normally": "abnormally");
                free(bground[i]->command);
                free(bground[i]);
                bground[i] = NULL;
                break;
            }
            
        }
    }
    end:
     printf("");

}

int handle_jobs(char * comm){
    INITIALIZE_COMMAND("jobs");
    int s_flag = 0, r_flag = 0, err = 0;
    for(int i=1;i<num_arg;i++){
        if(strcmp(arg_list[i],"-r") == 0){
            r_flag = 1;
        }
        else if(strcmp(arg_list[i],"-s") == 0){
            s_flag = 1;
        }
        else{
            err = 1;
        }
    }
    if(!err){
    if(r_flag == 0 && s_flag == 0){
        r_flag = 1;
        s_flag = 1;
    }
    
    struct bg bg[MAX_BG_PROCESSES];
    int num = 0;
    for(int i=0;i<MAX_BG_PROCESSES;i++){
        if(bground[i] != NULL){
            bg[num].command = bground[i]->command;
            bg[num].pid = bground[i]->pid;
            bg[num].position = bground[i]->position;
            bg[num].r_s = bground[i]->r_s;
            num++;
        }
    }
    qsort(bg, num, sizeof(struct bg), bg_compare);
    for(int i=0;i<num;i++){
        if(bg[i].r_s == 1 && r_flag == 1)
        printf("[%d] Running %s [%d] \n", bg[i].position ,bg[i].command, bg[i].pid);
        else if(bg[i].r_s == 0 && s_flag == 1){
        printf("[%d] Stopped %s [%d] \n", bg[i].position , bg[i].command, bg[i].pid);
        }
    }
    }

    FREE_COMMAND
    
    if(err){
        printf("Invalid args\n");   // need to handle for pipe
    }
    
}

int handle_sig(char * comm){
    INITIALIZE_COMMAND("sig");
    if(num_arg!=3){
        printf("Please enter valid number of args\n");
        goto end;
    }
    int positon = strtol(arg_list[1],NULL,0), signum = strtol(arg_list[2],NULL,0), err = 0;
    if(positon == 0 || signum == 0 || num_arg != 3){
        printf("Invalid args\n");
        err = 1;
    }
    if(!err){
    if(bground[positon-1] == NULL){
        printf("No job exists at this postion\n");
    }
    else{
        kill(bground[positon-1]->pid, signum);
    }
    }
    FREE_COMMAND
    end:
    return 1;
}

int handle_fg(char * comm){
    INITIALIZE_COMMAND("fg");
    if(arg_list[1] == NULL){
        printf("Invalid args\n");
        goto end;
    }
    int position = strtol(arg_list[1],NULL,0);
    char dum_comm[COMMAND_LEN];
    if(num_arg >=3 || position == 0){
        printf("Invalid args\n");
        return 0;
    }
    if(bground[position-1] != NULL){
        tcsetpgrp(STDIN_FILENO,bground[position-1]->pid);
        curr_fg = bground[position-1]->pid;
        //if(!bground[position-1]->r_s){
            // printf("before signal\n");
            kill(bground[position - 1]->pid, SIGCONT);
            // printf("after signal\n");
        //}
        printf("Moved %s job to foreground\n",bground[position-1]->command);
        strcpy(dum_comm, bground[position-1]->command);
            free(bground[position-1]->command);
            free(bground[position-1]);
            bground[position-1] = NULL;
        
        int status;
        
        waitpid(curr_fg, &status, WUNTRACED);
        tcsetpgrp(STDIN_FILENO, getpid());
        
        if(WIFSTOPPED(status)){
            put_into_bground(curr_fg, dum_comm ,0);
        }
        curr_fg = 0;
    }
    else{
        printf("No such job\n");
    }
    FREE_COMMAND
    end:
    return 1;
}

int handle_bg(char * comm){
    INITIALIZE_COMMAND("bg")
    int position;
    if(arg_list[1] != NULL){
        position = strtol(arg_list[1],NULL,0);
    }
    else{
        printf("Please specify arguements\n");
        goto end;
    }
    if(num_arg >=3 || position == 0){
        printf("Invalid args\n");
        goto end;
    }
    if(kill(bground[position - 1]->pid, SIGCONT) < 0){
        printf("Counld not run in background\n");
        goto end;
    }
    else{
        printf("Running %s in background", bground[position - 1]->command);
    }
    bground[position-1]->r_s = 1;
    FREE_COMMAND
    end: 
    return 1;
}