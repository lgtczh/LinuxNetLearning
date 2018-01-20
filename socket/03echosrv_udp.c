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

void echo_srv(int sock)
{
    char recvbuf[1024];
    struct sockaddr_in peer_addr;
    socklen_t peerlen;
    int n;
    while(1){
        memset(recvbuf, 0, sizeof(recvbuf));
        peerlen = sizeof(peer_addr);
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&peer_addr, &peerlen);
        if (n == -1){
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }else if (n > 0) {
            fputs(recvbuf, stdout);
            sendto(sock, recvbuf, n, 0, (struct sockaddr *) &peer_addr, peerlen);
        }
    }
    close(sock);
}

int main(void){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        ERR_EXIT("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5199);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
        ERR_EXIT("bind");

    echo_srv(sock);

}

