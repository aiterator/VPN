//
// Created by zy on 9/13/17.
//
#include <iostream>
#include <sys/epoll.h>
#include <tins/tins.h>
#include "../tun_device.h"
#include "udp_wrapper.h"

using namespace Tins;
using namespace std;


const char LOCAL_IP_STR[] = "192.168.23.148";
const uint16_t LOCAL_PORT = 35000;
const char SERVER_IP_STR[] = "115.159.146.17";
const uint16_t SERVER_PORT = 35000;

const int TUN0 = 0;
const int UDP0 = 1;
const int EVENT_SIZE = 107;
const int BUF_SIZE = 100000;

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

    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, sockFd, &ev) == -1)
    {
        perror("epoll_ctl");
        exit(errno);
    }
}

int main(int argc, char *argv[])
{
    TunDecive tun0;
    tun0.up();
    tun0.addRoute("220.181.57.217");

    WrapperUdp udp(LOCAL_IP_STR, LOCAL_PORT, SERVER_IP_STR, SERVER_PORT);

    int epollFd;
    if((epollFd = epoll_create1(0)) == -1)
    {
        perror("epoll_create");
        exit(errno);
    }

    epollAdd(epollFd, tun0.getFd(), TUN0, EPOLLIN);
    epollAdd(epollFd, udp.getFd(), UDP0, EPOLLIN);

    struct epoll_event event[EVENT_SIZE];
    char buf[BUF_SIZE];
    size_t read_bytes;

    while(true)
    {
        int size = epoll_wait(epollFd, event, EVENT_SIZE-1, -1);

        for(int i = 0; i < size; ++ i)
        {
            int mk = ((struct markMsg * )event[i].data.ptr)->mk;
            int fd = ((struct markMsg * )event[i].data.ptr)->fd;
            if(mk == TUN0)
            {
                if((read_bytes = tun0.readMsg(buf, BUF_SIZE)) <= 0)
                {
                    perror("tun0.readMsg");
                    continue;
                }
                printf("tun0 %d\n", read_bytes);

                if(udp.sentMsg(buf, read_bytes) <= 0)
                {
                    perror("udp.sendMsg");
                    continue;
                }
            }
            else
            {
                if((read_bytes = udp.readMsg(buf, BUF_SIZE)) <= 0)
                {
                    perror("read");
                    continue;
                }
                printf("udp %d\n", read_bytes);

                if(tun0.sendMsg(buf, read_bytes) <= 0)
                {
                    perror("read");
                    continue;
                }
            }
        }
    }

    return 0;
}
