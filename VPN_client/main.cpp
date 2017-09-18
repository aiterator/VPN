//
// Created by zy on 9/13/17.
//
#include <iostream>
#include <tins/tins.h>
#include "tun_device.h"
#include "udp_wrapper.h"

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

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("usage: %s <serverIp> <serverPort>\n", argv[0]);
        //return 0;
    }

    TunDecive tun0;
    tun0.up();
    tun0.addRoute("10.0.0.122");

    WrapperUdp udp(argv[1], argv[2]);
    
}
