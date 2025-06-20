#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_ADDRESS "127.0.1"
#define SERVER_PORT 6666
#define BUFFER_SIZE 1024

void write_to_server(int sock);

int main(void) 
{
    int sock;
    struct sockaddr_in servername;
    servername.sin_family = AF_INET;
    servername.sin_port = htons(SERVER_PORT);
    servername.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    if (0 > connect(sock, (struct sockaddr *) &servername, sizeof(servername)) )
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    sleep(30);
    write_to_server (sock);
    close (sock);
    exit (EXIT_SUCCESS);
}

void write_to_server(int sock)
{
    int nbytes;
    char message[BUFFER_SIZE] = "My message!";

    nbytes = write(sock, message, strlen(message) + 1);
    if (nbytes < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

