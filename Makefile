all: fserver tserver sserver pserver eserver 


fserver: fork_server.c
	gcc fork_server.c -o fork_server

tserver: pthread_server.c
	gcc pthread_server.c -o tserver

sserver: select_server.c
	gcc select_server.c -o sserver

pserver: poll_server.c
	gcc poll_server.c -o pserver

eserver: epoll_server.c
	gcc epoll_server.c -o eserver

clean: 
	rm fserver tserver sserver pserver eserver 

