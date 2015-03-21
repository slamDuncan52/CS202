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
	char* rollString;
};

char p1[100];
char p2[100];
struct player firstPlayer = {0,p1, 0, 0, ROLL,"rolling"};
struct player secondPlayer = {0,p2, 0, 0, ROLL,"rolling"};
int isChild = 0;
int p1Connect = 1;
int p2Connect = 1;

int make_server_socket_q(int portnum, int backlog);
int make_server_socket(int portnum);
void child_waiter(int signum);
void process_request(int fd1, int fd2);

int playPig(struct player p1, struct player p2);
int playTurn(struct player *p1, struct player *p2);
int playRound(struct player *p1, struct player *p2);

int main(int argc, char *argv[]){
	int sock, fd1, fd2;
	int port = 50000;
	signal(SIGCHLD, child_waiter);
	signal(SIGPIPE,SIG_IGN);
	sock = make_server_socket(port);
	if(sock == -1) exit(1);
	printf("SERVER START\n");

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
	p1Connect = read(fd1, p1, 100);
	printf("Got First User: %s\n",p1);
	firstPlayer.fd = fd1;
	//Get second player
	p2Connect = read(fd2, p2, 100);
	printf("Got Second User: %s\n",p2);
	secondPlayer.fd = fd2;
	//make their game
	printf("Make Game\n");
	if(fork() == 0){
		isChild = 1;
		firstPlayer.playerName = p1;
		secondPlayer.playerName = p2;
		char helloString[250] = "New game:\nPlayer 1 : ";
		strcat(helloString,p1);
		strcat(helloString,"\nPlayer 2 : ");
		strcat(helloString,p2);
		strcat(helloString,"\n");
		p1Connect = write(fd1,helloString,250);
		p2Connect = write(fd2,helloString,250);
		playPig(firstPlayer, secondPlayer);
		printf("GAME FINISHED BETWEEN %s AND %s\n",p1,p2);
	}
	close(fd1);
	close(fd2);
}
//PLAY GAME
int playPig(struct player p1, struct player p2){
	int nextTurn = 0;
	while(!nextTurn){
		nextTurn = playTurn(&p1,&p2);
	}

}

//PLAY TURN
int playTurn(struct player *p1, struct player *p2){
	int nextRound = 0;	
	int status;
	char statusStr[300];

	//Play a round
	while(!nextRound){
		nextRound = playRound(p1,p2);
	}
	//Update Total Scores
	p1->score += p1->tempPoints;
	p2->score += p2->tempPoints;
	p1->tempPoints = 0;
	p2->tempPoints = 0;
	//Reset player choices
	p1->choice = ROLL;
	p1->rollString = "rolling";
	p2->choice = ROLL;
	p2->rollString = "rolling";
	//Check if game is done
	if((p1->score >= 100 || p2->score >= 100) && (p1->score != p2->score)){
		status = WIN;
		if(p1->score > p2->score){
			sprintf(statusStr,"\n%s wins\nby a score of %d - %d\n",p1->playerName,
					p1->score,p2->score);
		} else {
			sprintf(statusStr,"\n%s wins\nby a score of %d - %d\n",p2->playerName,
					p1->score,p2->score);
		}
	} else if((p1->score >= 100 || p2->score >= 100) && (p1->score == p2->score)){
		status = TIE;
		sprintf(statusStr,"\nTie game\nFinal score %d - %d\n",p1->score,p2->score);
	} else{
		status = CONT;
		sprintf(statusStr,"\nEnd of turn scores:\n%s: %d\n%s: %d\n",p1->playerName,p1->score,p2->playerName,p2->score);
	}
	if(p1Connect == 0 || p1Connect == -1){
		sprintf(statusStr,"\n%s Disconnected, %s wins by default!\n",p1->playerName,p2->playerName);
		status = -1;
		p2Connect = write(p2->fd,statusStr,300);
		p2Connect = write(p2->fd,&status,sizeof(int));
		return -1;
	} else if(p2Connect == 0 || p2Connect == -1){
		sprintf(statusStr,"\n%s Disconnected, %s wins by default!\n",p2->playerName,p1->playerName);
		status = -1;		
		p1Connect = write(p1->fd,statusStr,300);
		p1Connect = write(p1->fd,&status,sizeof(int));
		return status;
	}
	p1Connect = write(p1->fd,statusStr,300);
	p2Connect = write(p2->fd,statusStr,300);

	p1Connect = write(p1->fd,&status,sizeof(int));
	p2Connect = write(p2->fd,&status,sizeof(int));
	return status;
}

