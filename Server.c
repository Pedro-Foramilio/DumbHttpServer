#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

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

    sock = make_socket(PORT);
    if (listen(sock, 1) < 0) {
        perror("listen");
        exit (EXIT_FAILURE);
    }

    while (1)
    {
        client_sock = accept(sock, (struct sockaddr *) &clientname, &size);
        if (client_sock < 0) 
        {
            perror("accept");
            exit (EXIT_FAILURE);
        }
        fprintf(stderr, "Server: connect from host %s, port %d.\n",
                             inet_ntoa(clientname.sin_addr),
                             ntohs (clientname.sin_port));
        read_from_client(client_sock);
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
    if (nbytes < 0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    } else if (nbytes == 0)
    {
        return -1;
    } else
    {
        fprintf (stderr, "Server: got message: `%s'\n", buffer);
        return 0;
    }

}
