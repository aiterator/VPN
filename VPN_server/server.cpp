//
// Created by zy on 17-9-27.
//

//
// Created by zy on 9/13/17.
//
#include <iostream>
#include <sys/epoll.h>
#include <tins/tins.h>
#include <unordered_map>
#include <ctime>
#include <list>
#include "../tun_device.h"
#include "udp_wrapper.h"

using namespace Tins;
using namespace std;

const uint16_t UDP_SERVER_PORT = 35000;

const int EVENT_SIZE = 100;
const int BUF_SIZE = 10000;

unordered_map<IPv4Address, pair<pair<uint32_t, sockaddr_in>, time_t>> localIp_mp_clientMsg;
unordered_map<string, IPv4Address> client_mp_localIp;

void epollAdd(int epollFd, int sockFd, uint32_t status)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = status;
    ev.data.fd = sockFd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, sockFd, &ev) == - 1)
    {
        perror("epoll_ctl");
        exit(errno);
    }
}

int main(int argc, char *argv[])
{
    TunDecive tun0;
    tun0.bindIp("192.168.100.0/24");
    tun0.up();

    IPv4Range ip_pool = IPv4Address("192.168.100.0") / 24;
    for(IPv4Range::iterator it = ip_pool.begin(); it != ip_pool.end(); ++ it)
        localIp_mp_clientMsg[*it] = make_pair(make_pair(0, sockaddr_in()), 0);

    WrapperUdp udp(UDP_SERVER_PORT);

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
    ssize_t read_bytes;

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

                IP ip_package;
                try
                {
                    ip_package = RawPDU((uint8_t*)buf, read_bytes).to<IP>();
                }
                catch (...)
                {
                    continue;
                }

                IPv4Address local_ip = ip_package.dst_addr();
                pair<pair<uint32_t, sockaddr_in>, time_t> &clientMsg = localIp_mp_clientMsg[local_ip];
                clientMsg.second = time(NULL);

                ip_package.dst_addr(IPv4Address(clientMsg.first.first));
                vector<uint8_t> ip_package_buf = ip_package.serialize();
                if(udp.sentMsg((char *)ip_package_buf.data(), ip_package_buf.size(), clientMsg.first.second) <= 0)
                {
                    perror("udp.sendMsg");
                    continue;
                }
            }
            else
            {
                if((read_bytes = udp.readMsg(buf, BUF_SIZE)) <= 0)
                {
                    perror("udp.readMsg");
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

                sockaddr_in src_addr = udp.getSockaddr_in();
                uint32_t client_ip_uint32;
                if((client_ip_uint32 = inet_addr(ip_package.src_addr().to_string().data())) == INADDR_NONE)
                {
                    perror("inet_addr");
                    exit(errno);
                }

                string client((char*)&src_addr, sizeof(src_addr));
                client += string((char *)&client_ip_uint32, sizeof(client_ip_uint32));

                //cout << "udp0" << endl;
                //cout << "src_ip:" << src_ip << " " << "src_port:" << src_port << endl;
                //cout << "client_ip:" << client_ip << " " << "client_port;" << client_port << endl;

                IPv4Address local_ip;
                if(client_mp_localIp.count(client) > 0)
                {
                    local_ip = client_mp_localIp[client];
                    pair<pair<uint32_t, sockaddr_in>, time_t> &tmp = localIp_mp_clientMsg[local_ip];
                    tmp.first.second = src_addr, tmp.second = time(NULL);
                }
                else
                {
                    time_t now = time(NULL);
                    for(IPv4Range::iterator it = ip_pool.begin(); it != ip_pool.end(); ++ it)
                    {
                        pair<pair<uint32_t, sockaddr_in>, time_t> &tmp = localIp_mp_clientMsg[*it];
                        if(tmp.first.first == 0 || (now - tmp.second) > 300)
                        {
                            local_ip = *it;
                            client_mp_localIp[client] = local_ip;

                            tmp.first.first = client_ip_uint32, tmp.first.second = src_addr;
                            tmp.second = now;
                            break;
                        }
                    }
                }

                ip_package.src_addr(local_ip);
                vector<uint8_t > ip_package_buf = ip_package.serialize();
                if(tun0.sendMsg((char *)ip_package_buf.data(), ip_package_buf.size()) <= 0)
                {
                    perror("tun0.sendMsg");
                    continue;
                }
            }
        }
    }
    return 0;
}
