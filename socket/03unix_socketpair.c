#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "../err_exit.h"

int main(void)
{
    int sock_fd[2];
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, sock_fd) == -1)
        ERR_EXIT("socketpair");

    pid_t pid = fork();
    if (pid == -1)
        ERR_EXIT("fork");

    if (pid == 0) {
        // child
        close(sock_fd[0]);
        int val = 0;
        while (val <= 10){
            printf("sending data : %d\n", val);
            write(sock_fd[1], &val, sizeof(val));
            read(sock_fd[1], &val, sizeof(val));
            printf("receive data : %d\n", val);
            printf("-----------------\n");
            sleep(1);
        }
    }else{
        // father
        int val = 0;
        close(sock_fd[1]);
        while(val >= 0) {
            read(sock_fd[0], &val, sizeof(val));
            ++val;
            write(sock_fd[0], &val, sizeof(val));
        }
    }
    return 0;
}