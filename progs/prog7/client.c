/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 7 - client                                         */
/* Client side console for playing 2 player swine herd pig                   */
/*****************************************************************************/
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
int playTurn();
int playRound();
int updateStatus();

char user[100];
char host[100];

int fd;
int gameStatus = 0;
int choice = ROLL;

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: Gets a username, connects to the server, begins playing a game   */
/* Parameters:                                                               */
/*		int argc     number of arguments passed to program           */
/*		char *argv[] additional port argument may be passed          */
/* Returns:     int          standard 0 return for successful completion     */
/*****************************************************************************/
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
	printf("Waiting for opponent...\n");
	char handShakeBuf[250];
	int val = read(fd, handShakeBuf, 250);
	if(val == 250){
		printf("%s\n",handShakeBuf);
	}

	//PLAY GAME
	while(!gameStatus){
		gameStatus = playTurn();
	}
	close(fd);
	return 0;
}

/*****************************************************************************/
/* Function: playTurn                                                        */
/* Purpose: handles one turn of the game, playing a number of rounds,        */
/*          resetting the roll/hold choice each round                        */
/*	    and checking if the game is over after each turn                 */
/* Parameters:                                                               */
/* Returns:     int         returns game status: 0 if continuing,            */
/*			    otherwise means game over                        */
/*****************************************************************************/
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

/*****************************************************************************/
/* Function: playRound                                                       */
/* Purpose: handles one round of the game, getting roll/hold input and       */ 
/*	    listening if a one is rolled or both players held                */
/*	    and checking if the game is over after each round                */
/* Parameters:                                                               */
/* Returns:     int         returns turn status: 0 if continuing,            */
/*			    otherwise means turn over                        */
/*****************************************************************************/
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
	} 
	else if (choice == HOLD && !oneCheck){
		printf("Waiting for opponent to hold\n");
	}

	read(fd,&status,sizeof(int));	
	return status;
}

/*****************************************************************************/
/* Function: connect_to_server                                               */
/* Purpose: handles networking black box code, talking kernel                */ 
/* Parameters:  char *host              the network host to connect to       */      
/*              int portnum             the port of said network host        */
/* Returns:     int         returns turn status: 0 if continuing,            */
/*			    otherwise means turn over                        */
/*****************************************************************************/
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
	if(connect(sock_id, (struct sockaddr*)&servadd,sizeof(servadd)) != 0){
		return -1;
	}
	return sock_id;
}
