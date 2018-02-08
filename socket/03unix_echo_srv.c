#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/un.h>
#include "../err_exit.h"

void echo_srv(int conn)
{
    char recvbuf[1024];
    int n;
    while (1){
        memset(recvbuf, 0, sizeof(recvbuf));
        n = read(conn, recvbuf, sizeof(recvbuf));
        if (n == -1){
            if (n == EINTR)
                continue;
            ERR_EXIT("read");
        }else if (n == 0){
            printf("client close\n ");
            break;
        }else{
            fputs(recvbuf, stdout);
            write(conn, recvbuf, strlen(recvbuf));
        }
    }
}
int main(void)
{
    int listenfd;
    if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket");

    unlink("test_socket");// 先删除，否则提示地址正在使用
    struct sockaddr_un servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, "/tmp/test_socket");

    //绑定, 产生文件test_socket, 类型为s(socket)
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind");

    //监听
    if (listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");

    int conn;
    pid_t pid;
    while(1){
        conn = accept(listenfd, NULL, NULL);
        if(conn == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("accept");
        }
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");

        if (pid == 0)
        {
            close(listenfd);
            echo_srv(conn);
            close(conn);
            exit(EXIT_SUCCESS);
        }else{
            close(conn);
        }
    }

}

