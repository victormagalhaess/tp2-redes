#ifndef COMMON_NET_TP1
#define COMMON_NET_TP1

#define BUFFER_SIZE_BYTES 500
#define MIN_PORT_VALUE 1025
#define MAX_PORT_VALUE 65535
#define ADDR_LEN 16
#define MAX_CLIENTS 15

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// messages enum
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
    REQ_LIST_ID,
};

// errors enum
enum
{
    EQUIPMENT_NONE = 0,
    EQUIPMENT_NOT_FOUND,
    SOURCE_EQUIPMENT_NOT_FOUND,
    TARGET_EQUIPMENT_NOT_FOUND,
    EQUIPMENT_LIMIT_EXCEEDED,
};

// successes enum
enum
{
    SUCCESFUL_REMOVAL = 1
};

struct Message
{
    int IdMsg;
    int IdOrigin;
    int IdDestination;
    int Payload;
};

void dieWithMessage(char *message);
void validateInputArgs(int argc, int minArgs);
void validateCommunication(int valread);
int getPort(char *port);
int buildUDPSocket(char *portString);
int sendUdpMessage(int originSock, char *response, struct sockaddr_in *targetSock);
void assembleMessage(char *buffer, struct Message *message);
int IdentifyMessage(char *buffer);

#endif