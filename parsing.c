#include <string.h>
#include <stdio.h>
#include "include.h"


int main(){
    char * comm = "  hello>>myname<isjop";
    char * command = "hello";
    char * arg_list[MAX_ARGS_FOR_SYS_PROCESS];
    char out_file[FILENAME_MAX];
    char in_file[FILENAME_MAX];
    int app;
    int * append = &app;
    io_redirect(comm, command ,arg_list, out_file, in_file, append);
}