// Client side C/C++ program to demonstrate Socket
// programming
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN 0 // file descriptor for standard input

#define BUFSIZE 500

int clientSock;
socklen_t clientLen = sizeof(struct sockaddr_in);

struct ThreadArgs
{
    struct sockaddr_in serverAddr;
};

int nonBlockRead(char *message)
{
    struct timeval tv;
    fd_set readfds;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);

    select(STDIN + 1, &readfds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN, &readfds))
    {
        read(STDIN, message, BUFSIZE - 1);
        return 1;
    }
    else
        fflush(stdout);
    return 0;
}

void readMessage(char *message)
{
    fgets(message, BUFSIZE - 1, stdin);
}

void *ReceiveThread(void *data)
{
    struct ThreadArgs *threadData = (struct ThreadArgs *)data;
    while (1)
    {
        char buffer[BUFSIZE];
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(clientSock, buffer, BUFSIZE, 0, (struct sockaddr *)&threadData->serverAddr, &clientLen);
        validateCommunication(bytesReceived);
        printf("%s", buffer);
    }
    free(threadData);
    pthread_exit(NULL);
}

void *SendThread(void *data)
{
    struct ThreadArgs *threadData = (struct ThreadArgs *)data;
    while (1)
    {
        char buffer[BUFSIZE];
        memset(buffer, 0, sizeof(buffer));
        if (nonBlockRead(buffer))
        {
            int totalBytesSent = sendUdpMessage(clientSock, buffer, &threadData->serverAddr);
            validateCommunication(totalBytesSent);
        }
    }
    free(threadData);
    pthread_exit(NULL);
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

    pthread_t receiveThread;
    struct ThreadArgs *receiveThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    receiveThreadArgs->serverAddr = serverAddr;
    int receiveThreadStatus = pthread_create(&receiveThread, NULL, ReceiveThread, (void *)receiveThreadArgs);
    if (receiveThreadStatus != 0)
    {
        dieWithMessage("pthread_create failed");
    }

    pthread_t sendThread;
    struct ThreadArgs *sendThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    sendThreadArgs->serverAddr = serverAddr;
    int sendThreadStatus = pthread_create(&sendThread, NULL, SendThread, (void *)sendThreadArgs);
    if (sendThreadStatus != 0)
    {
        dieWithMessage("pthread_create failed");
    }

    pthread_join(receiveThread, NULL);
    pthread_join(sendThread, NULL);

    return 0; // never reached
}
