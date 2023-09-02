#include "include.h"

int discover_main(char *act_path,char* path_printed, char * file, int d_flag, int f_flag, DIR* dr){
    struct dirent * ent = readdir(dr);
    while(ent != NULL){
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0){
            goto end;
        }
        char new_act_path[PATH_LEN];
        strcpy(new_act_path,act_path);
        strcat(new_act_path, "/");
        strcat(new_act_path, ent->d_name);

        char new_path_printed[PATH_LEN];
        strcpy(new_path_printed,path_printed);
        strcat(new_path_printed, "/");
        strcat(new_path_printed, ent->d_name);
        
        DIR * dr2 = opendir(new_act_path);
        if(dr2 == NULL && f_flag){
            if(file != NULL){
                if(strcmp(ent->d_name, file) == 0){
                    printf("%s\n", new_path_printed);
                }
            }
            else{
                printf("%s\n", new_path_printed);
            }
        }
        else if(dr2 != NULL){
            if(d_flag){
            if(file != NULL){
                if(strcmp(ent->d_name, file) == 0){
                    printf("%s\n", new_path_printed);
                }
            }
            else{
                printf("%s\n", new_path_printed);
            }
            }
            discover_main(new_act_path,new_path_printed,file,d_flag,f_flag,dr2);
            closedir(dr2);
        }
        end:
        ent = readdir(dr);

    }
    return 0;
}

int handle_discover(char ** comm, char * init_path){
     
char * arg1 = strtok_r(NULL," \t", comm);
    int d_flag = 0;
    int f_flag = 0;
    char dir[PATH_LEN];
    char file[PATH_LEN];
    int fp = 0, dp = 0;
    while(arg1 != NULL){
        if(fp){
            goto error;
        }
        if(arg1[0] == '"' && arg1[strlen(arg1)-1] == '"'){
            strcpy(file, arg1 + 1);
            file[strlen(arg1)-2] = 0;
            fp = 1;
            goto loop_end;
        }
        if(!fp){
            if(strcmp(arg1, "-d") == 0){
            d_flag = 1;
            goto loop_end;
            }
            else if(strcmp(arg1, "-f") == 0){
            f_flag = 1;
            goto loop_end;
            }
            if(!f_flag && !d_flag && !dp){
                strcpy(dir,arg1);
                dp = 1;
            }
            else{
                goto error;
            }
        }
        else{
            goto error;
        }
        
        loop_end:
        if(*comm == NULL || strlen(*comm) == 0){
            break;
        }
        
        arg1 = strtok_r(NULL," \t", comm);
    }
    char* file_t = NULL;
    if(!dp){
        strcpy(dir, ".");
    }
    if(fp){
        file_t = file;
    }
    if(!d_flag && !f_flag){
        d_flag = 1;
        f_flag = 1;
    }
    char act_path[PATH_LEN];
    if((dir[0] == '~' && dir[1] == 0 || dir[1] == '/')){
        strcpy(act_path, init_path);
        strcat(act_path, "/");
        strcat(act_path, dir + 1);
    }
    else{
        strcpy(act_path, dir);
    }
    DIR* dr = opendir(act_path);
    if(dr)
    discover_main(act_path, dir, file_t, d_flag , f_flag, dr);
    else{
        printf("Directory doesnt exist\n");
        return 0;
    }
    return 1;
    error:
    printf("Invalid Syntax\n");
    return 0;
}
