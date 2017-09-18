//
// Created by zy on 17-9-14.
//

#ifndef VPN_TUNDEVICE_H
#define VPN_TUNDEVICE_H
#include <string>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>

class TunDecive
{
public:
    explicit TunDecive();
    void up();
    void down();
    void addRoute(const char* route);
    char* name();
    int getFd();

private:
    struct ifreq ifr;
    int fd; //描述符
};

#endif //VPN_TUNDEVICE_H
