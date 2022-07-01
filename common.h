#ifndef COMMON_NET_TP1
#define COMMON_NET_TP1
#define BUFFER_SIZE_BYTES 500
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void dieWithMessage(char *message);
void validateInputArgs(int argc, int minArgs);
void validateCommunication(int valread);
int getPort(char *port);
int buildUDPSocket(char *portString);
int sendUdpMessage(int originSock, char *response, struct sockaddr_in *targetSock);

#endif