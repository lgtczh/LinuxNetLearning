#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "../err_exit.h"
#include "my_function.h"

void echo_srv(int conn_fd)
{
    char recvbuf[1024];

    while(1){
        memset(recvbuf, 0, sizeof(recvbuf));
        int recv_len = readline(conn_fd, recvbuf, sizeof(recvbuf));
        if (recv_len == 0){
            printf("Client close\n");
            break;
        }else if(recv_len < 0){
            ERR_EXIT("client read");
        }
        fputs(recvbuf, stdout);
        writen(conn_fd, recvbuf, recv_len);
    }
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
    socklen_t addrlen = sizeof(peeraddr);//必须有初始值，accept之后为对方地址的长度
   
    int conn_fd; 
    pid_t pid;
    while(1){
        if((conn_fd = accept(listenfd, (struct sockaddr*)&peeraddr, &addrlen)) < 0)
            ERR_EXIT("accept");
        printf("accept: ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
        
        pid = fork();
        if (pid < 0)
            ERR_EXIT("fork");
        else if (pid == 0){
            printf("I am child.I deal one client,my pid is %d\n", getpid());
            close(listenfd);
            echo_srv(conn_fd);
            exit(EXIT_SUCCESS);
        }else{
            printf("I am father.I wait one client,my pid is %d\n", getpid());
            close(conn_fd);
        }
    }
    return 0;
}

