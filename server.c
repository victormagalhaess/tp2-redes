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
#define MAX_THREADS 15
#define MAX_PENDING 5
#define BUFSIZE 500

int sock;
int numberOfThreads = 0;

void *ThreadMain(void *arg);

struct ThreadArgs
{
    socklen_t clientLen;
    struct sockaddr_in clientCon;
    char buf[BUFSIZE];
};

struct Thread
{
    int connection_id;
    struct Thread *next_thread;
};

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

void *ThreadMain(void *req)
{
    struct ThreadArgs *request = (struct ThreadArgs *)req;
    char *message = request->buf;
    int connection_id = request->clientCon.sin_port;
    struct sockaddr_in from = request->clientCon;
    char *ip = inet_ntoa(from.sin_addr);
    printf("%s:%d: %s\n", ip, connection_id, message);
    char *response = "OK\n";
    int bytesSent = sendto(sock, response, strlen(response), 0, (struct sockaddr *)&request->clientCon, request->clientLen);
    validateCommunication(bytesSent);
    return NULL;
}

int main(int argc, char const *argv[])
{
    validateInputArgs(argc, 2);
    char *port = strdup(argv[1]);
    sock = buildServerSocket(port);

    pthread_t threads[MAX_THREADS];

    for (;;)
    {
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        threadArgs->clientLen = sizeof(struct sockaddr_in);
        int bytesReceived = recvfrom(sock, threadArgs->buf, BUFSIZE, 0, (struct sockaddr *)&threadArgs->clientCon, &threadArgs->clientLen);
        validateCommunication(bytesReceived);

        int threadStatus = pthread_create(&threads[numberOfThreads], NULL, ThreadMain, (void *)threadArgs);
        if (threadStatus != 0)
        {
            dieWithMessage("pthread_create failed");
        }
        else
        {
            numberOfThreads++;
        }
    }
    return 0;
}