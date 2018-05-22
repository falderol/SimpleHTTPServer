#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <zconf.h>

#define MYPORT "8080"

void print_addr_info(struct addrinfo * info) {
    struct sockaddr_in * tmpp;
    char ip4[INET_ADDRSTRLEN];

    while(info) {
        if (info->ai_family == PF_INET) { //IPv4
            tmpp = (struct sockaddr_in *) info->ai_addr;
            inet_ntop(AF_INET, &tmpp->sin_addr, ip4, INET_ADDRSTRLEN);

            printf("IPv4 address: %s\n", ip4);
        }
        info = info->ai_next;
    }
}


void handle_connection(int acceptfd){//this is where we will implement http stuff
    char c;
    recv(acceptfd, &c, 1,  0);
    printf("%c\n", c);
}

int main() {
    struct addrinfo hints;
    struct addrinfo * address_resource;

    memset(&hints, 0, sizeof(hints)); // make sure the struct is clear
    hints.ai_family = AF_UNSPEC; // Any protocol the system supports
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;// Listening

    /////
    // Info for Default Ip (string), Info for port number (string), hints(struct addrinfo), status(struct *addrinfo)
    int ainfo_desc = getaddrinfo("localhost", MYPORT, &hints, &address_resource);

    print_addr_info(address_resource);
    /////
    // Allocate a socket for our program
    int socket_desc = socket(address_resource->ai_family, address_resource->ai_socktype, address_resource->ai_protocol);

    /////
    // socket descriptor, magic...
    int enable = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {//returns negative value if unsucsessful
        printf("Error: setsockopt failed\n");
    }

    /////
    // Bind

    int tmp_ret = bind(socket_desc, address_resource->ai_addr, address_resource->ai_addrlen);

    if(tmp_ret != 0){
        printf("Error: %s\n", strerror(errno));
    }

    /////
    // Listen

    listen(socket_desc, 1);

    /////
    // Accept

    struct sockaddr_storage remote_addr;
    socklen_t remote_addr_s = sizeof remote_addr;
    int accept_desc;

    while(1){
        accept_desc = accept(socket_desc,(struct sockaddr *) &remote_addr, &remote_addr_s);
        close(socket_desc);
        handle_connection(accept_desc);//read from user connection
        close(accept_desc);
        return 0;
    }

    ///////////////
    // Clean Up
    free(ainfo_desc);
    free(socket_desc);

    getchar();
}