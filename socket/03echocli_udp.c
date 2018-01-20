#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../err_exit.h"

void echo_cli(int sock)
{
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(5199);
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char send_buf[1024];
    char recv_buf[1024];
    connect(sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    int n;
    while(fgets(send_buf, sizeof(send_buf), stdin) != NULL){
        ssize_t ret = sendto(sock, send_buf, strlen(send_buf), 0, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        if (ret == -1)
            ERR_EXIT("sendto");

        ret = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, NULL, NULL);
        if (ret == -1)
            ERR_EXIT("recvfrom");
        fputs(recv_buf, stdout);
        memset(&recv_buf, 0, sizeof(recv_buf));
        memset(&send_buf, 0, sizeof(send_buf));
    }
    close(sock);
}

int main(void){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        ERR_EXIT("socket");

    echo_cli(sock);
    return 0;

}

