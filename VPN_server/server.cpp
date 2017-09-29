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
#include <list>
#include "../tun_device.h"
#include "udp_wrapper.h"

using namespace Tins;
using namespace std;

const char TUN0_IP[] = "192.168.100.100";
const char LOCAL_IP_STR[] = "";
const uint16_t UDP_SERVER_PORT = 35000;

const int EVENT_SIZE = 7;
const int BUF_SIZE = 10000;


unordered_map<uint64_t, uint16_t> srcIpPort_mp_localPort;
unordered_map<uint64_t, sockaddr_in> localPort_mp_srcAddr;
unordered_map<uint16_t, uint64_t> localPort_mp_clientIpPort;

list<uint16_t> be_use_port;

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
    for(uint16_t port = 35001; port < 40000; ++ port)
        be_use_port.push_back(port);

    TunDecive tun0;
    tun0.bindIp("192.168.100.0/24");
    tun0.up();

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

                uint16_t local_port;
                switch(ip_package.protocol())
                {
                    case 6:
                        local_port = ip_package.find_pdu<TCP>()->dport();
                        break;
                    case 17:
                        local_port = ip_package.find_pdu<UDP>()->dport();
                        break;
                    default:
                        local_port = 0;
                        break;
                }
                sockaddr_in src_addr = localPort_mp_srcAddr[local_port];
                uint64_t client_ip_port = localPort_mp_clientIpPort[local_port];

                uint32_t client_ip = (client_ip_port >> 32);
                uint16_t client_port = (client_ip_port & 0xffff);

                cout << "tun0" << endl;
                cout << "src_ip:" << src_addr.sin_addr.s_addr << " " << "src_port:" << src_addr.sin_port << endl;
                cout << "client_ip:" << client_ip << " " << "client_port:" << client_port << endl;

                ip_package.dst_addr(IPv4Address(client_ip));
                switch(ip_package.protocol())
                {
                    case 6:
                        ip_package.find_pdu<TCP>()->dport(client_port);
                        break;
                    case 17:
                        ip_package.find_pdu<UDP>()->dport(client_port);
                        break;
                    default:
                        break;
                }

                vector<uint8_t> ip_package_buf = ip_package.serialize();
                if(udp.sentMsg((char *)ip_package_buf.data(), ip_package_buf.size(), src_addr) <= 0)
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
                uint32_t src_ip = src_addr.sin_addr.s_addr;
                uint16_t src_port = src_addr.sin_port;

                uint32_t client_ip;
                if((client_ip = inet_addr(ip_package.src_addr().to_string().data())) == INADDR_NONE)
                {
                    perror("inet_addr");
                    exit(errno);
                }

                uint16_t client_port;
                switch (ip_package.protocol())
                {
                    case 6:
                        client_port = ip_package.find_pdu<TCP>()->sport();
                        break;
                    case 17:
                        client_port = ip_package.find_pdu<UDP>()->sport();
                        break;
                    default:
                        client_port = 0;
                        break;
                }

                cout << "udp0" << endl;
                cout << "src_ip:" << src_ip << " " << "src_port:" << src_port << endl;
                cout << "client_ip:" << client_ip << " " << "client_port;" << client_port << endl;

                if(client_port == 0)
                {
                    cout << "PORT = 0!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                    continue;
                }

                uint64_t src_ip_port = src_ip, client_ip_port = client_ip;
                src_ip_port = (src_ip_port << 32) + src_port;
                client_ip_port = (client_ip_port << 32) + client_port;

                uint64_t local_port;
                if(srcIpPort_mp_localPort.count(src_ip_port) > 0)
                    local_port = srcIpPort_mp_localPort[src_ip_port];
                else
                {
                    if(be_use_port.empty())
                    {
                        printf("No Port for use\n");
                        continue;
                    }

                    local_port = be_use_port.front();
                    be_use_port.pop_front();

                    srcIpPort_mp_localPort[src_ip_port] = local_port;
                }
                localPort_mp_srcAddr[local_port] = src_addr;
                localPort_mp_clientIpPort[local_port] = client_ip_port;

                ip_package.src_addr(IPv4Address(TUN0_IP));
                switch(ip_package.protocol())
                {
                    case 6:
                        ip_package.find_pdu<TCP>()->sport(local_port);
                        break;
                    case 17:
                        ip_package.find_pdu<UDP>()->sport(local_port);
                        break;
                    default:
                        break;
                }

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
