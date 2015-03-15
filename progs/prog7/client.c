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
void talkServer(int fd, char* msg);
void handShake();
int playTurn();
int updateStatus();

char user[100];
char host[100];

int fd;
int gameStatus = 0;
int curScore = 0;
int oppScore = 0;
int curRoll = 0;

int main(int argc, char* argv[]){
	int port;
	if((argc > 1) && atoi(argv[1]) != 0){
		port = atoi(argv[1]);
	} else {
		port = 50000;
	}
	printf("Welcome to Herd Pig! Please enter hostname >>> ");
	scanf("%s",&host);
	printf("Please enter a username (100 characters or less) >>> ");
	scanf("%s",&user);
	fd = connect_to_server(host, port);
	printf("Connected to Host: %s @ Port: %d\n",host,port);
	if(fd == -1) exit(1);
	talkServer(fd, user);
	handShake();
	while(!gameStatus){
		playTurn();
		gameStatus = updateStatus();
	}
	close(fd);
	return 0;
}

void talkServer(int fd, char* msg){
	write(fd, msg, 100);
	printf("Sent\n");
}

void handShake(){
	while(1){
		printf("Waiting for opponent...\n");
		char* handShakeBuf[250];
		int val = read(fd, handShakeBuf, 250);
		if(val == 250){
			printf("%s\n",handShakeBuf);
			break;
		}
		sleep(5);
	}
}

int playTurn(){
	char choice = ' ';
	char score_buf[7];
	read(fd, score_buf,7);
	curScore = atoi(score_buf);
	oppScore = atoi(score_buf+4);
	printf("Your Current Total is: %d\n", curScore);
	printf("Your Opponent's Total is: %d\n", oppScore);
	printf("The Amount Rolled This Round is: %d\n", curRoll);
	printf("Do you wish to (r)oll or (h)old? >>> ");
	scanf("%c", &choice);
}

int updateStatus(){
	int status;
	if(curScore >= 100 && curScore > oppScore){
		status = WIN;
		printf("Congratulations! You win!");
	} else if(oppScore >= 100 && oppScore > curScore){
		status = LOSE;
		printf("You lost, better luck next time!");
	} else if(curScore >= 100 && curScore == oppScore){
		status = TIE;
		printf("The game ends in a tie.");
	} else {
		status = 0;
	}
	return status;
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
