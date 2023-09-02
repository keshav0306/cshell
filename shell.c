#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <grp.h>
#ifdef __APPLE__
#include <sys/xattr.h>
#include <sys/acl.h>
#endif
#include <signal.h>

#define SYS_NAME_LEN 100
#define PATH_LEN PATH_MAX
#define COMMAND_LEN 200
#define MAX_LS_ARGS 50
#define MAX_NO_OF_FILES 200
#define LINK_BUFF 200
#define TIME_LEN 30
#ifdef __APPLE__
#define HALF_YEAR_IN_SECONDS 15768000
#endif
#define MAX_ARGS_FOR_SYS_PROCESS 20
#define MAX_BG_PROCESSES 20
#define ARG_LEN 20
#define PROC_FIELD_SIZE 100

struct bg{
    char * command;
    pid_t pid;
};

struct bg ** bground;
pid_t curr_fg = 0;

void initialize_bground(){
    bground = (struct bg ** )malloc(sizeof(struct bg * ) * MAX_BG_PROCESSES);
    for(int i=0;i<MAX_BG_PROCESSES;i++){
        bground[i] = NULL;
    }
}

int put_into_bground(pid_t pid, char * command){

    for(int i=0;i<MAX_BG_PROCESSES;i++){
        if(bground[i] == NULL){
            bground[i] = (struct bg *)malloc(sizeof(struct bg));
            bground[i]->command = (char *)malloc(sizeof(char) * (strlen(command) + 1));
            strcpy(bground[i]->command, command);
            bground[i]->pid = pid;
            return i + 1;
        }
    }
}


char* permish(struct stat sb, char * path){               // function used for displaying the permissions of the file/directory
    #ifdef __APPLE__
        char * perm = (char* )malloc(sizeof(char) * 12);
    #else
        char * perm = (char* )malloc(sizeof(char) * 11);
    #endif

    if(S_ISBLK(sb.st_mode)){
        perm[0] = 'b';
    }
    else if(S_ISCHR(sb.st_mode)){
        perm[0] = 'c';
    }
    else if(S_ISDIR(sb.st_mode)){
        perm[0] = 'd';
    }
    else if(S_ISSOCK(sb.st_mode)){
        perm[0] = 's';
    }
#ifdef __APPLE__
    else if(S_ISWHT(sb.st_mode)){
        perm[0] = 'w';
    }
#endif
    else if(S_ISLNK(sb.st_mode)){
        perm[0] = 'l';
    }
    else if(S_ISFIFO(sb.st_mode)){
        perm[0] = 'p';
    }
    else if(S_ISREG(sb.st_mode)){
        perm[0] = '-';
    }

    if((sb.st_mode & S_IRUSR) == 1 << 8){
        perm[1] = 'r';
    }
    else{
        perm[1] = '-';
    }

    if((sb.st_mode & S_IWUSR) == 1 << 7){
        perm[2] = 'w';
    }
    else{
        perm[2] = '-';
    }

    if((sb.st_mode & S_IXUSR) == 1 << 6){
        perm[3] = 'x';
    }
    else{
        perm[3] = '-';
    }

    if((sb.st_mode & S_IRGRP) == 1 << 5){
        perm[4] = 'r';    
    }
    else{
        perm[4] = '-';
    }

    if((sb.st_mode & S_IWGRP) == 1 << 4){
        perm[5] = 'w';
    }
    else{
        perm[5] = '-';
    }

    if((sb.st_mode & S_IXGRP) == 1 << 3){
        perm[6] = 'x';
    }
    else{
        perm[6] = '-';
    }

    if((sb.st_mode & S_IROTH) == 1 << 2){
        perm[7] = 'r';
    }
    else{
        perm[7] = '-';
    }

    if((sb.st_mode & S_IWOTH) == 1 << 1){
        perm[8] = 'w';
    }
    else{
        perm[8] = '-';
    }

    if((sb.st_mode & S_IXOTH) == 1 << 0){
        perm[9] = 'x';
    }
    else{
        perm[9] = '-';
    }
    
    #ifdef __APPLE__

    int ret = listxattr(path, NULL, 0, XATTR_NOFOLLOW);
    acl_t acl = acl_get_link_np(path, ACL_TYPE_EXTENDED);
    acl_entry_t dummy;

    if (acl && acl_get_entry(acl, ACL_FIRST_ENTRY, &dummy) == -1) {
        acl_free(acl);
        acl = NULL;
    }
    if(ret > 0){
        perm[10] = '@';
    }
    else if(acl != NULL){
        perm[10] = '+';
    }
    else{
        perm[10] = ' ';
    }

    perm[11] = 0;
    #else
    perm[10] = 0;
    #endif

    return perm;
}
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

