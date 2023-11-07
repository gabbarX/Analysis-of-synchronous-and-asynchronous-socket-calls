#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define MAX_CLIENTS 3000

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

void handle_client(int client_fd) {
    char buffer[1024];

    while (1) {
        ssize_t valread = read(client_fd, buffer, sizeof(buffer));

        if (valread < 0) {
            perror("Read error");
        } else if (valread == 0) {
            // The client has closed the connection; exit the loop.
            break;
        } else {
            unsigned long long n;
            if (sscanf(buffer, "%llu", &n) == 1) {
                unsigned long long result = fact(n);
                
                char response[100];
                snprintf(response, sizeof(response), "Factorial of %llu is %llu\n", n, result);
                
                send(client_fd, response, strlen(response), 0);
            }
        }
    }

    close(client_fd);
    exit(0);
}

int main() {
    int server_fd, new_socket;
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

    printf("Server is ready to handle clients on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(server_fd);
            handle_client(new_socket);
        } else if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            close(new_socket);
            wait(NULL); // Reap the child process
        }
    }

    return 0;
}