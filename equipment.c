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
#include <unistd.h>

#define STDIN 0 // file descriptor for standard input
#define BUFSIZE 500

enum
{
    LIST_EQUIPMENT = 1,
    DISCONNECT_EQUIPMENT,
    GET_INFO,
};

int equipmentId = 0;
int equipments[15];
int clientSock, clientBroadcastSock;
socklen_t clientLen = sizeof(struct sockaddr_in);

struct ThreadArgs
{
    struct sockaddr_in serverAddr;
};

void buildREQADD(char *buffer)
{
    struct Message message;
    message.IdMsg = REQ_ADD_ID;
    message.IdOrigin = 0;
    message.IdDestination = 0;
    message.Payload = 0;
    assembleMessage(buffer, &message);
}

void buildREQREM(char *buffer)
{
    struct Message message;
    message.IdMsg = REQ_REM_ID;
    message.IdOrigin = equipmentId;
    message.IdDestination = 0;
    message.Payload = 0;
    assembleMessage(buffer, &message);
}

void buildREQINFO(char *buffer, int destinationID)
{
    struct Message message;
    message.IdMsg = REQ_INF_ID;
    message.IdOrigin = equipmentId;
    message.IdDestination = destinationID;
    message.Payload = 0;
    assembleMessage(buffer, &message);
}

void buildRESINFO(char *buffer, int idOrigin, int idDestination)
{
    struct Message message;
    message.IdMsg = RES_INF_ID;
    message.IdOrigin = idDestination;
    message.IdDestination = idOrigin;
    message.Payload = 0;
    assembleMessage(buffer, &message);
    buffer[strlen(buffer) - 1] = '\0';
    sprintf(buffer, "%s%d.%d%d\n", buffer, rand() % 9, rand() % 9, rand() % 9); // nice little trick to fake a random decimal number
}

void RequestAdd(struct sockaddr_in serverAddr)
{
    char message[BUFSIZE];
    buildREQADD(message);
    int totalBytesSent = sendUdpMessage(clientSock, message, &serverAddr);
    validateCommunication(totalBytesSent);
}

void RequestRemove(struct sockaddr_in serverAddr)
{
    char message[BUFSIZE];
    buildREQREM(message);
    int totalBytesSent = sendUdpMessage(clientSock, message, &serverAddr);
    validateCommunication(totalBytesSent);
}

void RequestInfo(int targetEquipmentId, struct sockaddr_in serverAddr)
{
    char message[BUFSIZE];
    buildREQINFO(message, targetEquipmentId);
    int totalBytesSent = sendUdpMessage(clientSock, message, &serverAddr);
    validateCommunication(totalBytesSent);
}

void RespondInfo(int originEquipmentId, int targetEquipmentId, struct sockaddr_in serverAddr)
{
    char message[BUFSIZE];
    buildRESINFO(message, originEquipmentId, targetEquipmentId);
    int totalBytesSent = sendUdpMessage(clientSock, message, &serverAddr);
    validateCommunication(totalBytesSent);
}

void ListEquipments()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (equipments[i] != 0)
        {
            printf("%d ", equipments[i]);
        }
    }
    printf("\n");
}

int parseCommand(char *command, void *targetEquipmentID)
{
    int *targetEquipmentIDPtr = (int *)targetEquipmentID;
    if (strcmp(command, "list equipment\n") == 0)
    {
        return LIST_EQUIPMENT;
    }
    if (strcmp(command, "close connection\n") == 0)
    {
        return DISCONNECT_EQUIPMENT;
    }
    if (strcmp(strtok(command, " "), "request") == 0)
    {
        strtok(NULL, " "); // information
        strtok(NULL, " "); // from
        if (targetEquipmentIDPtr != NULL)
        {
            *targetEquipmentIDPtr = atoi(strtok(NULL, " ")); // got the id
        }
        return GET_INFO;
    }
    return 0;
}

void executeCommand(int command, int targetEquipmentID, struct sockaddr_in serverAddr)
{
    switch (command)
    {
    case LIST_EQUIPMENT:
        ListEquipments();
        break;
    case DISCONNECT_EQUIPMENT:
        RequestRemove(serverAddr);
        break;
    case GET_INFO:
        RequestInfo(targetEquipmentID, serverAddr);
        break;
    }
}

// this code was actually an adaptation of beejs code to read sockets in a non-blocking way
// I just made it work with stdin as file descriptor. The time choosen for the timeout is completely arbitrary, hope it works.
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

void processERRORID()
{
    int errorCode = atoi(strtok(NULL, " "));
    switch (errorCode)
    {
    case EQUIPMENT_NOT_FOUND:
        printf("Equipment not found\n");
        break;
    case SOURCE_EQUIPMENT_NOT_FOUND:
        printf("Source equipment not found\n");
        break;
    case TARGET_EQUIPMENT_NOT_FOUND:
        printf("Target equipment not found\n");
        break;
    case EQUIPMENT_LIMIT_EXCEEDED:
        dieWithMessage("Equipment limit exceeded\n");
        break;
    }
}

