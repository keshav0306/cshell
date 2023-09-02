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
#include "command.h"


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

#define INITIALIZE_COMMAND(x) \
    char command[COMMAND_LEN] = x ;\
    char * arg_list[MAX_ARGS_FOR_SYS_PROCESS];\
    char out_file[FILENAME_MAX];\
    char in_file[FILENAME_MAX];\
    int app;\
    int * append = &app;\
    int stdio[2];\
    save_the_stdio(stdio);\
    int num_arg = io_redirect(comm, command, arg_list, out_file, in_file, append);\
    int ret = make_files_the_stds(out_file, in_file, append);\
    if(!ret){\
        free_the_args(arg_list,num_arg);\
        restore_the_stdio(stdio);\
        return 0;\
    }\

#define FREE_COMMAND\
    free_the_args(arg_list,num_arg);\
    restore_the_stdio(stdio);
