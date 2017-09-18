//
// Created by zy on 17-9-14.
//

#include "tun_device.h"

TunDecive::TunDecive()
{
    if((fd = open("/dev/net/tun", O_RDWR)) == -1)
    {
        perror("open");
        exit(errno);
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, "", IFNAMSIZ);

    if(ioctl(fd, TUNSETIFF, &ifr) == -1)
    {
        perror("ioctl");
        exit(errno);
    }

    printf("tunDeviceName is %s\n", ifr.ifr_name);
}

void TunDecive::up()
{
    char buf[207];
    sprintf(buf, "sudo ip link set %s up", ifr.ifr_name);
    system(buf);

    printf("tunDevice %s open\n", ifr.ifr_name);
}

void TunDecive::down()
{
    char buf[207];
    sprintf(buf, "sudo ip link set %s down", ifr.ifr_name);
    system(buf);

    printf("tunDevice %s down\n", ifr.ifr_name);
}

void TunDecive::addRoute(const char *route)
{
    char buf[207];
    sprintf(buf, "sudo ip route add %s dev %s", route, ifr.ifr_name);
    system(buf);
}

char* TunDecive::name()
{
    return ifr.ifr_name;
}

int TunDecive::getFd()
{
    return fd;
}

