#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#define PORT 6666
#define DISCOVERY_PORT (PORT + 1)
#define BUFFER_SIZE 2048

int make_socket(uint16_t port);
int read_and_handle_client (int filedes);
void format_response_to_client(char *buffer, char *response);

int main(void)
{
    int sock;
    int udp_sock;
    int client_sock;
    struct sockaddr_in clientname;
    socklen_t size = sizeof(clientname);
    fd_set master_set, read_fds;

    sock = make_socket(PORT);

    /* create UDP discovery socket and bind to DISCOVERY_PORT */
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("udp socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in daddr;
    daddr.sin_family = AF_INET;
    daddr.sin_port = htons(DISCOVERY_PORT);
    daddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(udp_sock, (struct sockaddr *)&daddr, sizeof(daddr)) < 0) {
        perror("bind udp");
        close(udp_sock);
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    /* make udp socket non-blocking as well (optional) */
    flags = fcntl(udp_sock, F_GETFL, 0);
    fcntl(udp_sock, F_SETFL, flags | O_NONBLOCK);

    int fdmax = sock > udp_sock ? sock : udp_sock;
    FD_ZERO(&master_set);
    FD_SET(sock, &master_set);
    FD_SET(udp_sock, &master_set);

    if (listen(sock, 1) < 0) {
        perror("listen");
        exit (EXIT_FAILURE);
    }

    while (1)
    {
        read_fds = master_set;

        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i <= fdmax; i++) 
        {
            if (! FD_ISSET(i, &read_fds)) continue;

            if (i == sock)
            { // new TCP connection
                int newfd = accept(sock, (struct sockaddr *) &clientname, &size);
                if (newfd < 0) 
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                FD_SET(newfd, &master_set);
                if (newfd > fdmax) fdmax = newfd;
                fprintf(stderr, "New connection on fd %d\n", newfd);
            } else if (i == udp_sock) {
                /* discovery request on UDP */
                char dbuf[256];
                struct sockaddr_in from;
                socklen_t fromlen = sizeof(from);
                ssize_t n = recvfrom(udp_sock, dbuf, sizeof(dbuf)-1, 0, (struct sockaddr *)&from, &fromlen);
                if (n > 0) {
                    dbuf[n] = '\0';
                    fprintf(stderr, "UDP discovery message from %s: %s\n", inet_ntoa(from.sin_addr), dbuf);
                    if (strcmp(dbuf, "DISCOVER") == 0) {
                        char reply[128];
                        snprintf(reply, sizeof(reply), "SERVICE %d", PORT);
                        ssize_t wn = sendto(udp_sock, reply, strlen(reply), 0, (struct sockaddr *)&from, fromlen);
                        if (wn < 0) perror("sendto");
                    }
                }
            } else
            { //data from TCP client
                int nbytes = read_and_handle_client(i);
                if (nbytes <= 0)
                {
                    close(i);
                    FD_CLR(i, &master_set);
                }
            }
        }
    }
    close(udp_sock);
    close(sock);
    return 0;
}

int make_socket(uint16_t port)
{
    int sock;
    struct sockaddr_in name;
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit (EXIT_FAILURE);
    }
    
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
        perror("bind");
        exit (EXIT_FAILURE);
    }
    
    return sock;
}

int read_and_handle_client (int filedes)
{
    char buffer[BUFFER_SIZE];
    int nbytes;
    char response[BUFFER_SIZE];

    nbytes = read(filedes, buffer, sizeof(buffer));

    if (nbytes <= 0) {
        if (nbytes == 0) {
            fprintf(stderr, "Server: Connection on fd %d closed by peer\n", filedes);
        } else {
            perror("read");
        }
        return nbytes;
    }

    if (nbytes < (int)sizeof(buffer)) buffer[nbytes] = '\0';
    else buffer[sizeof(buffer)-1] = '\0';

    fprintf(stderr, "Server: Connection on fd %d sent message: \n`%s'\n", filedes, buffer);

    format_response_to_client(buffer, response);

    ssize_t wn = write(filedes, response, strlen(response));
    if (wn < 0) {
        perror("write");
    }

    return nbytes;
}

void format_response_to_client(char *buffer, char *response)
{
    strcpy(response, "Server received your message: ");
}