int handle_cd(char ** comm, char *path, char * init_path, char * old_path, int size){
    int cd;
    char * where = strtok_r(NULL, " \t",comm);
    char * second_arg = strtok_r(NULL, " \t",comm);
    if(second_arg){
        printf("cd doesn't accept more than 1 argument\n");
        goto done;
    }
    if(where == NULL){
        getcwd(old_path,PATH_LEN);
        cd_main(init_path, path, init_path, size); 
       goto done;
    }
    else if(strcmp(where, "-") == 0){
        printf("%s\n", old_path);
        char temp_path[PATH_LEN];
        getcwd(temp_path,PATH_LEN);
        cd = cd_main(old_path, path, init_path, size); 
        if(cd == -1){
            printf("No such directory exists\n");
        }
        strcpy(old_path, temp_path);
        goto done;
    }
    getcwd(old_path,PATH_LEN);
    char second[PATH_LEN];
    strcpy(second, init_path);
    if(where[0] == '~' && (where[1] == 0 || where[1] == '/')){
        strcat(second, where + 1);
        cd = cd_main(second, path, init_path, size); 
        if(cd == -1){
            printf("No such directory exists\n");
            goto done;
        }
    }
    else{
        cd = cd_main(where, path, init_path, size); 
        if(cd == -1){
            printf("No such directory exists\n");
            goto done;
        }
    }

    return 1;

    done:
    return 0;
}

int str_compare(const void* a, const void* b){
    return strcmp(*(const char**)a, *(const char**)b);
}
 

int handle_pwd(char ** comm){
    char curr_dir[PATH_LEN];
    char * second_arg = strtok_r(NULL, " \t",comm);
    if(second_arg){
        printf("pwd cant have any arguments\n");
        return 0;
    }
    printf("%s\n", getcwd(curr_dir,PATH_LEN));
    return 1;
}

int handle_echo(char ** comm){
    //printf("%s\n", *comm);
    char * next;
    while(next != NULL){
        next = strtok_r(NULL, " \t", comm);
        if(next)
        printf("%s ", next);
    }
    printf("\n");
    return 1;
}

int ls_l(char * dir, int dir_len, char * name, int flag){
    struct stat statty; 
    char concat_path[PATH_LEN];
    strcpy(concat_path,dir);
    concat_path[dir_len] = '/';
    concat_path[dir_len + 1] = 0;
    strcat(concat_path, name);
    int link_or_not;
    char * perms;
    char link[LINK_BUFF];
    if(flag){
        lstat(concat_path, &statty);
        perms = permish(statty, concat_path);
        link_or_not = readlink(concat_path,link,LINK_BUFF);
    }
    else{
        lstat(name, &statty);
        perms = permish(statty, name);
        link_or_not = readlink(name,link,LINK_BUFF);
    }
    
    char * times = ctime(&statty.st_mtime);
    // char * time_display = (char * )malloc(sizeof(char) * TIME_LEN);
    char time_display[TIME_LEN];
    strcpy(time_display, times);
    time_t seconds = time(NULL);
    
    #ifdef HALF_YEAR_IN_SECONDS
    if(seconds - statty.st_ctimespec.tv_sec >= HALF_YEAR_IN_SECONDS){
        char buff[6];
        strcpy(buff, times + strlen(times) - 5);
        time_display[strlen(times) - 14] = 0;
        strcat(time_display, buff);
        // time_display += 4;
    }
    else{
        time_display[strlen(times) - 9] = 0;
        // time_display += 4;
    }
    #else
        time_display[strlen(times) - 9] = 0;
        // time_display += 4;
    #endif

    int dev = statty.st_rdev;
    long size = statty.st_size;
    if(S_ISBLK(statty.st_mode) || S_ISCHR(statty.st_mode)){
        size = dev;
    }

    if(link_or_not != -1){
        printf("%s %3d %s %s %ld %s %s -> %s\n", perms,statty.st_nlink,getpwuid(statty.st_uid)->pw_name,getgrgid(statty.st_gid)->gr_name,size,time_display + 4, name, link);
    }
    else{
        printf("%s %3d %s %s %ld %s %s\n", perms,statty.st_nlink,getpwuid(statty.st_uid)->pw_name,getgrgid(statty.st_gid)->gr_name,size,time_display + 4, name);
    }
    free(perms);
}

