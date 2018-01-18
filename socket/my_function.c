#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>

#include "../err_exit.h"

ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    size_t nwritten;
    char * bufp = (char *)buf;

    while (nleft > 0){
        if ((nwritten = write(fd, bufp, count)) < 0){
            if (errno == EINTR)
                continue;
            return -1;
        }else if (nwritten == 0)
            continue;
        bufp += nwritten;
        nleft -= nwritten;
    }
    return count;
}

ssize_t readn(int fd, void *buf, size_t count)
{
    size_t nleft = count;
    size_t nread = 0;
    char * bufp = (char *)buf;
    while (nleft > 0){
        if ((nread = read(fd, bufp, nleft)) < 0){
            if (errno == EINTR)
                continue;
            return -1;
        }else if (nread == 0)
            return count - nleft;

        bufp += nread;
        nleft -= nread;
    }
    return count;
}
ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
    while (1){
        int recv_len = recv(sockfd, buf, len, MSG_PEEK);
        if (recv_len < 0 && errno == EINTR)
            continue;
        return recv_len;
    }
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
    int ret, nread = 0, nleft = maxline;
    char *bufp = (char *)buf;
    while (1){
        ret = recv_peek(sockfd, bufp, nleft);
        if (ret < 0){
            return ret;
        }else if (ret == 0)
            return nread;
        nread += ret;
        int i;
        for(i=0; i < ret; ++i){
            int len;
            if (bufp[i] == '\n'){
                len = readn(sockfd, bufp, i+1);
                if (len != i+1)
                    exit(EXIT_FAILURE);
                return nread - ret + len;
            }
        }
        if (nread > maxline)
            exit(EXIT_FAILURE);
        nleft -= nread;
        //ret = readn(sockfd, bufp, nread);
        //if (ret != nread)
        //    exit(EXIT_FAILURE);
        bufp += ret;
   }
}
/**
 * read_timeout : 读超时检测函数，不含读操作
 * @param fd 要检测的文件描述符
 * @param wait_seconds 超时时间
 * @return 未超时检测到可读数据返回0; 检测到超时后返回-1同时errno=ETIMEDOUT; 发生错误返回-1
 */
int read_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds <= 0)
        return 0;

    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    timeout.tv_sec = wait_seconds;
    timeout.tv_usec = 0;

    do{
        /**
         * 超时时间内select检测到可读事件，返回可读事件数量
         * 超时后返回0
         * 发生错误返回-1
         */
        ret = select(fd, &readfds, NULL, NULL, &timeout);
    }while(ret < 0 && errno == EINTR);//被打断后如果没有超时(ret!=0)则继续循环

    if (ret == 0){
        errno = ETIMEDOUT;
        ret = -1;
    }else if (ret == 1){
        ret = 0;
    }
    return ret;
}

/**
 * read_timeout : 读超时检测函数，不含写操作
 * @param fd 要检测的文件描述符
 * @param wait_seconds 超时时间
 * @return 未超时检测到可读数据返回0; 检测到超时后返回-1同时errno=ETIMEDOUT; 发生错误返回-1
 */
int write_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds <= 0)
        return 0;

    fd_set writefds;
    struct timeval timeout;

    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    timeout.tv_sec = wait_seconds;
    timeout.tv_usec = 0;

    do{
        /**
         * 超时时间内select检测到可写事件，返回可写事件数量
         * 超时后返回0
         * 发生错误返回-1
         */
        ret = select(fd, NULL, &writefds, NULL, &timeout);
    }while(ret < 0 && errno == EINTR);//被打断后如果没有超时(ret!=0)则继续循环

    if (ret == 0){
        errno = ETIMEDOUT;
        ret = -1;
    }else if (ret == 1){
        ret = 0;
    }
    return ret;
}

/**
 * accept_timeout : 带超时的accept
 * @param fd 套接字
 * @param addr 输出参数，返回对方地址
 * @param wait_seconds 等待超时时间；0表示正常模式不检测超时
 * @return 成功（未超时）返回已连接套接字，超时返回-1并且errno=ETIMEDOUT; 失败返回-1
 */
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if (wait_seconds > 0) {
        fd_set accept_fds;
        struct timeval timeout;

        FD_ZERO(&accept_fds);
        FD_SET(fd, &accept_fds);

        timeout.tv_sec = wait_seconds;
        timeout.tv_sec = 0;

        do {
            ret = select(fd, &accept_fds, NULL, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);
        // ret <= 0 说明超时或失败了，直接返回
        if (ret <= 0) {
            if (ret == 0)
                errno = ETIMEDOUT;
            return ret;
        }
    }
    if (addr != NULL)
        ret = accept(fd, (struct sockaddr*)addr, &addrlen);
    else
        ret = accept(fd, NULL, NULL);
    return ret;
}
/**
 * activate_nonblock 设置I/O为非阻塞模式
 * @param fd 文件描述符
 * @return
 */
void activate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        ERR_EXIT("fcntl");

    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if (ret == -1)
        ERR_EXIT("fcntl");
}

/**
 * deactivate_nonblock 设置I/O为阻塞模式
 * @param fd 文件描述符
 */
void deactivate_nobblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        ERR_EXIT("fcntl");

    flags &= ~O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if (ret == -1)
        ERR_EXIT("fcntl");
}

/**
 * connect_timeout 设置connect超时连接
 * @param fd 套接字
 * @param addr 要连接的对方地址
 * @param wait_seconds 等待超时秒数，如果为0表示正常模式
 * @return 成功（未超时）返回0；失败返回-1，超时返回-1且errno=ETIMEDOUT
 */
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if (wait_seconds > 0)
        activate_nonblock(fd);

    ret = connect(fd, (struct sockaddr *)addr, addrlen);
    if (ret < 0 && errno == EINPROGRESS){
        /**
         * ret == 0 : connect直接返回，连接成功，不需等待超时
         * ret < 0 但 errno != EPROGRESS 连接失败，不需再等待
         */
        fd_set connect_fds;
        struct timeval timeout;

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        FD_ZERO(&connect_fds);
        FD_SET(fd, &connect_fds);

        do{
            /**
             * 一旦连接建立，套接字就可写了
             */
            ret = select(fd+1, NULL, &connect_fds, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);
        if (ret == 0){
            //连接超时
            ret = -1;
            errno = ETIMEDOUT;
        }else if (ret == -1)
            return -1;
        else{
            /**
             * ret返回1，可能有两种情况：
             *   1. 连接建立成功
             *   2. 套接字产生错误: 此时由于select没有错误, 错误信息不回保存至errno, 因此需要调用getsockopt来获取
             */
            int err;
            socklen_t errlen = sizeof(err);
            int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen);
            if (sockoptret == -1)
                return -1;

            if (err == 0)
                ret = 0;//连接成功
            else{
                errno = err;
                ret = -1;
            }
        }
    }
    if (wait_seconds > 0)
        deactivate_nobblock(fd);

    return ret;
}