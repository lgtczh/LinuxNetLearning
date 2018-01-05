#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "my_function.h"
#include "../err_exit.h"
#include "packet.h"

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
    struct packet sendbuf;
    struct packet recvbuf;
    int n,head_len,buf_real_len;
    memset(&sendbuf, 0, sizeof(sendbuf));
    memset(&recvbuf, 0, sizeof(recvbuf));
    while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL){
        n = strlen(sendbuf.buf);
        sendbuf.len = htonl(n);
        writen(sock, &sendbuf, sizeof(sendbuf.len)+n);
        
        head_len = readn(sock, &recvbuf.len, sizeof(recvbuf.len));
        if (head_len == -1)
            ERR_EXIT("read buf len");
        else if (head_len < sizeof(recvbuf.len)){
            printf("[head]service close\n");
            return -1;
        }
        buf_real_len = ntohl(recvbuf.len);
        int buf_len = readn(sock, recvbuf.buf, buf_real_len);
        if (buf_len == -1)
            ERR_EXIT("read buf");
        else if (buf_len < buf_real_len){
            printf("service close\n");
            return -1;
        }
        fputs(recvbuf.buf, stdout);
        memset(&recvbuf, 0, sizeof(recvbuf));
        memset(&recvbuf, 0, sizeof(recvbuf));
    }
    close(sock);
    return 0;

}

