//
// Created by zy on 17-9-20.
//
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <tins/tins.h>
#include <unordered_map>
#include "udp_wrapper.h"

using namespace std;
using namespace Tins;

unordered_map<uint16_t , string> serverPort_Mp_clientIpPort;
const char IP_LOCAL[] = "115.159.146.17";
const int UDP_SERVER_PORT = 35000;
const int BUF_SIZE = 100000;
const int EVENT_SIZE = 107;
const int UDP_FD = 0;
const int SNIFFER_FD = 1;

struct markMsg
{
    markMsg(int fd, int mk) : fd(fd), mk(mk){}
    int fd, mk;
};

void epollAdd(int epollFd, int sockFd, int mk, uint32_t status)
{
    struct markMsg *msg = new markMsg(sockFd, mk);
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = status;
    ev.data.ptr = msg;

    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, sockFd, &ev) == - 1)
    {
        perror("epoll_ctl");
        exit(errno);
    }
}


int udpServerInit()
{
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket == -1)
    {
        perror("socket");
        exit(errno);
    }

    struct sockaddr_in udp_server;
    memset(&udp_server, 0, sizeof(udp_server));
    udp_server.sin_family = AF_INET;
    udp_server.sin_port = htons(UDP_SERVER_PORT);
    udp_server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(udp_socket, (struct sockaddr *)&udp_server, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(errno);
    }

    return udp_socket;
}



bool handle(PDU &some_pdu)
{
    IP &ip = some_pdu.rfind_pdu<IP>();
    TCP &tcp = some_pdu.rfind_pdu<TCP>();

    uint16_t port = tcp.dport();
    string d_addr = serverPort_Mp_clientIpPort[port];
    ip.dst_addr(IPv4Address(d_addr.data()));

    WrapperUdp udp(d_addr.data(), port);
    auto buf = ip.serialize();
    if(udp.sentMsg((char *)buf.data(), buf.size()) <= 0)
        perror("udp.sendMsg");

    return false;
}

int main()
{
    int epollFd = epoll_create1(0);
    if(epollFd == -1)
    {
        perror("epoll_create");
        exit(errno);
    }
    int udp_socket_fd = udpServerInit();
    epollAdd(epollFd, udp_socket_fd, UDP_FD, EPOLLIN);

    SnifferConfiguration config;
    config.set_filter("ip dst 115.159.146.17");
    Sniffer sniffer("etho", config);
    int sniffer_fd = sniffer.get_fd();
    epollAdd(epollFd, sniffer_fd, SNIFFER_FD, EPOLLIN);

    struct epoll_event event[EVENT_SIZE];
    char buf[BUF_SIZE];


    size_t read_bytes;
    struct sockaddr_in udp_client;
    socklen_t clientLen = sizeof(udp_client);
    PacketSender sender;

    while(true)
    {
        int size = epoll_wait(epollFd, event, EVENT_SIZE-1, -1);

        for(int i = 0; i < size; ++ i)
        {
            int mk = ((struct markMsg * )event[i].data.ptr)->mk;
            int fd = ((struct markMsg * )event[i].data.ptr)->fd;
            if(mk == UDP_FD)
            {
                read_bytes = recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&udp_client, &clientLen);
                if(read_bytes < 0)
                {
                    perror("recvfrom");
                    continue;
                }

                IP ip_package;
                try
                {
                    ip_package = RawPDU((uint8_t*)buf, read_bytes).to<IP>();
                }
                catch (...)
                {
                    continue;
                }

                serverPort_Mp_clientIpPort[udp_client.sin_port] = ip_package.src_addr().to_string();

                ip_package.src_addr(IPv4Address(IP_LOCAL));
                sender.send(ip_package);
            }
            else
            {
                sniffer.sniff_loop(handle);
            }
        }
    }
}