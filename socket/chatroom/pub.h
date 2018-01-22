#ifndef LINUXNET_PUB_H
#define LINUXNET_PUB_H

#include <list>
#include <algorithm>

using namespace std;

// C2S
#define C2S_LOGIN           0x01
#define C2S_LOGOUT          0x02
#define C2S_ONLINE_USER     0x03

#define MSG_LEN             512

// S2C
#define S2C_LOGIN_OK        0x01
#define S2C_ALREADY_LOGINED 0x02
#define S2C_SOMEONE_LOGIN   0x03
#define S2C_SOMEONE_LOGOUT  0x04
#define S2C_ONLINE_USER     0x05

// C2C
#define C2C_CHAT            0x06

// 消息结构
typedef struct message
{
    int cmd;  // 上面定义的宏变量
    /**
     * cmd:C2S_LOGIN => body:username
     */
    char body[MSG_LEN];
}  MESSAGE;

// 用户信息
typedef struct user_info
{
    char username[16];
    unsigned int ip;
    unsigned short port;
    bool operator == (const user_info& c) const //重载==
    {
        if (strcmp(username, c.username) != 0)
            return false;
        else
            return true;
    }
} USER_INFO;

// 聊天信息结构
typedef struct chat_msg
{
    char username[16];
    char msg[100];
} CHAT_MSG;

// 用户列表
typedef list<USER_INFO> USER_LIST;

#endif //LINUXNET_PUB_H