int ls_main(char * dir, int a_flag, int l_flag){
    DIR * dr = opendir(dir);
    if(dr == NULL){
        struct stat statty; 
        int ret = lstat(dir, &statty);
        if(ret != -1){
            if(l_flag){
                ls_l(dir,strlen(dir), dir ,0);
            }
            else{
                printf("%s", dir);
            }
            return 1;
        }
        else{
            return 0;
        }
    }
    struct dirent * ent = readdir(dr);
    char ** arr = (char **) malloc(sizeof(char *) * MAX_NO_OF_FILES);
    int num_file = 0;
    while(ent != NULL){
        if((ent->d_name)[0] != '.' && a_flag == 0){
            arr[num_file++] = (char* )malloc(sizeof(char) * strlen(ent->d_name));
            strcpy(arr[num_file - 1],ent->d_name);
        }
        else if(a_flag == 1){
            arr[num_file++] = (char* )malloc(sizeof(char) * strlen(ent->d_name));
            strcpy(arr[num_file - 1],ent->d_name);
        }
        ent = readdir(dr);
    }
    qsort(arr, num_file, sizeof(const char*), str_compare);
    int dir_len = strlen(dir);
    if(l_flag == 0){
        for(int i=0;i<num_file;i++){
            printf("%s\n", arr[i]);
        }
    }
    else{
        long long sum = 0;
        for(int i=0;i<num_file;i++){
            struct stat statty;
            char concat_path[PATH_LEN];
            strcpy(concat_path,dir);
            concat_path[dir_len] = '/';
            concat_path[dir_len + 1] = 0;
            strcat(concat_path, arr[i]);
            lstat(concat_path, &statty);
            sum += statty.st_blocks;
        }
        printf("total %lld\n", sum);
        for(int i=0;i<num_file;i++){
            ls_l(dir,dir_len,arr[i],1);
        }
    }
    for(int i=0;i<num_file;i++){
        free(arr[i]);
    }
    free(arr);
    closedir(dr);
    return 1;
}

