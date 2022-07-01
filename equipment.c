// Client side C/C++ program to demonstrate Socket
// programming
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define BUFSIZE 500

int clientSock;
socklen_t clientLen = sizeof(struct sockaddr_in);

void readMessage(char *message)
{
    fflush(stdin);
    scanf("%[^\n]%*c", message);
    return;
}

int main(int argc, char const *argv[])
{
    validateInputArgs(argc, 3);
    char *serverIP = strdup(argv[1]);
    char *serverPort = strdup(argv[2]);
    int serverPortNumber = getPort(serverPort);
    clientSock = buildUDPSocket("0");
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPortNumber);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    char buffer[BUFFER_SIZE_BYTES] = {0};

    for (;;)
    {
        readMessage(buffer);
        int totalBytesSent = sendUdpMessage(clientSock, buffer, &serverAddr);
        validateCommunication(totalBytesSent);
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(clientSock, buffer, BUFSIZE, 0, (struct sockaddr *)&serverAddr, &clientLen);
        validateCommunication(bytesReceived);
        printf("%s\n", buffer);
    }
    return 0; // never reached
}