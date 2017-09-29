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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

class WrapperUdp
{
public:
    WrapperUdp(uint16_t local_port);

    ssize_t sentMsg(const char* msg, int len, const sockaddr_in &Client);
    ssize_t readMsg(char* buf, int BUF_SIZE);
    sockaddr_in getSockaddr_in();
    int getFd();

private:
    int sockfd; //文件描述符；
    struct sockaddr_in client;
};

#endif //VPN_UDP_WRAPPER_H
