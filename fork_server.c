#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define LOG_FILE_NAME "2b.txt"
FILE *filedis = NULL;

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

	struct sockaddr_in serverAddr, clienAddr;

	socklen_t addr_size;

	char mssg[100];   
	pid_t pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	check(sockfd, "error in socket\n");


	memset(&serverAddr, '\0', sizeof(serverAddr));
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


	while(1){

		newSocket = accept(sockfd, (struct sockaddr*)&clienAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}

		char *IP = inet_ntoa(clienAddr.sin_addr);
		int PORT_NO = ntohs(clienAddr.sin_port);

		if( filedis == NULL)filedis = fopen( LOG_FILE_NAME, "w");

		printf("Connection accepted from IP : %s: and PORT : %d\n", IP, PORT_NO);

		if((pid = fork()) == 0){

			close(sockfd);
			
			int k=0;

			while(k++ <20){

				long long x;
				bzero(mssg, 100);
				recv(newSocket, &mssg, sizeof(mssg), 0);

				printf("Client x: %s\n", mssg);
				int num = atoi(mssg);
				x = factorial(num);
				fprintf(filedis, "IP : %s  PORT : %d \n INTEGER : %d  FACTORIAL : %lld\n", IP, PORT_NO, num, x );
				bzero(mssg, 100);
				sprintf( mssg, "%lld", x);
				
				send(newSocket, &mssg, sizeof(mssg), 0);
				x=0;
					
			}

		}
		
	}

	close(newSocket);

	return 0;
}
