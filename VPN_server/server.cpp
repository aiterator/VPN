//
// Created by zy on 17-9-18.
//

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <tins/tins.h>
#include <thread>
#include <list>
#include "udp_wrapper.h"

using namespace std;
using namespace Tins;

const char IP_LOCAL[] = "115.159.146.17";
const int UDP_SERVER_PORT = 35000;
const int BUFF_SIZE = 100000;

unordered_map<uint16_t , string> serverPort_Mp_clientIpPort;
//list<int> useless_port;

void udpServer()
{
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket == -1)
    {
        perror("socket");
        exit(errno);
    }

    struct sockaddr_in udp_server;
    memset(&udp_server, 0, sizeof(udp_server));
    udp_server.sin_family = AF_INET;
    udp_server.sin_port = htons(UDP_SERVER_PORT);
    udp_server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(udp_socket, (struct sockaddr *)&udp_server, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(errno);
    }

//    for(int i = 35001; i < 40000; ++ i)
//        useless_port.push_back(i);

    char buf[BUFF_SIZE];
    struct sockaddr_in udp_client;
    socklen_t clientLen = sizeof(udp_client);
    size_t read_bytes;
    PacketSender sender;

    while(true)
    {
        if((read_bytes = recvfrom(udp_socket, buf, BUFF_SIZE, 0, (struct sockaddr *)&udp_client, &clientLen)) < 0)
        {
            perror("recvfrom");
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
        serverPort_Mp_clientIpPort[udp_client.sin_port] = ip_package.src_addr().to_string();

        ip_package.src_addr(IPv4Address(IP_LOCAL));
        sender.send(ip_package);
    }
}


bool handle(PDU &some_pdu)
{
    IP &ip = some_pdu.rfind_pdu<IP>();
    TCP &tcp = some_pdu.rfind_pdu<TCP>();

    uint16_t port = tcp.dport();
    string d_addr = serverPort_Mp_clientIpPort[port];
    ip.dst_addr(IPv4Address(d_addr.data()));

    WrapperUdp udp(d_addr.data(), port);
    auto buf = ip.serialize();
    if(udp.sentMsg((char *)buf.data(), buf.size()) <= 0)
        perror("udp.sendMsg");

    return true;
}


int main(int argc, char *argv[])
{
    thread udp_server_thread_id = thread(udpServer);

    SnifferConfiguration config;
    config.set_filter("ip dst 115.159.146.17");
    Sniffer sniffer("etho", config);
    sniffer.sniff_loop(handle);
}
