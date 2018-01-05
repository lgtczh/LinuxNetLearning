#ifndef LINUXNET_MY_FUNCTION
#define LINUXNET_MY_FUNCTION

ssize_t writen(int fd, const void *buf, size_t count);
ssize_t readn(int fd, void *buf, size_t count);
ssize_t recv_peek(int sockfd, void *buf, size_t len);
ssize_t readline(int sockfd, void *buf, size_t maxline);

#endif