int handle_ls(char ** comm, char * init_path){
    int a_flag = 0, l_flag = 0;
    char * arg = strtok_r(NULL, " \t",comm);
    char ** dir_file = (char **) malloc(sizeof(char * ) * MAX_LS_ARGS);
    int file_no = 0;

    while(arg != NULL){
    if(strcmp(arg, "-l") == 0){
        l_flag = 1;
    }
    else if(strcmp(arg, "-a") == 0){
        a_flag = 1;
    }
    else if(strcmp(arg, "-al") == 0){
        l_flag = 1;
        a_flag = 1;
    }
    else if(strcmp(arg, "-la") == 0){
        l_flag = 1;
        a_flag = 1;
    }
    else{
        dir_file[file_no++] = (char *)malloc(sizeof(char) * strlen(arg));
        strcpy(dir_file[file_no - 1], arg);
    }
    arg = strtok_r(NULL, " \t",comm);
    }

    if(file_no == 0){
        ls_main(".", a_flag, l_flag);
    }
    else{
        int ls;
        for(int i=0;i<file_no;i++){
            if(dir_file[i][0] == '~' && (dir_file[i][1] == '/' || dir_file[i][1] == 0)){
                char path[PATH_LEN];
                strcpy(path, init_path);
                strcat(path, dir_file[i] + 1);
                ls = ls_main(path, a_flag, l_flag);
            }
            else{
                ls = ls_main(dir_file[i], a_flag, l_flag);
            }
            if(!ls){
                printf("%s : No such file or directory\n", dir_file[i]);
            }
        }
    }
    for(int i=0;i<file_no;i++){
        free(dir_file[i]);
    }
    free(dir_file);
}

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
    int d_flag = -1, f_flag = -1;
    int d_given, f_given;
    char dir[PATH_LEN], file[PATH_LEN];
    char dummy[COMMAND_LEN] = " ";

    if(*comm != NULL){
        strcat(dummy, *comm);
        strcpy(*comm, dummy);
    }
    char * file_quotes = NULL;
    char * file_q = strtok_r(NULL,"\"", comm);

    char * left = NULL;
    char * arg;
    if(file_q == NULL || strlen(file_q) == 0){
        goto right;
    }
    arg = strtok_r(file_q, " \t", &left);

    if(arg == NULL || strlen(arg) == 0){
        d_flag = 1;
        f_flag = 1;
        d_given = 0;
        printf("Sdfa\n");
        goto right;
    }
    else if(strcmp(arg, "-d") == 0){
        d_flag = 1;
        d_given = 0;
        arg = strtok_r(NULL, " \t", &left);
        if(arg != NULL && strcmp(arg,"-f") == 0 && strlen(arg)!=0){
            f_flag = 1;
        }
        else if(arg != NULL && strcmp(arg,"-f") != 0 && strlen(arg)!=0 ){
            goto error;
        }
    }
    else if(strcmp(arg,"-f") == 0){
        d_given = 0;
        f_flag = 1;
        arg = strtok_r(NULL, " \t", &left);
        if(arg != NULL && strcmp(arg,"-d") == 0 && strlen(arg)!=0 ){
            d_flag = 1;
        }
        else if(arg != NULL && strcmp(arg,"-d") != 0 && strlen(arg)!=0 ){
            goto error;
        }
    }
    else{
        d_given = 1;
        strcpy(dir, arg);
        arg = strtok_r(NULL, " \t", &left);
        if(arg == NULL || strlen(arg) == 0){
            goto right;
        }
        else if(strcmp(arg, "-d") == 0){
            d_flag = 1;
            arg = strtok_r(NULL, " \t", &left);
            if( arg != NULL && strcmp(arg,"-f") == 0 && strlen(arg)!=0){
                f_flag = 1;
            }
            else if(arg != NULL && strcmp(arg,"-f") != 0 && strlen(arg)!=0 ){
                goto error;
            }
            arg = strtok_r(NULL, " \t", &left);
            if(arg != NULL && strlen(arg)!=0 ){
                goto error;
            }
        }
        else if(strcmp(arg,"-f") == 0){
            f_flag = 1;
            arg = strtok_r(NULL, " \t", &left);
            if(arg != NULL && strcmp(arg,"-d") == 0 && strlen(arg)!=0){
                d_flag = 1;
            }
            else if(arg != NULL && strcmp(arg,"-d") != 0 && strlen(arg)!=0 ){
                goto error;
            }
        }
        else{
            goto error;
        }
    }

    right:
    if(*comm != NULL && strlen(*comm) != 0){
        file_q = strtok_r(*comm,"\"", &file_quotes);
        if(file_quotes == NULL){
            goto error;
        }
        if(file_q != NULL){
        strcpy(file,file_q);
        f_given = 1;
        file_q = strtok_r(file_quotes, " \t", &left);
        if(file_q!=NULL){
            goto error;
        }
        }
    }
    else{
        f_given = 0;
    }

    if(d_flag == 1 && f_flag != 1){
        f_flag = 0;
    }
    else if(d_flag != 1 && f_flag == 1){
        d_flag = 0;
    }
    else if(d_flag != 1 && f_flag != 1){
        d_flag = 1;
        f_flag = 1;
    }
    if(!d_given){
        strcpy(dir, ".");
    }
    DIR * dr; 
    char act_path[PATH_LEN];
    char path_printed[PATH_LEN];

    if(dir[0] == '~' && (dir[1] == '/' || dir[1] == 0)){
        char path[PATH_LEN];
        strcpy(path, init_path);
        strcat(path, "/");
        strcat(path, dir + 1);
        dr = opendir(path);
        strcpy(act_path,path);
        strcpy(path_printed,dir);
        printf("%s\n%s\n", act_path,path_printed);
    }
    else{
        dr = opendir(dir);
        if(dr == NULL){
            goto error2;
        }
        strcpy(act_path,dir);
        strcpy(path_printed,dir);
    }
    if(dr != NULL){
        discover_main(act_path,path_printed,(f_given) ? file : NULL, d_flag, f_flag, dr);
    }
    closedir(dr);
    return 1;
    error:
    printf("Invalid Syntax\n");
    return 0;
    error2:
    printf("Directory doesnt exist\n");
    return 0;
}

