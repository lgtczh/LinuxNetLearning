#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

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
    int ret, nread, nleft = maxline;
    char *bufp = (char *)buf;
    while (1){
        ret = recv_peek(sockfd, bufp, nleft);
        if (ret <= 0)
            return ret;
        nread = ret;
        int i;
        for(i=0; i < nread; ++i){
            if (bufp[i] == '\n'){
                ret = readn(sockfd, bufp, i+1);
                if (ret != i+1)
                    exit(EXIT_FAILURE);
                return ret;
            }
        }
        if (nread > maxline)
            exit(EXIT_FAILURE);
        nleft -= nread;
        //ret = readn(sockfd, bufp, nread);
        //if (ret != nread)
        //    exit(EXIT_FAILURE);
        bufp += nread;
    }
}
