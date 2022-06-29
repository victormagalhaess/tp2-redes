#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define INPUT_ARGS 2
#define MAX_CLIENTS 15
#define MAX_PENDING 5
#define BUFSIZE 500

int buildServerSocket(char *portString)
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

int main(int argc, char const *argv[])
{

    validateInputArgs(argc, 2);
    char *port = strdup(argv[1]);
    int sock = buildServerSocket(port);

    char buffer[BUFFER_SIZE_BYTES] = "";
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    for (;;)
    {
        memset(buffer, 0, sizeof(buffer));
        int totalBytesRead = recvfrom(sock, buffer, BUFFER_SIZE_BYTES, 0, (struct sockaddr *)&client_address, &client_address_len);
        validateCommunication(totalBytesRead);
        buffer[totalBytesRead] = '\0';
        printf("%s\n", buffer);
        int totalBytesSent = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&client_address, client_address_len);
        validateCommunication(totalBytesSent);
    }
    return 0;
}