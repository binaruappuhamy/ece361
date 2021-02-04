#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
/*
- Beej's Guide to Network Programming
- 03/02/2021
- Datagram Sockets(6.3)
- Source Code
- http://beej.us/guide/bgnet/examples/listener.c
*/

#define MAXBUFLEN 100
int main(int argc, char const *argv[])
{
    if (argc != 2) { 
        fprintf(stderr, "usage: server <UDP listen port>\n");
        exit(1);
    }

    int sockfd;	
 
    struct addrinfo hints;
    struct addrinfo* servinfo;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //IPv4 and IPv6
    hints.ai_socktype = SOCK_DGRAM; //set socket type for UDP
    hints.ai_flags = AI_PASSIVE; //use my IP

    if ((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    struct addrinfo* i;

    //loop through all the results and bind to the first successfull bind
    for (i = servinfo; i != NULL; i = i -> ai_next) {
        if ((sockfd = socket(i->ai_family, i->ai_socktype,
            i->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (bind(sockfd, i->ai_addr, i->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (i == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("server:waiting...\n");

    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXBUFLEN];

    //store the received packet in buf[]
    if ((recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
        (struct sockaddr*)&their_addr, &addr_len)) == -1) {
        perror("recvfrom\n");
        exit(1);
    }

    //compare the buffer for "ftp" and send the confirmation message
    if (strcmp(buf, "ftp") == 0) {
        if ((sendto(sockfd, "yes", strlen("yes"), 0,
            (struct sockaddr *) &their_addr, addr_len)) == -1) {
            perror("sendto error\n");
            exit(1);
        }
    } else {
        if ((sendto(sockfd, "no", strlen("no"), 0, 
            (struct sockaddr *)&their_addr, addr_len)) == -1) {
            perror("sendto error\n");
            exit(1);
        }
    }

    close(sockfd);
    return 0;
}
