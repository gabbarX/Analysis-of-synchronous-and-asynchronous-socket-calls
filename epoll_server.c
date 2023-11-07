#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_EVENTS 10

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
    int sockfd, newSocket, epoll_fd;
    struct epoll_event event, events[MAX_EVENTS];
    char buffer[1024];

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

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Error creating epoll");
        exit(1);
    }

    event.events = EPOLLIN;
    event.data.fd = sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
        perror("Error adding socket to epoll");
        exit(1);
    }

    printf("Server is ready...\n");

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Error in epoll_wait");
            exit(1);
        }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            if (fd == sockfd) {
                newSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);
                if (newSocket < 0) {
                    perror("Accept error");
                } else {
                    printf("New connection, socket fd is %d\n", newSocket);
                    event.events = EPOLLIN;
                    event.data.fd = newSocket;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newSocket, &event) == -1) {
                        perror("Error adding new socket to epoll");
                        exit(1);
                    }
                }
            } else {
                ssize_t valread = read(fd, buffer, sizeof(buffer));

                if (valread <= 0) {
                    getpeername(fd, (struct sockaddr*)&clientAddr, &addr_size);
                    printf("Host disconnected, ip %s, port %d\n",
                           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                    close(fd);
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