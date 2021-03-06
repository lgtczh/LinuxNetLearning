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
    sleep(5);
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

    int nready, maxfd = listenfd;//初始状态listenfd的文件描述符最大
    int conn_fd_set[FD_SETSIZE], last_idx = -1;//存放已连接的套接字, last_null_idx记录最后的空闲位置索引
    fd_set readfds, allset;//allset记录所有的文件描述符

    FD_ZERO(&readfds);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (int i = 0;i < FD_SETSIZE; ++i)
        conn_fd_set[i] = -1;


    while(1)
    {
        readfds = allset;
        nready = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if (nready == -1){
            if (errno == EINTR)
                continue;
            ERR_EXIT("select");
        }
        if (nready == 0)
            continue;//timeout为NULL，不会执行到此
        if (FD_ISSET(listenfd, &readfds)){
            //listenfd产生读事件后，说明accept不再堵塞了
            if ((conn_fd = accept(listenfd, (struct sockaddr * )&peeraddr, &addrlen)) < 0)
                ERR_EXIT("accept");
            printf("accept: ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

            int i;
            for (i = 0;i < FD_SETSIZE; ++ i) {
                //从头开始查找第一个空闲的数组元素
                if (conn_fd_set[i] == -1) {
                    conn_fd_set[i] = conn_fd;
                    if (i > last_idx)
                        last_idx = i;
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                fprintf(stderr, "too many clients\n");
                continue;
            }
            FD_SET(conn_fd, &allset);
            if (maxfd < conn_fd)
                maxfd = conn_fd;

            if (--nready <= 0)
                continue;
        }
        for (int i = 0;i <= last_idx; ++i){
            conn_fd = conn_fd_set[i];
            if (FD_ISSET(conn_fd, &readfds)) {
                //printf("get conn_fd = %d\nmax_fd = %d\nlast_idx = %d\n", conn_fd, maxfd, last_idx);
                int ret = echo_srv(conn_fd);
                if (ret == -1) {
                    FD_CLR(conn_fd, &allset);
                    conn_fd_set[i] = -1;
                    close(conn_fd);//必须关闭套接口
                    //判断是否要更新maxfd
                    if (i == maxfd)
                        for (int j = i-1; j >= 0; --j)
                            if (conn_fd_set[j] != -1) {
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

