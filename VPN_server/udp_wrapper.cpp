//
// Created by zy on 17-9-15.
//

#include "udp_wrapper.h"

WrapperUdp::WrapperUdp(const char* local_ip, uint16_t local_port)
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(errno);
    }

    memset(&client, 0, sizeof(client));
    if (inet_aton(local_ip, &(client.sin_addr)) == -1)
    {
        perror("inet_aton");
        exit(errno);
    }
    client.sin_port = htons(local_port);
    client.sin_family = AF_INET;
    if(bind(sockfd, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        perror("bind");
        exit(errno);
    }
}

ssize_t WrapperUdp::sentMsg(const char* msg, int len, const sockaddr_in &Client)
{
    return sendto(sockfd, msg, len, 0, (struct sockaddr *)&Client, sizeof(Client));
}

ssize_t WrapperUdp::readMsg(char* buf, int BUF_SIZE)
{
    socklen_t client_length = sizeof(client);
    return recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *)&client, &client_length);
}

sockaddr_in WrapperUdp::getSockaddr_in()
{
    return client;
}

int WrapperUdp::getFd()
{
    return sockfd;
}