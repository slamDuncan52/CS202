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

#define CONT 0
#define WIN 1
#define TIE 2

struct player{
	int fd;
	char* playerName;
	int score;
	int tempPoints;
	char* stringScore;
	int choice;
};

char p1[100];
char p2[100];
struct player firstPlayer = {0,p1, 0, 0, "000", HOLD};
struct player secondPlayer = {0,p2, 0, 0, "000", HOLD};
int isChild = 0;

int make_server_socket_q(int portnum, int backlog);
int make_server_socket(int portnum);
void child_waiter(int signum);
void process_request(int fd1, int fd2);
int playPig(struct player p1, struct player p2);
void playTurn(struct player p1, struct player p2);
int updateStatus(struct player p1, struct player p2);
int roll();

int main(int argc, char *argv[]){
	int sock, fd1, fd2;
	int port = 50000;
	signal(SIGCHLD, child_waiter);
	sock = make_server_socket(port);
	if(sock == -1) exit(1);
	printf("Begin.\n");

	while(1){
		if(isChild){
			break;
		}
		fd1 = accept(sock, NULL, NULL);
		fd2 = accept(sock, NULL, NULL);
		if(fd1 == -1 || fd2 == -1){
			if(errno != EINTR) strerror(errno);
			else break;
		}
		process_request(fd1, fd2);
	}
	return 0;
}

void child_waiter(int signum){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


void process_request(int fd1, int fd2){
	//Get first player
	read(fd1, p1, 100);
	printf("Got First User: %s\n",p1);
	//Get second player
	read(fd2, p2, 100);
	printf("Got Second User: %s\n",p2);
	//make their game
	printf("fork\n");
	if(fork() == 0){
		isChild = 1;
		firstPlayer.playerName = p1;
		secondPlayer.playerName = p2;
		char helloString[250] = "In game with: ";
		strcat(helloString,p1);
		strcat(helloString," and ");
		strcat(helloString,p2);
		write(fd1,helloString,250);
		write(fd2,helloString,250);
		playPig(firstPlayer, secondPlayer);
	}
	close(fd1);
	close(fd2);
}
//GAME
int playPig(struct player p1, struct player p2){
	int nextTurn = 1;
	while(nextTurn){
		playTurn(p1,p2);
		nextTurn = updateStatus(p1,p2);
	}

}

//TURN
void playTurn(struct player p1, struct player p2){
	//Roll
	int curDie = roll();
	//Report
	write(p1.fd,&curDie,sizeof(int));
	write(p2.fd,&curDie,sizeof(int));
	//Decide
	read(p1.fd,&p1.choice,1);
	read(p2.fd,&p2.choice,1);
	printf("Rolled a: %d\n",roll);fflush(stdout);

}

//ROUND

int updateStatus(struct player p1, struct player p2){
	int status;
	if(p1.score >= 100 || p2.score >= 100 && p1.score != p2.score){
		status = WIN;
	} else if(p1.score >= 100 || p2.score >= 100 && p1.score == p2.score){
		status = TIE;
	} else {
		status = CONT;
	}
	return status;
}

int roll(){
	return 6 + rand() / (RAND_MAX / (1 - 6 + 1) + 1);
}


/*
 * Game {
 *	 Turns [
 *		Rounds (
 *			roll!
 *			Annouce roll, tempscores, total scores
 *			update tempscores
 *			request rollers to hold
 *			break on 1 rolled, or 2 holds
 *		)
 *		update total scores
 *		check if game is done
 *	 ]
 * report end condition
 *
 * }
 *
 */






























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
