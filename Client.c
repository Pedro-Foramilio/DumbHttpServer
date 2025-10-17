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
void read_from_server(int sock);

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
    write_to_server (sock);

    read_from_server(sock);

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

void read_from_server(int sock)
{
    int nbytes;
    char buffer[BUFFER_SIZE];

    nbytes = read(sock, buffer, sizeof(buffer) - 1);
    if (nbytes < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    buffer[nbytes] = '\0'; // Null-terminate the string
    printf("Received from server: %s\n", buffer);
}

