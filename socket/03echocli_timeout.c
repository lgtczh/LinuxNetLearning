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
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0)
        ERR_EXIT("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5199);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect_timeout(sock, &servaddr, 5);
    if (ret == -1 && errno == ETIMEDOUT){
        printf("timeout...\n");
        return 1;
    }else if (ret == -1)
        ERR_EXIT("connect_timeout");

    struct sockaddr_in localaddr;
    socklen_t addr_len = sizeof(localaddr);
    if ((ret = getsockname(sock, (struct sockaddr*)&localaddr, &addr_len)) < 0)
        ERR_EXIT("getsockname");
    else
        printf("local_ip = %s, local_port = %d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

    return 0;
}
