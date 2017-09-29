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

const char SERVER_IP_STR[] = "";
const uint16_t SERVER_PORT = 35000;

const int EVENT_SIZE = 107;
const int BUF_SIZE = 100000;

void epollAdd(int epollFd, int sockFd, uint32_t status)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = status;
    ev.data.fd = sockFd;

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

    WrapperUdp udp;
    udp.setServerIpPort(SERVER_IP_STR, SERVER_PORT);

    int epollFd;
    if((epollFd = epoll_create1(0)) == -1)
    {
        perror("epoll_create");
        exit(errno);
    }

    epollAdd(epollFd, tun0.getFd(), EPOLLIN);
    epollAdd(epollFd, udp.getFd(), EPOLLIN);

    struct epoll_event event[EVENT_SIZE];
    char buf[BUF_SIZE];
    size_t read_bytes;

    while(true)
    {
        int size = epoll_wait(epollFd, event, EVENT_SIZE-1, -1);

        for(int i = 0; i < size; ++ i)
        {
            if(event[i].data.fd == tun0.getFd())
            {
                if((read_bytes = tun0.readMsg(buf, BUF_SIZE)) <= 0)
                {
                    perror("tun0.readMsg");
                    continue;
                }
                //printf("tun0 %d\n", read_bytes);

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
                //printf("udp %d\n", read_bytes);

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
