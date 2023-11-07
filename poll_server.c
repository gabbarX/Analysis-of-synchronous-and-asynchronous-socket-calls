#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 4000

long long factorial(long long n) {
    if (n > 20) {
        n = 20;
    }
    unsigned long long ans = 1;
    for (int i = 1; i <= n; i++) {
        ans *= i;
    }
    return ans;
}

int main() {
    int sockfd, newSocket;
    struct pollfd client_sockets[MAX_CLIENTS];
    char buffer[1024];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i].fd = -1;
        client_sockets[i].events = POLLIN;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size = sizeof(clientAddr);

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error in binding");
        exit(1);
    }

    if (listen(sockfd, 10) < 0) {
        perror("Error on listening");
        exit(1);
    }

    printf("Server is ready...\n");

    while (1) {
        int active_connections = 0;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i].fd == -1) {
                client_sockets[i].fd = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);
                if (client_sockets[i].fd < 0) {
                    break;
                }
                printf("New connection, socket fd is %d\n", client_sockets[i].fd);
                active_connections++;
            }
        }

        int poll_result = poll(client_sockets, active_connections, -1);
        if (poll_result < 0) {
            perror("Error in poll");
            exit(1);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i].fd;

            if (fd != -1 && (client_sockets[i].revents & POLLIN)) {
                ssize_t valread = read(fd, buffer, sizeof(buffer));

                if (valread <= 0) {
                    getpeername(fd, (struct sockaddr*)&clientAddr, &addr_size);
                    printf("Host disconnected, ip %s, port %d\n",
                           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                    close(fd);
                    client_sockets[i].fd = -1;
                } else {
                    buffer[valread] = '\0';
                    printf("Received: %s\n", buffer);

                    unsigned long long n;
                    if (sscanf(buffer, "%llu", &n) == 1) {
                        unsigned long long result = factorial(n);
                        printf("Factorial of %llu is %llu\n", n, result);
                        char response[100];
                        snprintf(response, sizeof(response), "Factorial of %llu is %llu\n", n, result);
                        send(fd, response, strlen(response), 0);
                    }
                }
            }
        }
    }

    close(sockfd);

    return 0;
}
