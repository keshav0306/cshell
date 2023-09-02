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

int main(){
    // if(pid){

    //     // printf("stop\n");
    //     // kill(pid, SIGSTOP);
    //     // sleep(3);
    //     // printf("continuing\n");
    //     // kill(pid, SIGCONT);
    // }
    // else{
        int i = 100000;
        while(i--){
        printf("child\n");
        usleep(100000);
        }
        //execlp("/bin/sleep","sleep", "5", NULL);
    
}