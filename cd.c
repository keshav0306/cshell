#include "include.h"

int cd_main(char * path, char * out_path, char * init_path, int size){

    char curr_dir[PATH_LEN];
    int ch = chdir(path);
    if(ch < 0){
        return -1;
    }
    getcwd(curr_dir,PATH_LEN);
    int a = strncmp(curr_dir, init_path, size);
    if(a == 0){
        if(size == strlen(curr_dir)){
            sprintf(out_path, "~");
        }
        else{
            sprintf(out_path, "~/%s", curr_dir + size + 1);
        }
    }
    else{
        sprintf(out_path, "%s", curr_dir);
    }
    return 0;
}

int handle_cd(char * comm, char *path, char * init_path, char * old_path, int size){
    INITIALIZE_COMMAND("cd");
    int cd;
    // printf("%s", arg_list[1]);
    if(arg_list[1] == NULL){
        getcwd(old_path,PATH_LEN);
        cd_main(init_path, path, init_path, size); 
        return 1;
    }
    
    if(num_arg != 2){
        FREE_COMMAND
        printf("cd doesn't accept more than 1 argument\n");
        return 0;
    }
   
    else if(strcmp(arg_list[1], "-") == 0){
        printf("%s\n", old_path);
        char temp_path[PATH_LEN];
        getcwd(temp_path,PATH_LEN);
        cd = cd_main(old_path, path, init_path, size); 
        if(cd == -1){
            FREE_COMMAND
            printf("No such directory exists\n");
        }
        strcpy(old_path, temp_path);
        if(cd != -1){
            FREE_COMMAND
            return 1;
        }
        return 0;
    }
    getcwd(old_path,PATH_LEN);
    char second[PATH_LEN];
    strcpy(second, init_path);
    if(arg_list[1][0] == '~' && (arg_list[1][1] == 0 || arg_list[1][1] == '/')){
        strcat(second, arg_list[1] + 1);
        cd = cd_main(second, path, init_path, size); 
        if(cd == -1){
            FREE_COMMAND
            printf("No such directory exists\n");
            return 0;
        }
    }
    else{
        cd = cd_main(arg_list[1], path, init_path, size); 
        if(cd == -1){
            FREE_COMMAND
            printf("No such directory exists\n");
            return 0;
        }
    }
}