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
#include <sys/time.h>

#define SERVER_ADDRESS "127.0.1"
#define SERVER_PORT 6666
#define BUFFER_SIZE 1024

void write_to_server(int sock);
void read_from_server(int sock);
int discover_servers(int discovery_port, int *servicePortList);
int initializeServiceList(int *servicePortList);

int main(void) 
{
    int sock;
    int servicePortList[9999];
    initializeServiceList(servicePortList);
    struct sockaddr_in servername;
   
    /* try discovery first (will print any discovered servers) */
    discover_servers(SERVER_PORT + 1, servicePortList);

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
    write_to_server(sock);

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

int discover_servers(int discovery_port, int *servicePortList)
{
    int dsock;
    int yes = 1;
    struct sockaddr_in baddr;
    struct timeval tv;

    dsock = socket(AF_INET, SOCK_DGRAM, 0);
    if (dsock < 0) {
        perror("discovery socket");
        return -1;
    }

    if (setsockopt(dsock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
        perror("setsockopt SO_BROADCAST");
        close(dsock);
        return -1;
    }

    /* 1 second receive timeout */
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(dsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&baddr, 0, sizeof(baddr));
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(discovery_port);
    baddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    const char *probe = "DISCOVER";
    ssize_t wn = sendto(dsock, probe, strlen(probe), 0, (struct sockaddr *)&baddr, sizeof(baddr));
    if (wn < 0) {
        perror("sendto discovery");
        close(dsock);
        return -1;
    }

    fprintf(stderr, "Discovery probe sent, waiting for replies...\n");

    while (1) {
        int port = -1;
        char buf[256];
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t n = recvfrom(dsock, buf, sizeof(buf)-1, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0) break; /* timeout or error -> stop */
        buf[n] = '\0';
        fprintf(stderr, "Discovered server %s -> %s\n", inet_ntoa(from.sin_addr), buf);
        if (sscanf(buf, "SERVICE %4d", &port) == 1) 
        {
            // Store the discovered port in the servicePortList
            if (port >= 0 && port < 9999) {
                printf("Storing discovered service port: %d\n", port);
                servicePortList[port] = port;
            }
        }
    }

    close(dsock);
    return 0;
}

int initializeServiceList(int *servicePortList)
{
    for (int i = 0; i < 9999; i++) {
        servicePortList[i] = -1;
    }
    return 0;
} 