//PLAY ROUND
int playRound(struct player *p1, struct player *p2){
	char writeBuf[250];
	int returnStatus = 0;
	int oneFlag = 0;
	//Roll
	int curDie = (rand() % 6) + 1;	
	//If 1 is rolled, bail out
	if(curDie == 1){
		returnStatus = 2;
		oneFlag = 1;
		p1Connect = write(p1->fd,&oneFlag,sizeof(int));
		p2Connect = write(p2->fd,&oneFlag,sizeof(int));
		if(p1->choice == ROLL && p2->choice == ROLL){
			p1->tempPoints = 0;
			p2->tempPoints = 0;
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d points banked\n"
					"%s had %d points banked\nAnd you both lost them!",p1->playerName,p1->tempPoints,p2->playerName,p2->tempPoints);
		}
		else if(p1->choice == ROLL && p2->choice == HOLD){
			p1->tempPoints = 0;
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d points banked\n"
					"%s had %d points banked\n%s lost them, but %s's are safe!",p1->playerName,p1->tempPoints,p2->playerName,p2->tempPoints,p1->playerName,p2->playerName);
		}
		if(p1->choice == HOLD && p2->choice == ROLL){
			p2->tempPoints = 0;
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d points banked\n"
					"%s had %d points banked\n%s's are safe, but %s lost them!",p1->playerName,p1->tempPoints,p2->playerName,p2->tempPoints,p1->playerName,p2->playerName);
		}
		else if(p1->choice == HOLD && p2->choice == HOLD){
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d points banked\n"
					"%s had %d points banked\nAnd they're all safe!",p1->playerName, p1->tempPoints,p2->playerName,p2->tempPoints);
		}
		p1Connect = write(p1->fd,writeBuf,250);
		p2Connect = write(p2->fd,writeBuf,250);
		p1Connect = write(p1->fd,&returnStatus,sizeof(int));
		p2Connect = write(p2->fd,&returnStatus,sizeof(int));
		return returnStatus;
	}
	//Update temp scores
	if(p1->choice == ROLL){p1->tempPoints += curDie;}
	if(p2->choice == ROLL){p2->tempPoints += curDie;}
	//Announce current state
	p1Connect = write(p1->fd,&oneFlag,sizeof(int));
	p2Connect = write(p2->fd,&oneFlag,sizeof(int));
	sprintf(writeBuf,"The current roll is: %d\n%s has %d points banked and"
			" is %s\n%s has %d points banked and is %s\nCurrent score is:\n""%s: %d\n%s: %d\n",
			curDie,p1->playerName,p1->tempPoints,p1->rollString,p2->playerName,p2->tempPoints,p2->rollString,p1->playerName,p1->score,p2->playerName,p2->score);
	p1Connect = write(p1->fd,writeBuf,250);
	p2Connect = write(p2->fd,writeBuf,250);
	//Request rollers to hold
	if(p1->choice == ROLL){
		p1Connect = read(p1->fd,&p1->choice,sizeof(int));
		if(p1->choice == ROLL){p1->rollString = "rolling";}
		else{p1->rollString = "holding";}
	}
	if(p2->choice == ROLL){
		p2Connect = read(p2->fd,&p2->choice,sizeof(int));
		if(p2->choice == ROLL){p2->rollString = "rolling";}
		else{p2->rollString = "holding";}
	}
	if(p1->choice == HOLD && p2->choice == HOLD){
		returnStatus = 1;
		p1Connect = write(p1->fd,&returnStatus,sizeof(int));
		p2Connect = write(p2->fd,&returnStatus,sizeof(int));
		return returnStatus;
	}

	p1Connect = write(p1->fd,&returnStatus,sizeof(int));
	p2Connect = write(p2->fd,&returnStatus,sizeof(int));
	return returnStatus;
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
