//
// Created by zy on 17-9-15.
//

#ifndef VPN_UDP_WRAPPER_H
#define VPN_UDP_WRAPPER_H

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

class WrapperUdp
{
public:
    WrapperUdp();

    void setServerIpPort(const char* server_ip, uint16_t server_port);
    ssize_t sentMsg(const char* msg, int len);
    ssize_t readMsg(char* buf, int BUF_SIZE);

    int getFd();

private:
    int sockfd; //文件描述符；
    struct sockaddr_in server;
};

#endif //VPN_UDP_WRAPPER_H
