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
	int choice;
};

char p1[100];
char p2[100];
struct player firstPlayer = {0,p1, 0, 0, ROLL};
struct player secondPlayer = {0,p2, 0, 0, ROLL};
int isChild = 0;

int make_server_socket_q(int portnum, int backlog);
int make_server_socket(int portnum);
void child_waiter(int signum);
void process_request(int fd1, int fd2);
int playPig(struct player p1, struct player p2);
int playTurn(struct player *p1, struct player *p2);
int playRound(struct player *p1, struct player *p2);
int updateStatus(struct player *p1, struct player *p2);
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
	firstPlayer.fd = fd1;
	//Get second player
	read(fd2, p2, 100);
	printf("Got Second User: %s\n",p2);
	secondPlayer.fd = fd2;
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
		playTurn(&p1,&p2);
		nextTurn = updateStatus(&p1,&p2);
	}

}

//TURN
int playTurn(struct player *p1, struct player *p2){
	int nextRound = 1;	
	//Play a round
	while(nextRound){
		nextRound = playRound(p1,p2);
	}
	//Update Total Scores
	p1->score += p1->tempPoints;
	p2->score += p2->tempPoints;
	p1->tempPoints = 0;
	p2->tempPoints = 0;
	//Check if game is done
	if (p1->score >= 100){
		return 1;
	} else if (p2->score >= 100){
		return 2;
	} else { 
		return 0;
	}
}

//ROUND
int playRound(struct player *p1, struct player *p2){
	int returnStatus = 1;
	//Roll
	int curDie = roll();
	//If 1 is rolled, bail out
	if(curDie == 1){
		returnStatus = 0;
		write(p1->fd,"Uh oh, a one was rolled!\n",250);
		write(p2->fd,"Uh oh, a one was rolled!\n",250);
		if(p1->choice == ROLL){p1->tempPoints = 0;}
		if(p2->choice == ROLL){p2->tempPoints = 0;}
		write(p1->fd,&returnStatus,sizeof(int));
		write(p2->fd,&returnStatus,sizeof(int));
		return returnStatus;
	}
	//Update temp scores
	if(p1->choice == ROLL){p1->tempPoints += curDie;}
	if(p2->choice == ROLL){p2->tempPoints += curDie;}
	//Announce current state
	char writeBuf[250];
	sprintf(writeBuf,"The current Roll is: %d\nThe current banked "
			"points for each player this round is:\nPlayer 1: %d\n"
			"Player 2: %d\n"
			"Each Player's total points is:\nPlayer 1: %d\nPlayer 2: %d\n",
			curDie,p1->tempPoints,p2->tempPoints,p1->score,p2->score);
	write(p1->fd,writeBuf,250);
	write(p2->fd,writeBuf,250);
	//Request rollers to hold
	if(p1->choice == ROLL){
		printf("Waiting for player 1 to decide on roll or hold\n");
		read(p1->fd,&p1->choice,sizeof(int));
	}
	if(p2->choice == ROLL){
		printf("Waiting for player 2 to decide on roll or hold\n");
		read(p2->fd,&p2->choice,sizeof(int));
	}
	if(p1->choice == HOLD && p2->choice == HOLD){
		returnStatus = 0;
		write(p1->fd,&returnStatus,sizeof(int));
		write(p2->fd,&returnStatus,sizeof(int));
		return returnStatus;
	}

	write(p1->fd,&returnStatus,sizeof(int));
	write(p2->fd,&returnStatus,sizeof(int));
	return returnStatus;
}




int updateStatus(struct player *p1, struct player *p2){
	int status;
	if(p1->score >= 100 || p2->score >= 100 && p1->score != p2->score){
		status = WIN;
	} else if(p1->score >= 100 || p2->score >= 100 && p1->score == p2->score){
		status = TIE;
	} else {
		status = CONT;
	}
	return status;
}

int roll(){
	return (rand() % 6) + 1;
}


/*
 * Game {
 *	 Turns [
 *		Rounds (
 *			roll!
 *			update tempscores
 *			Annouce roll, tempscores, total scores
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
