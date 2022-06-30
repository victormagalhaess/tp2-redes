#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define INPUT_ARGS 2
#define MAX_THREADS 15
#define MAX_CLIENTS 15
#define MAX_PENDING 5
#define BUFSIZE 500
#define EQUIPMENT_NONE 0

enum
{
    REQ_ADD_ID = 1,
    REQ_REM_ID,
    RES_ADD_ID,
    RES_LIST_ID,
    REQ_INF_ID,
    RES_INF_ID,
    ERROR_ID,
    OK_ID,
};

enum
{
    EQUIPMENT_NOT_FOUND = 1,
    SOURCE_EQUIPMENT_NOT_FOUND,
    TARGET_EQUIPMENT_NOT_FOUND,
    EQUIPMENT_LIMIT_EXCEEDED,
};

enum
{
    SUCCESFUL_REMOVAL = 1
};

int serverSock;
int numberOfThreads = 0;
int numberOfClients = 0;
int equipmentIdCounter = 1;
int equipmentsIds[MAX_CLIENTS] = {0};

void *ThreadMain(void *arg);

struct ThreadArgs
{
    socklen_t clientLen;
    struct sockaddr_in clientCon;
    char buffer[BUFSIZE];
};

struct Message
{
    int IdMsg;
    int IdOrigin;
    int IdDestination;
    int Payload;
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

void assembleMessage(char *buffer, struct Message *message)
{
    char finalMessage[BUFSIZE] = "";
    char partialMessage[BUFSIZE] = "";
    if (message->IdMsg)
    {
        sprintf(partialMessage, "%d ", message->IdMsg);
        strcat(finalMessage, partialMessage);
    }
    if (message->IdOrigin)
    {
        sprintf(partialMessage, "%d ", message->IdOrigin);
        strcat(finalMessage, partialMessage);
    }
    if (message->IdDestination)
    {
        sprintf(partialMessage, "%d ", message->IdDestination);
        strcat(finalMessage, partialMessage);
    }
    if (message->Payload)
    {
        sprintf(partialMessage, "%d", message->Payload);
        strcat(finalMessage, partialMessage);
    }
    strcpy(buffer, finalMessage);
}

void buildERROR(char *buffer, int idDestination, int payload)
{
    struct Message message;
    message.IdMsg = ERROR_ID;
    message.IdOrigin = EQUIPMENT_NONE;
    message.IdDestination = idDestination;
    message.Payload = payload;
    assembleMessage(buffer, &message);
}

void buildRESADD(char *buffer)
{

    struct Message message;
    message.IdMsg = RES_ADD_ID;
    message.IdOrigin = EQUIPMENT_NONE;
    message.IdDestination = EQUIPMENT_NONE;
    message.Payload = equipmentIdCounter;
    equipmentsIds[numberOfClients] = equipmentIdCounter;
    equipmentIdCounter++;
    assembleMessage(buffer, &message);
}

void buildOK(char *buffer, int idOrigin, int status)
{
    struct Message message;
    message.IdMsg = OK_ID;
    message.IdOrigin = idOrigin;
    message.IdDestination = EQUIPMENT_NONE;
    message.Payload = status;
    assembleMessage(buffer, &message);
}

int IdentifyMessage(char *buffer)
{
    char *command = strtok(buffer, " ");
    int id = atoi(command);
    return id;
}

void AddEquipment(char *response)
{
    buildRESADD(response);
}

void RemoveEquipment(char *response)
{
    char *command = strtok(NULL, " ");
    int originId = atoi(command);
    int deleted = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) // logical removal of equipment
    {
        if (equipmentsIds[i] == originId)
        {
            equipmentsIds[i] = equipmentsIds[numberOfClients];
            equipmentsIds[numberOfClients] = 0;
            deleted = 1;
        }
    }
    if (deleted)
    {
        buildOK(response, originId, SUCCESFUL_REMOVAL);
    }
    else
    {
        buildERROR(response, EQUIPMENT_NONE, EQUIPMENT_NOT_FOUND);
    }
}

void *ThreadMain(void *args)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    int idMessage = IdentifyMessage(threadArgs->buffer);
    char response[BUFSIZE] = "";
    switch (idMessage)
    {
    case REQ_ADD_ID:
        numberOfClients++;
        AddEquipment(response);
        break;
    case REQ_REM_ID:
        numberOfClients--;
        RemoveEquipment(response);
        break;
    case REQ_INF_ID:
        // GetEquipmentInfo(threadArgs->buffer);
        break;
    }

    char *message = threadArgs->buffer;
    int connection_id = threadArgs->clientCon.sin_port;
    struct sockaddr_in from = threadArgs->clientCon;
    char *ip = inet_ntoa(from.sin_addr);
    printf("%s:%d: %s\n", ip, connection_id, message);
    int bytesSent = sendto(serverSock, response, strlen(response), 0, (struct sockaddr *)&threadArgs->clientCon, threadArgs->clientLen);
    validateCommunication(bytesSent);

    free(threadArgs);
    numberOfThreads--;
    return NULL;
}

int main(int argc, char const *argv[])
{
    validateInputArgs(argc, 2);
    char *port = strdup(argv[1]);
    serverSock = buildServerSocket(port);

    pthread_t threads[MAX_THREADS];

    for (;;)
    {
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        threadArgs->clientLen = sizeof(struct sockaddr_in);
        int bytesReceived = recvfrom(serverSock, threadArgs->buffer, BUFSIZE, 0, (struct sockaddr *)&threadArgs->clientCon, &threadArgs->clientLen);
        validateCommunication(bytesReceived);
        printf("numberOfClients: %d\n", numberOfClients);
        printf("%s\n", threadArgs->buffer);
        int firstReqId = IdentifyMessage(strdup(threadArgs->buffer));
        if (numberOfClients == MAX_CLIENTS - 1 && firstReqId == REQ_ADD_ID)
        {
            char response[BUFFER_SIZE_BYTES] = "";
            buildERROR(response, EQUIPMENT_NONE, EQUIPMENT_LIMIT_EXCEEDED);
            int bytesSent = sendto(serverSock, response, strlen(response), 0, (struct sockaddr *)&threadArgs->clientCon, threadArgs->clientLen);
            validateCommunication(bytesSent);
            continue;
        }

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