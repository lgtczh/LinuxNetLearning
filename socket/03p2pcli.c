#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "../err_exit.h"

void handler(int sig)
{
    printf("recv a sig=%d\n", sig);
    exit(EXIT_SUCCESS);
}

int main(void)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0)
        ERR_EXIT("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5199);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("connect");
    pid_t pid;
    pid = fork();
    if (pid < 0)
        ERR_EXIT("fork");
    else if (pid == 0){
        signal(SIGUSR1, handler);
        char sendbuf[1024] = {0};
        while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL){
            write(sock, sendbuf, strlen(sendbuf));
            memset(sendbuf, 0, sizeof(sendbuf));
        }
    }else{
        char recvbuf[1024] = {0};
        while(1){
            memset(recvbuf, 0, sizeof(recvbuf));
            int recv_len = read(sock, recvbuf, sizeof(recvbuf));
            if (recv_len < 0)
                ERR_EXIT("read");
            else if (recv_len == 0){
                printf("service close\n");
                kill(pid, SIGUSR1);
                break;
            }else
                fputs(recvbuf, stdout);
        }
    } 
    close(sock);
    return 0;
}
