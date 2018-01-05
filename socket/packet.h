//
// Created by 曹志昊 on 2018/1/4.
//

#ifndef LINUXNET_PACKET_H
#define LINUXNET_PACKET_H
struct packet
{
    int len;
    char buf[1024];
};
#endif //LINUXNET_PACKET_H
