#include "include.h"

int handle_pwd(char * comm){
    INITIALIZE_COMMAND("pwd");
    char curr_dir[PATH_LEN];
    if(num_arg != 1){
        goto error;
    }
    printf("%s\n", getcwd(curr_dir,PATH_LEN));
    FREE_COMMAND
    return 1;
    error:
    printf("pwd cant have any arguments\n");
    return 0;
}