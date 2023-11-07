#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define ADDIP "10.0.2.15"

long long factorial(long long n){
    if(n>20){
        n = 20;
    }
	unsigned long long ans = 1;
	for (int i = 1 ; i <= n ; i++){
		ans *= i;
	}
	return ans;
}


int check(int exp, const char* msg){
	if( exp < 0){
		perror(msg);
		exit(1);
	}
}

int main(){
	int sockfd, b, newSocket;
	struct sockaddr_in serverAddr;
	socklen_t addr_size = sizeof(serverAddr);
	int opt = 1;

	char mssg[100];   
	pid_t pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	check(sockfd, "error in socket\n");

	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))== -1){
		perror("Setsockopt failed");
		exit(1);
	}

	// memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_addr.s_addr = inet_addr(ADDIP);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);


	b = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(b<0){
		printf("Error in binding!\n");
		exit(1);
	}

	if(listen(sockfd, 10) == 0){
		printf("Listening...\n\n");
	}
	else{
		perror("Error on listening\n");
	};

	printf("Server is ready..\n");

	while(1){

		newSocket = accept(sockfd, (struct sockaddr*)&serverAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}

		if((pid = fork()) == 0){

			close(sockfd);

			while(1){

				close(sockfd);

				unsigned long long x;
				bzero(mssg, 100);
				recv(newSocket, &mssg, sizeof(mssg), 0);

				printf("Client x: %s\n", mssg);
				int num = atoi(mssg);
				x = factorial(num);
				printf("factorial is %llu",x);
				bzero(mssg, 100);
				sprintf( mssg, "%lld", x);
				
				send(newSocket, &mssg, sizeof(mssg), 0);
				x=0;
			}
		}
		else if(pid<0){
			perror("Fork failed!\n");
			exit(EXIT_FAILURE);
		}	
		else{
			close(newSocket);
			wait(NULL);
		}
	}
	close(newSocket);

	return 0;
}
