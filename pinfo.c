#ifndef __APPLE__
#include "include.h"
int handle_pinfo(char ** comm){
    char * pid_c = strtok_r(NULL," \t", comm);
    char fields[4][PROC_FIELD_SIZE] = {"pid", "process status", "memory", "executable path"};
    int pid_i;
    FILE * fp,*fp2 ,* fp3;
    char path[PATH_LEN] = "/proc/";
    char exe_path[PATH_LEN] = "/proc/";
    char stat_path[PATH_LEN] = "/proc/";
    if(*comm != NULL && strlen(*comm) != 0){
    	printf("Invalid Syntax");
    	goto error;
    }
    if(pid_c != NULL && strlen(pid_c) != 0){
        pid_i = atoi(pid_c);
        if(pid_i == 0){
            printf("Please enter a valid pid\n");
            goto error;
        }
        strcat(path, pid_c);
        strcat(exe_path, pid_c);
        strcat(stat_path, pid_c);
         // printf("%s\n%s\n", path,exe_path);
        strcat(path, "/status");
        strcat(exe_path, "/exe");
        strcat(stat_path, "/stat");
          //printf("%s\n%s\n", path,exe_path);
        fp = fopen(path,"r");
        fp2 = fopen(exe_path,"r");
        fp3 = fopen(stat_path, "r");
    }
    else{
        strcat(path, "self/status");
        strcat(exe_path, "self/exe");
        strcat(stat_path, "self/stat");
        fp = fopen(path,"r");   
        fp2 = fopen(exe_path,"r");   
        fp3 = fopen(stat_path,"r");
    }
     if(fp == NULL){
     	printf("No such process exists for the given pid\n");
     	goto error;   	
     }

    char arr[4][PROC_FIELD_SIZE];
    char temp[PROC_FIELD_SIZE];
    char * rest = NULL;
    char *dummy;
    char * dummy2;
    while(fgets(temp, PROC_FIELD_SIZE, fp)!=NULL){
  //printf("%s",temp);
        dummy = strtok_r(temp, ":", &rest);
//      printf("%s\n", rest); 
        rest = strtok_r(rest, " \t", &dummy2);
        if(strcmp(dummy, "Pid") == 0){
        
            strcpy(arr[0], rest);
        }
        else if(strcmp(dummy, "State") == 0){
            strcpy(arr[1], rest);
        }
        else if(strcmp(dummy, "VmSize") == 0){
            strcpy(arr[2], rest);
            break;
        }
    }
    fgets(temp, PROC_FIELD_SIZE, fp3);
    dummy = strtok_r(temp, " \t", &dummy2);
    char pgid[10], tpgid[10];
    for(int i=0;i<8;i++){
    	//printf("%s\n", dummy);
    	if(i == 4){
    	strcpy(pgid, dummy);
    	}
    	if(i==7){
    	strcpy(tpgid, dummy);
    	break;
    	}
        dummy = strtok_r(NULL, " \t", &dummy2);
    }
    //printf("%s %s\n",pgid, tpgid);
    if(strcmp(pgid, tpgid) == 0){
    strcat(arr[1], "+");
    }
    int char_count = readlink(exe_path, temp, PROC_FIELD_SIZE);
    if(char_count == -1){
        goto error;
    }
    else{
        temp[char_count] = 0;
        strcpy(arr[3], temp);
    }
    printf("%s: %s",fields[0],arr[0]);
    for(int i=1;i<4;i++){
        printf("%s: %s\n",fields[i],arr[i]);
    }
    fclose(fp);
    fclose(fp2);
    return 1;
    error:
    return 0;
}

#endif

