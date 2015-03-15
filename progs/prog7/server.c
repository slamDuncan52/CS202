#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define HOSTLEN 256
#define BACKLOG 1

#define HOLD 0
#define ROLL 1

struct player{
	char* playerName;
	int score;
	char* stringScore;
};

char p1[100];
char p2[100];
struct player firstPlayer = {p1, 0, "000"};
struct player secondPlayer = {p2, 0, "000"};
int havePlayer = 0;
int readyToPlay = 0;
int isChild = 0;

int make_server_socket_q(int portnum, int backlog);
int make_server_socket(int portnum);
void child_waiter(int signum);
void process_request(int fd);
int playPig(struct player p1, struct player p2, int fd);

int main(int argc, char *argv[]){
	int sock, fd;
	int port = 50000;
	signal(SIGCHLD, child_waiter);
	sock = make_server_socket(port);
	if(sock == -1) exit(1);
	printf("Begin.\n");

	while(1){
		if(isChild){
			break;
		}
		fd = accept(sock, NULL, NULL);
		if(fd == -1){
			if(errno != EINTR) strerror(errno);
			else break;
		}
		process_request(fd);
	}
	return 0;
}

void child_waiter(int signum){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


void process_request(int fd){
	if(havePlayer == 0){
		//Get first player
		havePlayer = 1;
		readyToPlay = 0;
		read(fd, p1, 100);
		printf("Got First User: %s\n",p1);
	} else {
		//Get second player
		havePlayer = 0;
		readyToPlay = 1;
		read(fd, p2, 100);
		printf("Got Second User: %s\n",p2);
	}
	//make their game
	if(readyToPlay == 1){
		printf("fork\n");
		if(fork() == 0){
			isChild = 1;
			firstPlayer.playerName = p1;
			secondPlayer.playerName = p2;
			char helloString[250] = "In game with: ";
			strcat(helloString,p1);
			strcat(helloString," and ");
			strcat(helloString,p2);
			write(fd,helloString,250);
			playPig(firstPlayer, secondPlayer, fd);
		}
		havePlayer = 0;
		readyToPlay = 0;
	}
	close(fd);
}

int playPig(struct player p1, struct player p2, int fd){
//Report scores
char scoreBuf[7];
strcat(scoreBuf,p1.stringScore);
scoreBuf[4] = ' ';
strcat(scoreBuf,p2.stringScore);
write(fd,scoreBuf,7);
int roll = 6 + rand() / (RAND_MAX / (1 - 6 + 1) + 1);
printf("%d", roll);
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
