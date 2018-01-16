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

#include "my_function.h"
#include "../err_exit.h"

void echo_cli(int sock)
{
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    int max_fd = sock, nread;
    //fileno(FILE* stream) examines the argument stream and returns its integer descriptor.
    int fd_stdin = fileno(stdin);
    if (fd_stdin > max_fd)
        max_fd = fd_stdin;

    fd_set readfds;
    FD_ZERO(&readfds);

    while(1){
        FD_SET(sock, &readfds);
        FD_SET(fd_stdin, &readfds);
        nread = select(max_fd+1, &readfds, NULL, NULL, NULL);
        if (nread == -1)
            ERR_EXIT("select");
        if (nread == 0)
            continue;

        if (FD_ISSET(sock, &readfds)){
            int ret = readline(sock, recvbuf, sizeof(recvbuf));
            if (ret == -1)
	            ERR_EXIT("readline");
            else if (ret == 0) {
                printf("service close\n");
                break;
            }
            fputs(recvbuf, stdout);
            memset(recvbuf, 0, sizeof(recvbuf));
        }
        if (FD_ISSET(stdin, &readfds)){
            //如果返回NULL，说明触发了Ctrl-D，客户端被关闭，跳出循环
            if (fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)
                break;
            writen(sock, sendbuf, strlen(sendbuf));
            memset(sendbuf, 0, sizeof(sendbuf));
        }
    }

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
    
    echo_cli(sock);

    close(sock);
    return 0;

}

