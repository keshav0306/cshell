#include "include.h"

int handle_echo(char * comm,int *fds){
    INITIALIZE_COMMAND("echo");
    for(int i=1;i<num_arg;i++){
        printf("%s ", arg_list[i]);
    }
    printf("\n");
    // char buff[30];
    // scanf("%s", buff);
    // printf("%s", buff);
    FREE_COMMAND
    return 1;
}