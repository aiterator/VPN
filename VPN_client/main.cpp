//
// Created by zy on 9/13/17.
//
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <cstdio>
#include <fcntl.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>
#include <tins/tins.h>
#include <iostream>

using namespace Tins;
using namespace std;

//bool handler(const PDU &pdu)
//{
//    const IP &ip = pdu.rfind_pdu<IP>();
//    const TCP &tcp = pdu.rfind_pdu<TCP>();
//    std::cout << ip.src_addr() << ':' << tcp.sport() << " -> "
//              << ip.dst_addr() << ':' << tcp.dport() << std::endl;
//    return true;
//}

void upDevice(const char* device_name)
{
    char buf[207];
    sprintf(buf, "sudo ip link set %s up", device_name);
    system(buf);
}

void downDevice(const char* device_name)
{
    char buf[207];
    sprintf(buf, "sudo ip link set %s down", device_name);
    system(buf);
}

void addRoute(const char* device_name, const char* ip_route)
{
    char buf[207];
    sprintf(buf, "sudo ip route add %s dev %s", ip_route, device_name);
    system(buf);
}

int main()
{
    int fd;
    if((fd = open("/dev/net/tun", O_RDWR)) == -1)
    {
        perror("open");
        exit(errno);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if(ioctl(fd, TUNSETIFF, &ifr) == -1)
    {
        perror("ioctl");
        exit(errno);
    }

    printf("ifname is %s\n", ifr.ifr_name);
    upDevice(ifr.ifr_name);
    addRoute(ifr.ifr_name, "123.123.123.123");

//    int n;
//    cin >> n;
}
