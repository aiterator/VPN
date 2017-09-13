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

const int BUF_SIZE = 4096;
const char serverIp[20] = "101.254.136.59";

void Send_udp(char *buf, int len)
{
    return ;
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


    int n;
    char buf[BUF_SIZE];

    while(true)
    {
        if((n = read(fd, buf, BUF_SIZE)) == -1)
        {
            perror("read");
            exit(errno);
        }

        Send_udp(buf, n);
    }
}