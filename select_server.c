#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define ADDIP "10.0.2.15"
#define MAX_CLIENTS 3000

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
    int sockfd, newSocket, max_sd;
    int client_sockets[MAX_CLIENTS];
    fd_set read_fds;
    char buffer[1024];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size = sizeof(clientAddr);

    serverAddr.sin_addr.s_addr = inet_addr(ADDIP);
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
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_sd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (sd > 0) {
                FD_SET(sd, &read_fds);
                if (sd > max_sd) {
                    max_sd = sd;
                }
            }
        }

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            newSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);
            if (newSocket < 0) {
                perror("Accept error");
            } else {
                printf("New connection, socket fd is %d\n", newSocket);

                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_sockets[i] == 0) {
                        client_sockets[i] = newSocket;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (FD_ISSET(sd, &read_fds)) {
                ssize_t valread = read(sd, buffer, sizeof(buffer));

                if (valread <= 0) {
                    getpeername(sd, (struct sockaddr*)&clientAddr, &addr_size);
                    printf("Host disconnected, ip %s, port %d\n",
                           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("Received: %s\n", buffer);

                    unsigned long long n;
                    if (sscanf(buffer, "%llu", &n) == 1) {
                        unsigned long long result = factorial(n);
                        printf("Factorial of %llu is %llu\n", n, result);
                        char response[100];
                        snprintf(response, sizeof(response), "Factorial of %llu is %llu\n", n, result);
						printf("Factorial of %llu is %llu\n", n, result);
                        send(sd, response, strlen(response), 0);
                    }
                }
            }
        }
    }

    close(sockfd);

    return 0;
}
