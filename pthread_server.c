#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define LOG_FILE_NAME "2c.txt"
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


void * perform_calculation(void * p_client_socket){      // pthred funtion accept void pointer

    int client_socket = *(( int *)p_client_socket);      // converting to int pointer
    free(p_client_socket);
	char mssg[100];

    int k=0;
    while(k++ <20){

		long long x;

        bzero(mssg, 100);
		recv(client_socket, &mssg, sizeof(mssg), 0);

		printf("Client x: %s\n", mssg);
		int num = atof(mssg);

        x = factorial(num);
		fprintf(filedis, "INTERGER : %d  FACTORIAL : %lld\n", num , x);

		bzero(mssg, 100);
		sprintf(mssg, "%lld", x);
        send(client_socket, &mssg, sizeof(mssg), 0);
        x=0;			

	}

    close(client_socket);
    return NULL;

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
	check(b, "error in bind\n");

	if(listen(sockfd, 10) < 0)perror("Error on listening\n");

	double time_spent = 0.0;
	clock_t begin = clock();

	while(1){

		

		newSocket = accept(sockfd, (struct sockaddr*)&clienAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		char *IP = inet_ntoa(clienAddr.sin_addr);
		int PORT_NO = ntohs(clienAddr.sin_port);

		printf("Connection accepted from  IP %s : PORT NO : %d\n", IP, PORT_NO);
		if( filedis == NULL)filedis = fopen( LOG_FILE_NAME, "w");

        int * pclient = malloc(sizeof(int));
        *pclient = newSocket;

        pthread_t new_thr;
		fprintf(filedis, "IP : %s  PORT : %d\n", IP, PORT_NO);
        pthread_create(&new_thr, NULL, perform_calculation, pclient);    

		

	}

	clock_t end = clock();
	time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("The elapsed time is %f seconds", time_spent);


	return 0;
}
