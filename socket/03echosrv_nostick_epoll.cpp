#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <vector>
#include <algorithm>

#include "../err_exit.h"
#include "my_function.h"

typedef std::vector<struct epoll_event> EventList;

int echo_srv(int conn_fd)
{
    char recvbuf[1024];
    memset(recvbuf, 0, sizeof(recvbuf));
    int recv_len = readline(conn_fd, recvbuf, sizeof(recvbuf));
    if (recv_len == 0){
        printf("Client[%d] close\n", conn_fd);
        return -1;
    }else if(recv_len < 0){
        ERR_EXIT("client read");
    }
    fputs(recvbuf, stdout);
    writen(conn_fd, recvbuf, recv_len);
    return 0;
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

//    std::vector<int> clients;
    int epollfd = epoll_create1(EPOLL_CLOEXEC);

    //将监听套接字及其感兴趣事件加入到epoll实例
    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);

    EventList events(16);
    printf("events.size() = %d\n",static_cast<int>(events.size()));
    int nready;

    while(1)
    {
        nready = epoll_wait(epollfd, &*events.begin(), static_cast<int>(events.size()), -1);
        if (nready == -1){
            if (errno == EINTR)
                continue;
            ERR_EXIT("epoll_wait");
        } else if (nready == 0)
            continue;//timeout为NULL，不会执行到此

        // 套接字数量达到上限后，扩容
        if ((size_t)nready == events.size())
            events.resize(events.size()*2);

        for (int i = 0;i < nready; ++i){
            if (events[i].data.fd == listenfd){
                //listenfd产生读事件后，说明accept不再堵塞了
                if ((conn_fd = accept(listenfd, (struct sockaddr * )&peeraddr, &addrlen)) < 0)
                    ERR_EXIT("accept");
                printf("accept: ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

//                clients.push_back(conn_fd);

                activate_nonblock(conn_fd);
                //将已连接套接字添加到epoll实例中
                event.data.fd = conn_fd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_fd, &event);

            } else if( events[i].events && EPOLLIN){
                if((conn_fd = events[i].data.fd) < 0)
                    continue;
                int ret = echo_srv(conn_fd);
                if (ret == -1) {
                    close(conn_fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, conn_fd, &events[i]);
                }
            }
        }

    }
    return 0;
}