void processRESADDID()
{
    equipmentId = atoi(strtok(NULL, " "));
    printf("New ID: %d\n", equipmentId);
}

void processBroadcastRESADDID()
{
    int newEquipment = atoi(strtok(NULL, " "));
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (equipments[i] == newEquipment)
        {
            return; // somehow the equipment is already in the list
        }
        if (equipments[i] == 0)
        {
            equipments[i] = newEquipment;
            printf("Equipment %d added\n", equipments[i]);
            return;
        }
    }
}

void processBroadcastREQREMID()
{
    int equipmentId = atoi(strtok(NULL, " "));
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (equipments[i] == equipmentId)
        {
            equipments[i] = 0;
            printf("Equipment %d removed\n", equipmentId);
            return;
        }
    }
}

void processRESLISTID()
{
    int i = 0;
    char *tok;
    while ((tok = strtok(NULL, " ")) != NULL)
    {
        equipments[i] = atoi(tok);
        i++;
    }
}

void processOKID()
{
    dieWithMessage("Successful removal\n");
}

void processREQINFID(struct sockaddr_in serverAddr)
{
    int originId = atoi(strtok(NULL, " "));
    int destinationId = atoi(strtok(NULL, " "));
    RespondInfo(originId, destinationId, serverAddr);
}

void processRESINFID()
{
    int originId = atoi(strtok(NULL, " "));
    strtok(NULL, " ");
    char *payload = strtok(NULL, " ");
    printf("Value from %d: %s", originId, payload);
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
        buffer[bytesReceived] = '\0';
        int status = IdentifyMessage(buffer);

        switch (status)
        {
        case ERROR_ID:
            processERRORID();
            break;
        case RES_ADD_ID:
            processRESADDID();
            break;
        case RES_LIST_ID:
            processRESLISTID();
            break;
        case OK_ID:
            processOKID();
            break;
        case REQ_INF_ID:
            processREQINFID(threadData->serverAddr);
            break;
        case RES_INF_ID:
            processRESINFID();
            break;
        }
    }
    free(threadData);
    pthread_exit(NULL);
}

void *ReceiveBroadcastThread(void *data)
{
    struct ThreadArgs *threadData = (struct ThreadArgs *)data;
    while (1)
    {
        char buffer[BUFSIZE];
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(clientBroadcastSock, buffer, BUFFER_SIZE_BYTES, 0, (struct sockaddr *)&threadData->serverAddr, &clientLen);
        validateCommunication(bytesReceived);
        buffer[bytesReceived] = '\0';
        int status = IdentifyMessage(buffer);
        fflush(stdout); // Somehow, this line is really crucial to the execution of the code, even so I don't know why is it.
        switch (status)
        {
        case RES_ADD_ID:
            processBroadcastRESADDID();
            break;
        case REQ_REM_ID:
            processBroadcastREQREMID();
        }
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
        int targetEquipmentID = 0;
        if (nonBlockRead(buffer))
        {
            int command = parseCommand(strdup(buffer), &targetEquipmentID);
            executeCommand(command, targetEquipmentID, threadData->serverAddr);
            int totalBytesSent = sendUdpMessage(clientSock, buffer, &threadData->serverAddr);
            validateCommunication(totalBytesSent);
        }
    }
    free(threadData);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    srand(time(0));
    validateInputArgs(argc, 3);
    char *serverIP = strdup(argv[1]);
    char *serverPort = strdup(argv[2]);
    int serverPortNumber = getPort(serverPort);
    clientSock = buildUDPSocket(0, UNICAST);
    clientBroadcastSock = buildUDPSocket(serverPortNumber + 1, UNICAST);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPortNumber);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    struct sockaddr_in broadcastServerAddr;
    broadcastServerAddr.sin_family = AF_INET;
    broadcastServerAddr.sin_port = htons(serverPortNumber + 1);
    broadcastServerAddr.sin_addr.s_addr = inet_addr(serverIP);

    RequestAdd(serverAddr);

    pthread_t receiveThread;
    struct ThreadArgs *receiveThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    receiveThreadArgs->serverAddr = serverAddr;
    int receiveThreadStatus = pthread_create(&receiveThread, NULL, ReceiveThread, (void *)receiveThreadArgs);
    if (receiveThreadStatus != 0)
    {
        dieWithMessage("pthread_create failed");
    }

    pthread_t receiveBroadcastThread;
    struct ThreadArgs *receiveBroadcastThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    receiveBroadcastThreadArgs->serverAddr = broadcastServerAddr;
    int receiveBroadcastThreadStatus = pthread_create(&receiveBroadcastThread, NULL, ReceiveBroadcastThread, (void *)receiveBroadcastThreadArgs);
    if (receiveBroadcastThreadStatus != 0)
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
    pthread_join(receiveBroadcastThread, NULL);
    pthread_join(sendThread, NULL);

    return 0; // never reached
}
