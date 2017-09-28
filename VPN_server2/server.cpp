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
#include "../udp_wrapper.h"

using namespace Tins;
using namespace std;

const char TUN0_IP[] = "192.168.100.100";
const char LOCAL_IP_STR[] = "";
const uint16_t UDP_SERVER_PORT = 35000;

const int EVENT_SIZE = 7;
const int BUF_SIZE = 10000;
uint32_t LOCAL_IP_UINT32;

unordered_map<uint64_t, uint64_t> dstIpPort_mp_srcIpPort, srcIpPort_mp_localIpPort;
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
    //tun0.addRoute(TUN0_IP);

    LOCAL_IP_UINT32 = inet_addr(LOCAL_IP_STR);
    if(inet_aton(LOCAL_IP_STR, (in_addr*)&LOCAL_IP_UINT32) == 0)
    {
        perror("inet_aton(local)");
        exit(errno);
    }

    WrapperUdp udp(LOCAL_IP_STR, UDP_SERVER_PORT);

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

                uint32_t dst_ip;
                if(inet_aton(ip_package.src_addr().to_string().data(), (in_addr*)&dst_ip) == 0)
                {
                    perror("inet_aton(src)");
                    continue;
                }

                uint16_t dst_port;
                switch(ip_package.protocol())
                {
                    case 6:
                        dst_port = ip_package.find_pdu<TCP>()->sport();
                        break;
                    case 17:
                        dst_port = ip_package.find_pdu<UDP>()->sport();
                        break;
                    default:
                        dst_port = 0;
                        break;
                }

                cout << "dst_ip:" << dst_ip << " " << "dst_port:" << dst_port << endl;

                uint64_t dst_ip_port = dst_ip;
                dst_ip_port = (dst_ip_port << 32) + dst_port;

                uint64_t src_ip_port = dstIpPort_mp_srcIpPort[dst_ip_port];

                ip_package.dst_addr(IPv4Address(src_ip_port >> 32));
                switch(ip_package.protocol())
                {
                    case 6:
                        ip_package.find_pdu<TCP>()->dport(src_ip_port & 0xffff);
                        break;
                    case 17:
                        ip_package.find_pdu<UDP>()->dport(src_ip_port & 0xffff);
                        break;
                    default:
                        break;
                }

                vector<uint8_t> ip_package_buf = ip_package.serialize();
                if(udp.sentMsg((char *)ip_package_buf.data(), ip_package_buf.size(), IPv4Address(src_ip_port >> 32).to_string().data(), (src_ip_port & 0xffff)) <= 0)
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

                uint32_t src_ip, dst_ip;
                if(inet_aton(ip_package.src_addr().to_string().data(), (in_addr*)&src_ip) == 0)
                {
                    perror("inet_aton(src)");
                    continue;
                }
                if(inet_aton(ip_package.dst_addr().to_string().data(), (in_addr*)&dst_ip) == 0)
                {
                    perror("inet_aton(dst)");
                    continue;
                }

                uint16_t src_port, dst_port;
                switch(ip_package.protocol())
                {
                    case 6:
                        src_port = ip_package.find_pdu<TCP>()->sport();
                        dst_port = ip_package.find_pdu<TCP>()->dport();
                        break;
                    case 17:
                        src_port = ip_package.find_pdu<UDP>()->sport();
                        dst_port = ip_package.find_pdu<UDP>()->dport();
                        break;
                    default:
                        src_port = 0;
                        dst_port = 0;
                        break;
                }

                cout << "src_ip:" << src_ip << " " << "src_port:" << src_port << endl;
                cout << "dst_ip:" << dst_ip << " " << "dst_port" << dst_port << endl;

                uint64_t src_ip_port = src_ip, dst_ip_port = dst_ip;
                src_ip_port = (src_ip_port << 32) + src_port;
                dst_ip_port = (dst_ip_port << 32) + dst_port;

                uint64_t local_ip_port;
                if(srcIpPort_mp_localIpPort.count(src_ip_port) > 0)
                    local_ip_port = srcIpPort_mp_localIpPort[src_ip_port];
                else
                {
                    if(be_use_port.empty())
                    {
                        printf("No Port for use\n");
                        continue;
                    }

                    uint16_t local_port = 0;
                    if(src_port != 0)
                    {
                        local_port = be_use_port.front();
                        be_use_port.pop_front();
                    }

                    local_ip_port = LOCAL_IP_UINT32;
                    local_ip_port = (local_ip_port << 32) + local_port;

                    srcIpPort_mp_localIpPort[src_ip_port] = local_ip_port;
                }
                dstIpPort_mp_srcIpPort[dst_port] = src_ip_port;

                ip_package.src_addr(IPv4Address(TUN0_IP));
                switch(ip_package.protocol())
                {
                    case 6:
                        ip_package.find_pdu<TCP>()->sport((local_ip_port & 0xffff));
                        break;
                    case 17:
                        ip_package.find_pdu<UDP>()->sport((local_ip_port & 0xffff));
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
