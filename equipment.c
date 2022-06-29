// Client side C/C++ program to demonstrate Socket
// programming
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* This section is responsible to just read the commands from the console.*/

void readMessage(char *message)
{
    fflush(stdin);
    scanf("%[^\n]%*c", message);
    return;
}

/* All the socket creation logic is segregated in the following function.
It receives the ip of the server (expected to be an valid ipv4 or ipv6) and the port,
builds the socket and returns the int that represents the socket connection */

int buildClientSocket(int argc, char const *argv[])
{
    validateInputArgs(argc, 3);
    int sock = 0;
    int domain;
    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv_addrv6;

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) > 0)
    {
        domain = AF_INET;
    }

    if (inet_pton(AF_INET6, argv[1], &serv_addrv6.sin6_addr) > 0)
    {
        domain = AF_INET6;
    }

    int port = getPort(strdup(argv[2]));

    if ((sock = socket(domain, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        dieWithMessage("Socket creation error\n");
    }

    serv_addr.sin_family = domain;
    serv_addr.sin_port = htons(port);

    serv_addrv6.sin6_family = domain;
    serv_addrv6.sin6_port = htons(port);

    struct sockaddr *conAddress = (struct sockaddr *)&serv_addr;
    int conSize = sizeof(serv_addr);
    if (domain == AF_INET6)
    {
        conAddress = (struct sockaddr *)&serv_addrv6;
        conSize = sizeof(serv_addrv6);
    }

    if (connect(sock, conAddress, conSize) < 0)
    {
        dieWithMessage("Connection Failed \n");
    }

    return sock;
}

/*The last section covers the main function that is responsible to connect to the server,
nitialize variables, and start the reading loop, where the client enters in a state that it waits the terminal inputs to
send them as a message to the server. Then it awaits the server to and execute the validcommands that it receives and
to return an feedback.*/

int main(int argc, char const *argv[])
{
    int sock = buildClientSocket(argc, argv);
    char buffer[BUFFER_SIZE_BYTES] = {0};
    char message[BUFFER_SIZE_BYTES];

    for (;;)
    {
        readMessage(message);
        int totalBytesSent = send(sock, message, strlen(message), 0);
        validateCommunication(totalBytesSent);
        memset(buffer, 0, sizeof(buffer));
        int totalBytesRead = read(sock, buffer, BUFFER_SIZE_BYTES);
        validateCommunication(totalBytesRead);
        printf("%s\n", buffer);
    }
    return 0; // never reached
}