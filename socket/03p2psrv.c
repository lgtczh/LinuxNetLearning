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
    printf("recv sig=%d\n", sig);
    exit(EXIT_SUCCESS);
}

int main(void)
{
    int listenfd;
    if((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5199);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1")
    //inet_aton("127.0.0.1", &servaddr.sin_addr);
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind");
    
    if(listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");

    struct sockaddr_in peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    int conn_fd;
    if((conn_fd = accept(listenfd, (struct sockaddr*)&peeraddr, &addrlen)) < 0)
        ERR_EXIT("accept");

    printf("accept: ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
    pid_t pid;
    pid = fork();
    if (pid < 0)
        ERR_EXIT("fork");
    else if (pid > 0){
        signal(SIGUSR1, handler);
        char sendbuf[1024] = {0};
        while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL){
            write(conn_fd, sendbuf, strlen(sendbuf));
            memset(sendbuf, 0, sizeof(sendbuf));
        }
    }else{
        char recvbuf[1024];
        while(1){
            memset(recvbuf, 0, sizeof(recvbuf));
            int recv_len = read(conn_fd, recvbuf, sizeof(recvbuf));
            if (recv_len < 0)
                ERR_EXIT("read");
            else if (recv_len == 0){
                printf("peer close\n");
                //发送信号给父进程，通知其退出
                kill(getppid(), SIGUSR1);
                break;
            }else
               fputs(recvbuf, stdout);
        }
        exit(EXIT_SUCCESS);
    }
    return 0;
}

