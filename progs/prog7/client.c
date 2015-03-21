#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define WIN 1
#define LOSE 2
#define TIE 3

#define HOLD 0
#define ROLL 1

int connect_to_server(char*, int);
void handShake();
int playTurn();
int playRound();
int updateStatus();

char user[100];
char host[100];

int fd;
int gameStatus = 0;
int choice = ROLL;

int main(int argc, char* argv[]){
	int port;
	if((argc > 1) && atoi(argv[1]) != 0){
		port = atoi(argv[1]);
	} else {
		port = 50000;
	}
	printf("Welcome to Herd Pig! Please enter hostname >>> ");
	scanf("%s",host);
	printf("Please enter a username (100 characters or less) >>> ");
	scanf("%s",user);
	fd = connect_to_server(host, port);
	printf("Connected to Host: %s @ Port: %d\n",host,port);
	if(fd == -1) exit(1);
	write(fd,user,100);
	printf("Sent\n");
	handShake();
	//PLAY GAME
	while(!gameStatus){
		gameStatus = playTurn();
	}
	close(fd);
	return 0;
}
//PLAY TURN
int playTurn(){
	int overallStatus;
	char statusStr[300];
	int turnStatus = 0;
	choice = ROLL;
	while(!turnStatus){
		turnStatus = playRound();
	}
	read(fd,statusStr,300);
	printf("%s",statusStr);
	read(fd,&overallStatus,sizeof(int));
	return overallStatus;
}
//PLAY ROUND
int playRound(){
	int status;
	int oneCheck;
	char choiceBuf;
	char statusBuf[250];
	char oneStatus[100];
	read(fd,&oneCheck,sizeof(int));
	read(fd,statusBuf,250);
	printf("\n%s\n",statusBuf);

	if(choice == ROLL && oneCheck == 0){
		printf("Do you wish to continue rolling? >>> ");
		while(1){
			fflush(stdout);
			choiceBuf = getchar();
			if(choiceBuf == 'y'){ choice = ROLL;break;}
			if(choiceBuf == 'n'){ choice = HOLD;break;}
			if(choiceBuf != '\n'){
				printf("Do you wish to continue rolling? >>> ");
			}
		}
		write(fd,&choice,sizeof(int));
	} else if (choice == HOLD && !oneCheck){
		printf("Waiting for opponent to hold\n");
	} else if (oneCheck == 1){
	}
	read(fd,&status,sizeof(int));	
	return status;
}

void handShake(){
	printf("Waiting for opponent...\n");
	char handShakeBuf[250];
	int val = read(fd, handShakeBuf, 250);
	if(val == 250){
		printf("%s\n",handShakeBuf);
	}
}

















































































int connect_to_server(char *host, int portnum){
	struct sockaddr_in servadd;
	struct hostent *hp;
	int sock_id;

	sock_id = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_id==-1) return -1;

	bzero(&servadd, sizeof(servadd));
	hp = gethostbyname(host);
	if(hp == NULL) return -1;
	bcopy(hp->h_addr, (struct sockaddr*)&servadd.sin_addr, hp->h_length);
	servadd.sin_port = htons(portnum);
	servadd.sin_family= AF_INET;
	if(connect(sock_id, (struct sockaddr*)&servadd,sizeof(servadd)) != 0) return -1;

	return sock_id;
}