void bg_child(){
    pid_t pid;
    ((pid = wait(NULL)));
    if(pid == curr_fg){
        goto end;
    }
    for(int i=0;i<MAX_BG_PROCESSES;i++){
        if(bground[i] == NULL){
            continue;
        }
        else{
            if(bground[i]->pid == pid){
                fprintf(stdout,"%s with pid %d exited normally\n", bground[i]->command, pid);
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

#ifndef __APPLE__

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

int main(){
    initialize_bground();
    signal(SIGCHLD, bg_child);
    //signal(SIGINT, handle_history);
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

    while(1){
        printf("<%s@%s:%s> ", pass->pw_name, name, path);
        char arr[COMMAND_LEN];
        fflush(NULL);
        FILE * fp = fopen(his_path, "r");
        fgets(arr, COMMAND_LEN, stdin);
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
        fflush(NULL);
        char * kot = NULL;
        arr[strlen(arr) - 1] = 0;
        char * tok = strtok_r(arr,";",&kot);
        if(tok == NULL){
            continue;
        }
        while(tok != NULL){
            //   printf("\n%s %d\n" ,tok,strlen(tok));
            int bg = 1;
            char * and = NULL;  
            char * and_comm = strtok_r(tok, "&", &and);
            // printf("%s %d\n",and ,strlen(and));
            if(and  == NULL){
                bg = 0;
            }
            // printf("\n%s %d\n",and_comm, strlen(and_comm));
            while(and_comm != NULL){
                // printf("%s" ,and_comm);
                char * comm = NULL; 
                char * command = strtok_r(and_comm, " \t",&comm);   // comm contains the rest of the string
                //printf("%s\n",command);
                if(!command ){
                    goto end;
                }
                if(strcmp(command, "echo") == 0){
                    int echo = handle_echo(&comm);
                }
                else if(strcmp(command, "pwd") == 0){
                    int pwd = handle_pwd(&comm);
                    if(!pwd){
                        goto end;
                    }
                }
                else if(strcmp(command, "cd") == 0){
                    int cd = handle_cd(&comm,path,init_path,old_path,size);
                    if(!cd){
                        goto end;
                    }
                }
                else if(strcmp(command, "ls") == 0){
                    int ls = handle_ls(&comm, init_path);
                    if(!ls){
                        goto end;
                    }
                }
                else if(strcmp(command, "discover") == 0){
                    int discover = handle_discover(&comm, init_path);
                    if(!discover){
                        goto end;
                    }
                }
                else if(strcmp(command, "history") == 0){
                    int smaller = (history_count > 9) ? 9 : history_count;
                    for(int i=smaller;i>=0;i--){
                        printf("%s", buff[i]);
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
                    // printf("%s\n", comm);
                    char * arg_list[MAX_ARGS_FOR_SYS_PROCESS];
                    int ret = 0;
                    //arg_list[0] = (char * )malloc(sizeof(char) * (strlen(command)+1));
                    arg_list[0] = command;
                    int num_arg = 1;
                    while(comm != NULL){
                        char * arg = strtok_r(NULL, " \t",&comm);
                        if(arg == NULL){
                            break;
                        }
                        arg_list[num_arg] = (char * )malloc(sizeof(char) * (strlen(arg)+1));
                        strcpy(arg_list[num_arg],arg);
                        num_arg++;
                    }
                    arg_list[num_arg] = NULL;

                    pid_t pid = fork();
                    if(pid == 0){
                        ret = execvp(command, arg_list);
                        if(ret == -1){
                            printf("No such command!\n");
                        }
                        return 0;
                    }
                    else{
                        if(!bg){
                            curr_fg = pid;
                            waitpid(pid,NULL,0);
                        }
                        else{
                            int position = put_into_bground(pid, command);
                            printf("[%d] %d\n", position, pid);
                        }
                    }
                    for(int i=1;i<num_arg-1;i++){
                        free(arg_list[i]);
                    }
                    goto end;
                }
                end:
                //printf("%d\n", bg);
                and_comm = strtok_r(NULL, "&", &and);
                //printf("\n%s %d\n", and_comm,strlen(and_comm));
                if(and  == NULL){
                    bg = 0;
                }
            }
            tok = strtok_r(NULL, ";",&kot);
        }
     }
}
