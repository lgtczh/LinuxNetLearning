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
void echo_cli(int sock)
{
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL){
        int w_l = writen(sock, sendbuf, strlen(sendbuf));
        int ret = readline(sock, recvbuf, sizeof(recvbuf));
        if (ret == -1)
	    ERR_EXIT("readline");
        else if (ret == 0){
	    printf("service close\n");
            break;
        }    
        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
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

