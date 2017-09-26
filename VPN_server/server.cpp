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

unordered_map<uint16_t , uint64_t> serverPort_mp_clientIpPort;
unordered_map<uint64_t , uint16_t> clientIpPort_mp_serverPort;

list<uint16_t> useless_port;

void udpServer()
{
    for(uint16_t port = 35001; port < 40000; ++ port)
        useless_port.push_back(port);

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

        uint32_t src_ip = udp_client.sin_addr.s_addr;
        uint16_t src_port = udp_client.sin_port;

        uint64_t src_ip_port = src_ip;
        src_ip_port = (src_ip_port << 32) + src_port;

        uint16_t server_port;
        if(clientIpPort_mp_serverPort.count(src_ip_port) > 0)
            server_port = clientIpPort_mp_serverPort[src_ip_port];
        else
        {
            if(useless_port.empty())
            {
                printf("No Port for use\n");
                continue;
            }

            server_port = useless_port.front();
            useless_port.pop_front();

            clientIpPort_mp_serverPort[src_ip_port] = server_port;
            serverPort_mp_clientIpPort[server_port] = src_ip_port;
        }

        ip_package.src_addr(IPv4Address(IP_LOCAL));
        switch(ip_package.protocol())
        {
            case 6:
                ip_package.find_pdu<TCP>()->dport(server_port);
                break;
            case 17:
                ip_package.find_pdu<UDP>()->dport(server_port);
                break;
            default:
                break;
        }
        sender.send(ip_package);
    }
}


bool handle(PDU &some_pdu)
{
    IP &ip = some_pdu.rfind_pdu<IP>();
    TCP &tcp = some_pdu.rfind_pdu<TCP>();

    uint16_t local_port = tcp.dport();
    uint64_t client_ip_port = serverPort_mp_clientIpPort[local_port];
    uint32_t client_ip = client_ip_port >> 32;
    uint16_t client_port = (client_ip_port & 0xffff);

    ip.dst_addr(IPv4Address(client_ip));
    switch(ip.protocol())
    {
        case 6:
            ip.find_pdu<TCP>()->dport(client_port);
            break;
        case 17:
            ip.find_pdu<UDP>()->dport(client_port);
            break;
        default:
            break;
    }
    WrapperUdp udp(client_ip, client_port);
    auto buf = ip.serialize();
    if(udp.sentMsg((char *)buf.data(), buf.size()) <= 0)
        perror("udp.sendMsg");

    return true;
}


int main(int argc, char *argv[])
{
    thread udp_server_thread_id = thread(udpServer);

    SnifferConfiguration config;
    config.set_filter("ip dst 115.159.146.17 and port 35001-39999");
    config.set_immediate_mode(true);
    Sniffer sniffer("eth0", config);
    sniffer.sniff_loop(handle);
}
