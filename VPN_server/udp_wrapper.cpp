//
// Created by zy on 17-9-15.
//

#include "udp_wrapper.h"

WrapperUdp::WrapperUdp(const char *ip, const char *port) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(errno);
    }

    memset(&client, 0, sizeof(client));
    if (inet_aton(ip, &(client.sin_addr)) == -1) {
        perror("inet_aton");
        exit(errno);
    }

    uint16_t Port = 0;
    for (int i = 0; port[i]; ++i)
        Port = Port * 10 + port[i] - '0';

    client.sin_port = htons(Port);
    client.sin_family = AF_INET;
}

WrapperUdp::WrapperUdp(const char *ip, uint16_t port) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(errno);
    }

    memset(&client, 0, sizeof(client));
    if (inet_aton(ip, &(client.sin_addr)) == -1) {
        perror("inet_aton");
        exit(errno);
    }

    client.sin_port = htons(port);
    client.sin_family = AF_INET;
}

size_t WrapperUdp::sentMsg(const char* msg, int len)
{
    return sendto(sockfd, msg, len, 0, (struct sockaddr *)&client, sizeof(client));
}

size_t WrapperUdp::sentMsg(std::string& msg)
{
    return sendto(sockfd, msg.data(), msg.size(), 0, (struct sockaddr *)&client, sizeof(client));
}

ssize_t WrapperUdp::readMsg(char* buf, int BUF_SIZE)
{
    socklen_t server_length = sizeof(server);
    ssize_t read_bytes = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *)&server, &server_length);
    return read_bytes;
}

int WrapperUdp::getFd()
{
    return sockfd;
}