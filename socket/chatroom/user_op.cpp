#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "pub.h"

int main(void)
{
    USER_LIST client_list;
    USER_INFO user1;
    strcpy(user1.username,"tom");
    user1.ip = 122324;
    USER_INFO user2;
    strcpy(user2.username, "jack");
    client_list.push_back(user1);
    client_list.push_back(user2);
    USER_INFO user;
    strcpy(user.username, "olive");
    USER_LIST::iterator it = std::find(client_list.begin(), client_list.end(), user);
    if (it != client_list.end())
        printf("YES\n");
    else
        printf("NO\n");
    return 0;
}
