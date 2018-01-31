#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "../../err_exit.h"
#include "pub.h"


USER_LIST client_list;

void do_login(MESSAGE& new_client, int sock, struct sockaddr_in *cli_addr);
void do_logout(char* username, int sock, struct sockaddr_in *cli_addr);
void do_send_list(int, struct sockaddr_in *);

void chat_srv(int sock);

int main(void)
{
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        ERR_EXIT("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5199);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        ERR_EXIT("bind");

    chat_srv(sock);
    return 0;
}

void chat_srv(int sock)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len;

    MESSAGE msg;  // 从客户端接收到的消息
    ssize_t n;
    while(1){
        memset(&cli_addr, 0, sizeof(cli_addr));
        cli_len = sizeof(cli_addr);
        n = recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&cli_addr, &cli_len);

        if (n == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }

        switch(ntohl(msg.cmd)){
            case C2S_LOGIN:
                printf("user loggin\n");
                do_login(msg, sock, &cli_addr);  // 处理用户登录
                break;
            case C2S_LOGOUT:
                char user_name[16];
                memcpy(user_name, &msg.body, sizeof(user_name));
                do_logout(user_name, sock, &cli_addr);  // 处理用户退出
                break;
            case C2S_ONLINE_USER:
                do_send_list(sock, &cli_addr);  // 给用户返回所有用户列表
                break;
            default:
                printf("%d\n", C2S_ONLINE_USER);
                printf("receive err_cmd : %d\n", ntohl(msg.cmd));
        }
    }
}

void do_login(MESSAGE& new_client, int sock, struct sockaddr_in* cli_addr)
{
    //结构体因为自身太大，所以为了提高效率，传参应该用指针
    //MESSAGE& msg 定义别名，即指针

    // 创建用户信息结构体
    USER_INFO user;
    strcpy(user.username, new_client.body);
    user.ip = cli_addr->sin_addr.s_addr;  // 网络字节序
    user.port = cli_addr->sin_port;  // 网络字节序

    // 检查用户名是否存在
    bool already_has = false;
    for (USER_LIST::iterator it = client_list.begin(); it != client_list.end(); ++ it)
        if (strcmp(it->username, user.username) == 0) {
            already_has = true;
            break;
        }

    // 创建回复信息结构体reply_msg:给新创建的用户返回信息
    MESSAGE reply_msg;
    memset(&reply_msg, 0, sizeof(reply_msg));

    if (already_has) {
        // username已存在
        reply_msg.cmd = htonl(S2C_ALREADY_LOGINED);
        if(sendto(sock, &reply_msg, sizeof(reply_msg), 0, (struct sockaddr*)cli_addr, sizeof(*cli_addr)) == -1)
            ERR_EXIT("sendto[ALREADY_LOGINED]");

        printf("The client %s[%s:%d] has already logined\n", user.username,inet_ntoa(cli_addr->sin_addr), ntohs(user.port));
    } else {
        // username不存在
        client_list.push_back(user);
        printf("Create a new user: %s[%s:%d]\n", user.username, inet_ntoa(cli_addr->sin_addr), ntohs(user.port));

        // 登录成功应答
        reply_msg.cmd = htonl(S2C_LOGIN_OK);
        if (sendto(sock, &reply_msg, sizeof(reply_msg), 0, (struct sockaddr*)cli_addr, sizeof(*cli_addr)) == -1)
            ERR_EXIT("sendto[LOGIN_OK]");

        // 发送在线人数
        int count = htonl((int)client_list.size());
        if (sendto(sock, &count, sizeof(int), 0, (struct sockaddr*)cli_addr, sizeof(*cli_addr)) == -1)
            ERR_EXIT("sendto[count]");

        struct sockaddr_in peer_addr;
        socklen_t peer_len;

        printf("Send the list of users to %s[%s:%d]\n", user.username, inet_ntoa(cli_addr->sin_addr), ntohs(user.port));

        //msg: 给其他用户通知有新用户加入
        MESSAGE msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(S2C_SOMEONE_LOGIN);
        // 将用户信息放入信息body
        memcpy(msg.body, &user, sizeof(user));

        // 给该user发送在线列表, 同时给该user发送在线列表, 同时
        for(USER_LIST::iterator it = client_list.begin(); it != client_list.end(); ++ it) {
            if (sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr *)cli_addr, sizeof(*cli_addr)) == -1)
                ERR_EXIT("sendto[user_list]");

            if (strcmp(it->username, user.username) == 0)
                continue;

            memset(&peer_addr, 0, sizeof(peer_addr));
            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = it->port;
            peer_addr.sin_addr.s_addr = it->ip;

            if (sendto(sock, &msg, sizeof(MESSAGE), 0, (struct sockaddr *) &peer_addr, sizeof(peer_addr)) == -1)
                ERR_EXIT("sendto");
        }
    }

}

void do_logout(char* username, int sock, struct sockaddr_in *cli_addr)
{
    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++ it)
        if (strcmp(it->username, username) == 0)
            break;

    if (it != client_list.end()) {
        client_list.erase(it);
        printf("user \"%s\" has logout!\n", username);
    } else {
        printf("can't find user \"%s\"\n", username);
    }

    MESSAGE msg;
    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(S2C_SOMEONE_LOGOUT);
    memcpy(&msg.body, username, sizeof(username));
    printf("logout_clinet : %s\n", username);
    struct sockaddr_in peer_addr;
    socklen_t peer_len;
    for (it = client_list.begin(); it != client_list.end(); ++ it){
        memset(&peer_addr, 0, sizeof(peer_addr));
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = it->port;
        peer_addr.sin_addr.s_addr = it->ip;
        peer_len = sizeof(peer_addr);
        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&peer_addr, peer_len) == -1)
            ERR_EXIT("logout_sendto");
    }
}

void do_send_list(int sock, sockaddr_in *peer_addr)
{
    MESSAGE msg;
    msg.cmd = htonl(S2C_ONLINE_USER);
    if (sendto(sock, &msg, sizeof(MESSAGE), 0, (struct sockaddr*)peer_addr, sizeof(*peer_addr)) == -1)
        ERR_EXIT("sendto[ONLINE_USER]");

    int size = htonl((int)client_list.size());
    if (sendto(sock, (const char*)&size, sizeof(int), 0, (struct sockaddr*)peer_addr, sizeof(*peer_addr)) == -1)
        ERR_EXIT("sendto");

    for (USER_LIST::iterator it = client_list.begin(); it != client_list.end(); ++ it){
        if (sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr*)peer_addr, sizeof(*peer_addr)) == -1)
            ERR_EXIT("sendto");
    }
    printf("Send user list successfully\n");
}
