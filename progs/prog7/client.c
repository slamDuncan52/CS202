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

char user[100];//Memory for given username
char host[100];//Memory for given hostname

int fd;//File descriptor of server
int gameStatus = 0;//Status from server for exiting game
int choice = ROLL;//Choice of roll or hold

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: Gets a username, connects to the server, begins playing a game   */
/* Parameters:                                                               */
/*		int argc     number of arguments passed to program           */
/*		char *argv[] additional port argument may be passed          */
/* Returns:     int          standard 0 return for successful completion     */
/*****************************************************************************/
int main(int argc, char* argv[]){
	int port;//Port to connect to server on
	char handShakeBuf[250];//Memory for usernames print out
	if((argc > 1) && atoi(argv[1]) != 0){//If an arg is given
		port = atoi(argv[1]);//it's for the port number
	} else {
		port = 50000;//Otherwise, default to 50000
	}

	printf("Welcome to Herd Pig! Please enter hostname >>> ");
	scanf("%s",host);//prompt for hostname, read it in
	printf("Please enter a username (100 characters or less) >>> ");
	scanf("%s",user);//read in username

	fd = connect_to_server(host,port);//Use magic to connect to the server
	if(fd == -1) exit(1);//Error check that fd
	printf("Connected to Host: %s @ Port: %d\n",host,port);//Echo it out

	write(fd,user,100);//Send username to server
	printf("Waiting for opponent...\n");//Wait for server to handshake
	int val = read(fd, handShakeBuf, 250);
	if(val == 250){//check val here to verify server is sending properly
		printf("%s\n",handShakeBuf);//Print out username of self and opponent
	}

	//PLAY GAME
	while(!gameStatus){//A game is a series of turns
		gameStatus = playTurn();//Play turns until its over
	}
	close(fd);//clean up clean up!
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
	int gameStatus;//overall game status from server (passed up)
	char statusStr[300];//The status string to be printed at end of turn
	int turnStatus = 0;//The status of the current turn (set below)

	choice = ROLL;//reset roll/hold each turn
	while(!turnStatus){//a turn is a series of rounds
		turnStatus = playRound();//play rounds until it ends
	}

	read(fd,statusStr,300);//Get that status string
	printf("%s",statusStr);//print it
	read(fd,&gameStatus,sizeof(int));//Get that status int
	return gameStatus;//send it up
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
	int status;//Turn status (passed up)
	int oneCheck;//set by server to let us know a one was rolled
	char choiceBuf;//memory for getting user input about rolling/holding
	char statusBuf[250];//memory for printing round information
	char oneStatus[100];//memory for printing info when a one is rolled

	read(fd,&oneCheck,sizeof(int));//Check if a one was rolled
	read(fd,statusBuf,250);//Get the current round status
	printf("\n%s\n",statusBuf);//Print it

	if(choice == ROLL && oneCheck == 0){//If we're rolling (no one rolled)
		printf("Do you wish to continue rolling? ('y' or 'n') >>> ");
		while(1){//Until good input is given
			fflush(stdout);
			choiceBuf = getchar();
			if(choiceBuf == 'y'){ choice = ROLL;break;}
			if(choiceBuf == 'n'){ choice = HOLD;break;}
			if(choiceBuf != '\n'){
		printf("Do you wish to continue rolling? ('y' or 'n') >>> ");
			}
		}
		write(fd,&choice,sizeof(int));//Send server the choice
	} 
	else if (choice == HOLD && !oneCheck){//If we're holding
		printf("Waiting for opponent to hold\n");//Opponent can keep
				    //rolling until 1 or rolled or they hold
	}

	read(fd,&status,sizeof(int));//Get the turn status
	return status;//send it up
}

/*****************************************************************************/
/* Function: connect_to_server                                               */
/* Purpose: handles networking black box code, talking kernel                */ 
/* Parameters:  char *host              the network host to connect to       */      
/*              int portnum             the port of said network host        */
/* Returns:     int         returns turn status: 0 if continuing,            */
/*			    otherwise means turn over                        */
/*****************************************************************************/
int connect_to_server(char *host, int portnum){//This is a mystery 8^Y
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
