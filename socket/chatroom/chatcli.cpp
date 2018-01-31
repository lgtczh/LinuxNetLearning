#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#include "../../err_exit.h"
#include "pub.h"

// 当前用户名
char username[16];
// 聊天室成员列表
USER_LIST client_list;

void chat_cli(int sock);
void parse_cmd(char * cmdline, int sock, struct sockaddr_in *srv_addr);
void do_someone_login(MESSAGE&);
void do_someone_logout(int, MESSAGE&, sockaddr_in*);
void do_getlist(int);
void send_getlist(int, sockaddr_in*);
bool send_msg(int, char*, char*, sockaddr_in*);
void do_chat(const MESSAGE&);

int main(void)
{
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        ERR_EXIT("socket");

    chat_cli(sock);
    return 0;
}

void chat_cli(int sock)
{
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(5199);
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    MESSAGE msg, reply_msg;

    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(C2S_LOGIN);
    printf("Welcome!\n");
    while(1){
        memset(username, 0, sizeof(username));
        printf("Please input your name:");
        fflush(stdout);
        scanf("%s", username);

        // 发送登录请求
        strcpy(msg.body, username);
        sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        // 接收服务器回复
        recvfrom(sock, &reply_msg, sizeof(reply_msg), 0, NULL, NULL);
        int cmd = ntohl(reply_msg.cmd);
        if (cmd == S2C_ALREADY_LOGINED)
            printf("\"%s\" has already logined room, please use another name!\n", username);
        else if (cmd == S2C_LOGIN_OK){
            printf("You has logined room successfully!\n");
            break;
        }
    }
    // 接收当前用户数量
    int count;
    recvfrom(sock, &count, sizeof(count), 0, NULL, NULL);
    count = ntohl(count);
    printf("The room has %d users\n", count);

    // 接收其他客户信息，并打印
    for (int i = 0;i < count; ++ i){
        USER_INFO user;
        recvfrom(sock, &user, sizeof(user), 0, NULL, NULL);
        client_list.push_back(user);

        in_addr tmp;
        tmp.s_addr = user.ip;
        printf("\t(%d)User : %s [%s:%d]\n", i, user.username, inet_ntoa(tmp), ntohs(user.port));
    }

    // 进入聊天
    printf("\nYou can use these commands:\n");
    printf("send <username> <msg>   给<username>发送<msg>\n");
    printf("list                    列出所有在线客户名单\n");
    printf("exit                    退出客户端\n");

    fd_set rset;
    FD_ZERO(&rset);
    int nready;
    struct sockaddr_in addr;
    socklen_t addr_len;
    while(1){
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sock, &rset);
        nready = select(sock+1, &rset, NULL, NULL, NULL);
        // 报错
        if (nready == -1){
            if (errno == EINTR)
                continue;
            ERR_EXIT("select");
        }
        // 键盘输入命令
        if (FD_ISSET(STDIN_FILENO, &rset)) {
            char cmdline[1024] = {0};
            if (fgets(cmdline, sizeof(cmdline), stdin) == NULL)
                // 退出
                break;
            if (cmdline[0] == '\n')
                // 输入为空
                continue;
            cmdline[strlen(cmdline) - 1] = '\0';  // 将"\n"置为"\0"
            // 解析命令
            parse_cmd(cmdline, sock, &srv_addr);
        }

        // 接收到信息
        if (FD_ISSET(sock, &rset)){
            addr_len = sizeof(addr);
            memset(&reply_msg, 0, sizeof(reply_msg));
            recvfrom(sock, &reply_msg, sizeof(reply_msg), 0, (struct sockaddr*)&addr, &addr_len);
            switch(ntohl(reply_msg.cmd)){
                case S2C_SOMEONE_LOGOUT:
                    do_someone_logout(sock, reply_msg, &srv_addr);
                    break;
                case S2C_SOMEONE_LOGIN:
                    do_someone_login(reply_msg);
                    break;
                case S2C_ONLINE_USER:
                    do_getlist(sock);
                    break;
                case C2C_CHAT:
                    do_chat(reply_msg);
                    break;
                default:
                    printf("Received an cmd[%d] that I could not identify.\n", ntohl(reply_msg.cmd));
            }
        }
    } // end while
    msg.cmd = htonl(C2S_LOGOUT);
    strcpy(msg.body, username);
    if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1)
        ERR_EXIT("sendto[C2S_LOGOUT]");
    printf("You has logout from room.\n");
    exit(EXIT_SUCCESS);
}

