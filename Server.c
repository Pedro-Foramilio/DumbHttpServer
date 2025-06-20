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

#define PORT 6666
#define BUFFER_SIZE 1024

int make_socket(uint16_t port);
int read_from_client (int filedes);

int main(void)
{
    int sock;
    int client_sock;
    struct sockaddr_in clientname;
    socklen_t size = sizeof(clientname);
    fd_set master_set, read_fds;

    sock = make_socket(PORT);

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    int fdmax = sock;
    FD_ZERO(&master_set);
    FD_SET(sock, &master_set);

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
            { //new connection
                int newfd = accept(sock, (struct sockaddr *) &clientname, &size);
                if (newfd < 0) 
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                FD_SET(newfd, &master_set);
                if (newfd > fdmax) fdmax = newfd;
                fprintf(stderr, "New connection on fd %d\n", newfd);
            } else
            { //data from client
                int nbytes = read_from_client(i);
                if (nbytes <= 0)
                {
                    close(i);
                    FD_CLR(i, &master_set);
                }
            }
        }
    }
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

int read_from_client (int filedes)
{
    char buffer[BUFFER_SIZE];
    int nbytes;

    nbytes = read(filedes, buffer, sizeof(buffer));
    
    if (nbytes <= 0) perror("read");
    
    else fprintf(stderr, "Server: got message: `%s'\n", buffer);
    
    return nbytes;
}
