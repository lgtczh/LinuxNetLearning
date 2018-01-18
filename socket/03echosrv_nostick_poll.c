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
#include <poll.h>
#include <sys/resource.h>

#include "../err_exit.h"
#include "my_function.h"

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

    //获取当前最大文件打开数
    struct rlimit rlim;
    if (getrlimit(RLIMIT_NOFILE, &rlim) < 0)
        ERR_EXIT("getrlimit");
    int nofile = rlim.rlim_cur;

    struct sockaddr_in peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    int conn_fd;

    int nready, maxfd = listenfd;//初始状态listenfd的文件描述符最大

    struct pollfd conn_fd_set[nofile];//存放已连接的套接字,
    int last_idx = 0;//last_idx记录最后的空闲位置索引
    conn_fd_set[0].fd = listenfd;
    conn_fd_set[0].events = POLLIN;
    for (int i = 1;i < nofile; ++i)
        conn_fd_set[i].fd = -1;

    while(1)
    {
        nready = poll(conn_fd_set, maxfd+1, -1);
        if (nready == -1){
            if (errno == EINTR)
                continue;
            ERR_EXIT("poll");
        }else if (nready == 0)
            continue;//timeout为NULL，不会执行到此

        if (conn_fd_set[0].revents & POLLIN){
            //listenfd产生读事件后，说明accept不再堵塞了
            if ((conn_fd = accept(listenfd, (struct sockaddr * )&peeraddr, &addrlen)) < 0)
                ERR_EXIT("accept");
            printf("accept: ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

            int i;
            for (i = 0;i < nofile; ++ i) {
                //从头开始查找第一个空闲的数组元素
                if (conn_fd_set[i].fd == -1) {
                    conn_fd_set[i].fd = conn_fd;
                    conn_fd_set[i].events = POLLIN;
                    if (i > last_idx)
                        last_idx = i;
                    break;
                }
            }
            if (i == nofile) {
                fprintf(stderr, "too many clients\n");
                continue;
            }
            //更新最大文件描述符
            if (maxfd < conn_fd)
                maxfd = conn_fd;

            if (--nready <= 0)
                continue;
        }
        for (int i = 0;i <= last_idx; ++i){
            if (conn_fd_set[i].revents & POLLIN) {
                conn_fd = conn_fd_set[i].fd;
                int ret = echo_srv(conn_fd);
                if (ret == -1) {
                    conn_fd_set[i].fd = -1;
                    close(conn_fd);//必须关闭套接口
                    //判断是否要更新maxfd
                    if (i == maxfd)
                        for (int j = i-1; j >= 0; --j)
                            if (conn_fd_set[j].fd != -1) {
                                maxfd = j;
                                break;
                            }
                }
                if (--nready <= 0)
                    break;
            }
        }

    }
}

