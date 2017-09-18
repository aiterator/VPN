//
// Created by zy on 17-9-15.
//

#ifndef VPN_UDP_WRAPPER_H
#define VPN_UDP_WRAPPER_H

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

class WrapperUdp
{
public:
    WrapperUdp(const char* ip, const char* port);
    WrapperUdp(const char* ip, uint16_t port);

    void sentMsg(const char* msg, int len);
    void sentMsg(std::string& msg);
    int getFd();

private:
    int sockfd; //文件描述符；
    struct sockaddr_in client;
};

#endif //VPN_UDP_WRAPPER_H
