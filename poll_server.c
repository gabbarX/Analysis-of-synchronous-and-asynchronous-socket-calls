#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>

#define PORT 8080
#define MAX_CLIENTS 4000

unsigned long long fact(int n) {
    if (n > 20) {
        n = 20;
    }
    unsigned long long result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[MAX_CLIENTS];
    memset(fds, 0, sizeof(fds));

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (1) {
        if (poll(fds, MAX_CLIENTS, -1) < 0) {
            perror("Poll error");
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN) {
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            int i;
            for (i = 1; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    fds[i].fd = new_socket;
                    fds[i].events = POLLIN;
                    break;
                }
            }

            if (i == MAX_CLIENTS) {
                perror("Too many clients");
                close(new_socket);
            }

            fds[0].revents = 0;
        }

        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0 && (fds[i].revents & POLLIN)) {
                char buffer[1024];
                ssize_t valread = read(client_sockets[i], buffer, sizeof(buffer));
                if (valread < 0) {
                    perror("Read error");
                    close(client_sockets[i]);
                    fds[i].fd = -1;
                    client_sockets[i] = -1;
                } else if (valread == 0) {
                    close(client_sockets[i]);
                    fds[i].fd = -1;
                    client_sockets[i] = -1;
                } else {
                    unsigned long long n;
                    if (sscanf(buffer, "%llu", &n) == 1) {
                        unsigned long long result = fact(n);
                        char response[100];
                        snprintf(response, sizeof(response), "Factorial of %llu is %llu\n", n, result);
                        send(client_sockets[i], response, strlen(response), 0);
                    }
                }
                fds[i].revents = 0;
            }
        }
    }

    return 0;
}
