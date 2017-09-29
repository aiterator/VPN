//
// Created by zy on 17-9-15.
//

#include "udp_wrapper.h"


WrapperUdp::WrapperUdp()
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(errno);
    }
}

void WrapperUdp::setServerIpPort(const char* server_ip, uint16_t server_port)
{
    memset(&server, 0, sizeof(server));
    if (inet_aton(server_ip, &(server.sin_addr)) == -1)
    {
        perror("inet_aton");
        exit(errno);
    }
    server.sin_port = htons(server_port);
    server.sin_family = AF_INET;
}

ssize_t WrapperUdp::sentMsg(const char* msg, int len)
{
    return sendto(sockfd, msg, len, 0, (struct sockaddr *)&server, sizeof(server));
}

ssize_t WrapperUdp::readMsg(char* buf, int BUF_SIZE)
{
    sockaddr_in tmp;
    socklen_t client_length = sizeof(tmp);
    return recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *)&tmp, &client_length);
}

int WrapperUdp::getFd()
{
    return sockfd;
}