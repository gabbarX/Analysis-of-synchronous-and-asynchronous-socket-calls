#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>
#define LOG_FILE_NAME "2d1.txt"
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
	check(b, "error in bind\n");

	if(listen(sockfd, 10) < 0)perror("Error on listening\n");


    nfds_t nfds = 0;
    struct pollfd *pollfds;
    int maxfds = 0, numfds = 0;

    pollfds = malloc( 11*sizeof(struct pollfd));
    maxfds = 11;

    pollfds -> fd = sockfd;
    pollfds -> events = POLLIN;
    pollfds -> revents = 0;
    numfds = 1;

    if( filedis == NULL)filedis = fopen( LOG_FILE_NAME, "w");

	double time_spent = 0.0;
	clock_t begin = clock();


    while(1){

        nfds = numfds;

        if( poll( pollfds, nfds, -1) < 0)perror("error at poll");
        
        for( int fd = 0; fd < (nfds + 1); fd++){
            

            if( ((pollfds + fd) -> revents & POLLIN ) == POLLIN){  // check if this fd is ready for reading

                if( ( pollfds + fd) -> fd == sockfd){              // request for new connection

                    int  newSocket = accept(sockfd, (struct sockaddr*)&clienAddr, &addr_size);
                    if(newSocket < 0){
                        exit(1);
                    }


                    (pollfds + numfds ) -> fd = newSocket;
                    (pollfds + numfds ) -> events = POLLIN;
                    (pollfds + numfds ) -> revents = 0;

                    numfds++;

                    char *IP = inet_ntoa(clienAddr.sin_addr);
		            int PORT_NO = ntohs(clienAddr.sin_port);
                    fprintf(filedis, "IP : %s  PORT : %d\n", IP, PORT_NO);
                    printf("Connection accepted from IP :  %s PORT : %d\n", IP, PORT_NO);


                }else{   // some client is sending data


                    bzero(mssg, 100);
                    int numbytes = recv( (pollfds + fd) -> fd, &mssg, sizeof(mssg), 0);

                    long long num = atoi(mssg);
                    
                    sprintf(mssg, "%lld", factorial(num));
                    fprintf(filedis, "INTERGER : %lld  FACTORIAL : %lld\n", num , factorial(num));

                    send( (pollfds + fd) -> fd, &mssg, sizeof(mssg), 0);

                }

            }
        }

    }

    clock_t end = clock();
	time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("The elapsed time is %f seconds", time_spent);

    close(sockfd);
	return 0;
}

