#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "../err_exit.h"

int main(void)
{
    pid_t fpid;
    int count = 0;
    fpid = fork();
    if (fpid < 0)
        ERR_EXIT("fork");
    
    if (fpid == 0){
        printf("I am the child process, my process id is %d\n", getpid());
        count ++;
    }else{
        printf("I am the father process, my process id is %d\n", getpid());
        count ++;
    }
    printf("The result is %d\n", count);
    return 0;
}
        
