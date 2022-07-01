#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define MIN_PORT_VALUE 1025
#define MAX_PORT_VALUE 65535
#define ADDR_LEN 16

void dieWithMessage(char *message)
{
    printf("%s\n", message);
    exit(-1);
}

void validateInputArgs(int argc, int minArgs)
{
    if (argc < minArgs)
    {
        dieWithMessage("Invalid number of arguments");
    }
}

void validateCommunication(int status)
{
    if (status < 1)
    {
        dieWithMessage("Error during communication");
    }
}

int getPort(char *portString)
{
    int port = atoi(portString);
    if ((port >= MIN_PORT_VALUE && port <= MAX_PORT_VALUE) || port == 0)
    {
        return port;
    }
    dieWithMessage("Port in invalid range. You must use a non-root valid Unix port betwheen 1025 and 65535");
    return -1; // never reached
}

int buildUDPSocket(char *portString)
{

    int sock;
    int opt = 1;

    struct sockaddr_in address;

    int domain = AF_INET;
    int port = getPort(portString);

    if ((sock = socket(domain, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        dieWithMessage("socket failed");
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        dieWithMessage("setsockopt failed");
    }

    address.sin_family = domain;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    struct sockaddr *conAddress = (struct sockaddr *)&address;
    int conSize = sizeof(address);

    if (bind(sock, conAddress, conSize) < 0)
    {
        dieWithMessage("bind failed");
    }

    return sock;
}

int sendUdpMessage(int originSock, char *response, struct sockaddr_in *targetSock)
{
    return sendto(originSock, response, strlen(response), 0, (struct sockaddr *)targetSock, ADDR_LEN);
}