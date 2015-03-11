#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <strings.h>
#include <errno.h>

#define HOSTLEN 256
#define BACKLOG 1

int make_server_socket_q(int portnum, int backlog);
int make_server_socket(int portnum);
void child_waiter(int signum);
void process_request(int fd);

int main(int argc, char *argv[]){
	int sock, fd;
	int port = 13000;
	signal(SIGCHLD, child_waiter);
	sock = make_server_socket(port);
	if(sock == -1) exit(1);
	printf("Begin.\n");

	while(1){
		printf("Pre accept  ");
		fd = accept(sock, NULL, NULL);
		printf("Post accept\n");
		/*	if(fd == -1){
			if(errno != EINTR) strerror(errno);
			else strerror(errno);
			}*/
		process_request(fd);
		close(fd);
	}
	return 0;
}

void child_waiter(int signum){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void process_request(int fd){
	printf("Pre fork\n");
if(fork() == 0){
	printf("Pre dup ");
	dup2(fd, 1);
	close(fd);
	printf("Post dup close\n");
}
}


































int make_server_socket(int portnum){
	return make_server_socket_q(portnum, BACKLOG);
}

int make_server_socket_q(int portnum, int backlog){
	struct sockaddr_in saddr;
	struct hostent *hp;
	char hostname[HOSTLEN];
	int sock_id;

	sock_id = socket(PF_INET, SOCK_STREAM, 0);
	if(sock_id ==-1) return -1;

	bzero( (void *)&saddr, sizeof(saddr));

	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(portnum);
	saddr.sin_family = AF_INET;

	if(bind(sock_id, (struct sockaddr*)&saddr, sizeof(saddr)) != 0)
		return -1;

	if(listen(sock_id, backlog) != 0) return -1;

	return sock_id;
}
