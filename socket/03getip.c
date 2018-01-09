#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "../err_exit.h"

int main(void)
{
    char host[100] = {0};
    int ret = gethostname(host, sizeof(host));
    if (ret == -1)
        ERR_EXIT("gethostname");
    else
        printf("hostname = %s\n", host);

    struct hostent * host_ips;
    host_ips = gethostbyname(host);
    printf("the length of ip is %d\nthe type of ip is %d\n", host_ips->h_length,host_ips->h_addrtype);
    int i = 0;
    while(host_ips->h_addr_list[i] != NULL){
        printf("%s\n", inet_ntoa(*(struct in_addr*)host_ips->h_addr_list[i]));
        ++i;
    }
    return 0;
}