void parse_cmd(char *cmdline, int sock, struct sockaddr_in *srv_addr)
{
    char cmd[1024] = {0};
    char *p;
    p = strchr(cmdline, ' ');
    if (p != NULL)
        *p = '\0';
    strcpy(cmd, cmdline);// 只将将命令拷贝到cmd

    // 创建信息发送结构体变量
    MESSAGE msg;
    memset(&msg, 0, sizeof(msg));
    if (strcmp(cmd, "exit") == 0) {
        msg.cmd = htonl(C2S_LOGOUT);
        strcpy(msg.body, username);
        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)srv_addr, sizeof(*srv_addr)) == -1)
            ERR_EXIT("sendto[C2S_LOGOUT]");
        printf("You has logout from room.\n");
        exit(EXIT_SUCCESS);
    } else if (strcmp(cmd, "list") == 0) {
        send_getlist(sock, srv_addr);

    } else if (strcmp(cmd, "send") == 0) {
        char peer_name[16];
        char msg_body[MSG_LEN] = {0};
        while(*p++ == ' ');  // 过滤中间空格，p指向去掉命令cmd后第一个不是空格的字符
        char *p2 = strchr(p, ' ');
        if (p2 == NULL){
            printf("Bad command\n");
            printf("You should use \"send <username> <msg>\n");
            return;
        }
        *p2 = '\0';
        strcpy(peer_name, p);

        while(*p2++ == ' ');
        strcpy(msg_body, p2);
        send_msg(sock, peer_name, msg_body, srv_addr);
    } else{
        printf("I can't parse the cmdline : %s\n", cmdline);
        printf("\nYou can user these commands:\n");
        printf("send <username> <msg>   给<username>发送<msg>\n");
        printf("list                    列出所有在线客户名单\n");
        printf("exit                    退出客户端\n");
    }
}

void do_someone_login(MESSAGE& login)
{
    USER_INFO *user = (USER_INFO*)login.body;
    client_list.push_back(*user);
    struct in_addr tmp;
    tmp.s_addr = user->ip;
    printf("The new user[%s] login room\n", user->username);
}

void do_someone_logout(int sock, MESSAGE& logout, sockaddr_in *srv_addr)
{
    USER_INFO user;
    memset(&user, 0, sizeof(USER_INFO));
    memcpy(user.username, logout.body, sizeof(user.username));
    int count = 1;
    do {
        printf("%s logout\n", user.username);
        USER_LIST::iterator it = find(client_list.begin(), client_list.end(), user);
        if (it != client_list.end()) {
            client_list.erase(it);
            printf("User[%s] has logout the room\n", username);
            return;
        }
        if (count > 0) {
            printf("Find unknown user[%s], update the client list...\n", user.username);
            send_getlist(sock, srv_addr);
            sleep(3);
            --count;
            continue;
        } else {
            printf("The user[%s] is not find in server.\n", user.username);
            -- count;
        }
    }while(count >= 0);
}

void send_getlist(int sock, struct sockaddr_in *srv_addr)
{
    MESSAGE msg;
    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(C2S_ONLINE_USER);

    if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)srv_addr, sizeof(*srv_addr)) == -1)
        ERR_EXIT("sendto");
}

void do_getlist(int sock)
{
    int count;
    recvfrom(sock, &count, sizeof(int), 0, NULL, NULL);
    count = ntohl(count);
    USER_INFO user;
    for (int i = 0;i < count; ++ i){
        recvfrom(sock, &user, sizeof(user), 0, NULL, NULL);
        struct in_addr tmp;
        tmp.s_addr = user.ip;
        printf("\t(%d)User : %s [%s:%d]\n", i, user.username, inet_ntoa(tmp), ntohs(user.port));
        USER_LIST::iterator it = find(client_list.begin(), client_list.end(), user);
        if (it == client_list.end()){
            client_list.push_back(user);
        }
    }
}

bool send_msg(int sock, char* user, char* msg_body, sockaddr_in* srv_addr)
{
    if (strcmp(user, username) == 0){
        printf("You can't send message to yourself\n");
        return false;
    }
    int  count = 1;
    USER_LIST::iterator it;
    do {
        for (it = client_list.begin(); it != client_list.end(); ++ it)
            if (strcmp(it->username, user) == 0)
                break;
        if (it != client_list.end())
            break;
        if (it == client_list.end() && count > 0) {
            printf("I can't find the user \"%s\", update the client data...\n", user);
            send_getlist(sock, srv_addr);
            --count;
            continue;
        }else{
            printf("The user \"%s\" did not login in room\n", user);
            return false;
        }

    }while(count <= 0);
    MESSAGE msg;
    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(C2C_CHAT);

    CHAT_MSG cm;
    strcpy(cm.username, username);
    strcpy(cm.msg, msg_body);

    memcpy(msg.body, &cm, sizeof(cm));

    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = it->port;
    peer_addr.sin_addr.s_addr = it->ip;

    sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
    printf("Send message[%s] to [%s\\%s:%d] successfulyy\n", msg_body, user, inet_ntoa(peer_addr.sin_addr), ntohs(it->port));
    return true;
}

void do_chat(const MESSAGE& msg)
{
    CHAT_MSG *cm = (CHAT_MSG*)msg.body;
    printf("Recv[%s] : %s\n", cm->username, cm->msg);
}
