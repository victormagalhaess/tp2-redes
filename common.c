#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define MIN_PORT_VALUE 1025
#define MAX_PORT_VALUE 65535

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
    if (port >= MIN_PORT_VALUE && port <= MAX_PORT_VALUE)
    {
        return port;
    }
    dieWithMessage("Port in invalid range. You must use a non-root valid Unix port betwheen 1025 and 65535");
    return -1; // never reached
}