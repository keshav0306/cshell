#include "include.h"
#include "colour.h"

int str_compare(const void* a, const void* b){
    return strcmp(*(const char**)a, *(const char**)b);
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

    
    if(S_ISDIR(statty.st_mode)){
        blue();
    }
    else if(statty.st_mode & S_IXUSR){
        green();
    }
    else{
        white();
    }

    if(link_or_not != -1){
        printf("%s %3d %s %s %ld %s %s -> %s\n", perms,statty.st_nlink,getpwuid(statty.st_uid)->pw_name,getgrgid(statty.st_gid)->gr_name,size,time_display + 4, name, link);
    }
    else{
        printf("%s %3d %s %s %ld %s %s\n", perms,statty.st_nlink,getpwuid(statty.st_uid)->pw_name,getgrgid(statty.st_gid)->gr_name,size,time_display + 4, name);
    }
    reset_colour();
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
                printf("\n");
            }
            else{
                printf("%s", dir);
                printf("\n");
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
            arr[num_file++] = (char* )malloc(sizeof(char) * (strlen(ent->d_name)+1));
            strcpy(arr[num_file - 1],ent->d_name);
        }
        else if(a_flag == 1){
            arr[num_file++] = (char* )malloc(sizeof(char) * (strlen(ent->d_name)+1));
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
        printf("\n");
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
        printf("\n");
    }
    for(int i=0;i<num_file;i++){
        free(arr[i]);
    }
    free(arr);
    closedir(dr);
    return 1;
}

int handle_ls(char * comm, char * init_path){
    INITIALIZE_COMMAND("ls");
    int a_flag = 0, l_flag = 0;
    // char * arg = strtok_r(NULL, " \t",comm);
    char ** dir_file = (char **) malloc(sizeof(char * ) * MAX_LS_ARGS);
    int file_no = 0;
    int i=1;
    while(arg_list[i] != NULL){
    if(strcmp(arg_list[i], "-l") == 0){
        l_flag = 1;
    }
    else if(strcmp(arg_list[i], "-a") == 0){
        a_flag = 1;
    }
    else if(strcmp(arg_list[i], "-al") == 0){
        l_flag = 1;
        a_flag = 1;
    }
    else if(strcmp(arg_list[i], "-la") == 0){
        l_flag = 1;
        a_flag = 1;
    }
    else{
        dir_file[file_no++] = (char *)malloc(sizeof(char) * (strlen(arg_list[i])+1));
        strcpy(dir_file[file_no - 1], arg_list[i]);
    }
    i++;
    // arg = strtok_r(NULL, " \t",comm);
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
                FREE_COMMAND
                printf("%s : No such file or directory\n", dir_file[i]);
                return 0;
            }
        }
    }
    for(int i=0;i<file_no;i++){
        free(dir_file[i]);
    }
    free(dir_file);
    FREE_COMMAND
}

int print_for_tab(char * file_name, char * out_name, int * file_or_dir){
    DIR * dr = opendir(".");
    struct dirent * ent = readdir(dr);
    char ** arr = (char **) malloc(sizeof(char *) * MAX_NO_OF_FILES);
    int num_file = 0;
    while(ent != NULL){
        if((ent->d_name)[0] != '.'){
            arr[num_file++] = (char* )malloc(sizeof(char) * (strlen(ent->d_name)+1));
            strcpy(arr[num_file - 1],ent->d_name);
        }
        ent = readdir(dr);
    }
    int possible = 0;

    for(int i=0;i<num_file;i++){
        if(strncmp(file_name, arr[i], strlen(file_name)) == 0){
            possible++;
        }
    }

    if(possible > 1){
        printf("\n");
        for(int i=0;i<num_file;i++){
        if(strncmp(file_name, arr[i], strlen(file_name)) == 0){
            printf("%s\n", arr[i]);
        }
    }
    }
    else if(possible == 1){
        for(int i=0;i<num_file;i++){
        if(strncmp(file_name, arr[i], strlen(file_name)) == 0){
            strcpy(out_name, arr[i]);
             DIR * dr2 = opendir(arr[i]);
            if(dr2 == NULL){
                *file_or_dir = 0;
            }
            else{
                *file_or_dir = 1;
                closedir(dr2);
            }
            
        }
    }
    }
    for(int i=0;i<num_file;i++){
        free(arr[i]);
    }
    free(arr);
    closedir(dr);
    return possible;
}