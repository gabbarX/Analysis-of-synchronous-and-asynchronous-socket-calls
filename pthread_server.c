#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define ADDIP "10.0.2.15"
#define MAX_THREADS 10

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

typedef struct {
    int client_fd;
} ThreadArg;

void* handle_client(void* arg) {
    ThreadArg* threadArg = (ThreadArg*)arg;
    int client_fd = threadArg->client_fd;
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
                unsigned long long result = factorial(n);
                printf("Factorial of %llu is %llu\n", n, result);
                char response[100];
                snprintf(response, sizeof(response), "Factorial of %llu is %llu\n", n, result);
                send(client_fd, response, strlen(response), 0);
            }
        }
    }

    close(client_fd);
    free(threadArg);
    return NULL;
}

int main() {
    int sockfd, newSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size = sizeof(serverAddr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }

    serverAddr.sin_addr.s_addr = inet_addr(ADDIP);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error in binding");
        exit(1);
    }

    if (listen(sockfd, 10) == 0) {
        printf("Listening...\n\n");
    } else {
        perror("Error on listening");
    }

    printf("Server is ready..\n");

    // Create a thread pool to handle connections
    pthread_t threads[MAX_THREADS];
    int threadCount = 0;

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&serverAddr, &addr_size);
        if (newSocket < 0) {
            perror("Accept failed");
            continue;
        }

        // Create a thread to handle the client
        if (threadCount < MAX_THREADS) {
            ThreadArg* threadArg = (ThreadArg*)malloc(sizeof(ThreadArg));
            if (threadArg == NULL) {
                perror("ThreadArg allocation failed");
                close(newSocket);
                continue;
            }
            threadArg->client_fd = newSocket;
            if (pthread_create(&threads[threadCount], NULL, handle_client, (void*)threadArg) != 0) {
                perror("Thread creation failed");
                close(newSocket);
                free(threadArg);
                continue;
            }
            threadCount++;
        } else {
            // Handle the case when the thread pool is full
            printf("Thread pool is full. Rejecting the connection.\n");
            close(newSocket);
        }
    }

    close(sockfd);

    return 0;
}
