#ifndef COMMON_NET_TP1
#define COMMON_NET_TP1
#define BUFFER_SIZE_BYTES 500

void dieWithMessage(char *message);
void validateInputArgs(int argc, int minArgs);
void validateCommunication(int valread);
int getPort(char *port);

#endif