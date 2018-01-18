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

int main(void)
{
    //1.创建套接字，类型为TCP，其为监听套接字，即监听三次握手完成，一旦完成就把它放到已连接队列，下面的accept函数就可以从队列中返回一个连接，是一个已连接套接字
    int listenfd;
    if((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");

    //2.新建IPv4地址结构，并将其绑定到套接字listenfd上
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5199);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1")
    //inet_aton("127.0.0.1", &servaddr.sin_addr);
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind");
    //3.开始监听该套接字
    if(listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");
    //4.开始接受客户端连接
    struct sockaddr_in peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    int conn_fd;
    if((conn_fd = accept(listenfd, (struct sockaddr*)&peeraddr, &addrlen)) < 0)
        ERR_EXIT("accept");

    printf("accept: ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

    return 0;
}


    
