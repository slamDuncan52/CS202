/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 7 - server                                         */
/* Server side program for playing 2 player swine herd pig                   */
/*****************************************************************************/
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
	int fd;//The player's file descriptor
	char* playerName;//The player's received username
	int score;//The player's safe scored points
	int tempPoints;//The player's unsafe points scored during a round
	int choice;//Holds whether the player is rolling or holding this turn
	char* rollString;//A string representation of the above int
};

char p1[100];//The memory for player one's username
char p2[100];//player two's username
struct player firstPlayer = {0,p1, 0, 0, ROLL,"rolling"};//initialize players
struct player secondPlayer = {0,p2, 0, 0, ROLL,"rolling"};
int isChild = 0;//Flag for handling fork exiting appropriately
int p1Connect = 1;//Flags for testing that the players are connected
int p2Connect = 1;//Set each read or write, non-negative values mean connected

int make_server_socket_q(int portnum, int backlog);
int make_server_socket(int portnum);
void child_waiter(int signum);
void process_request(int fd1, int fd2);

void playPig(struct player p1, struct player p2);
int playTurn(struct player *p1, struct player *p2);
int playRound(struct player *p1, struct player *p2);

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: Opens a socket, sits listening for client players                */
/* Parameters:                                                               */
/*		int argc     not used                                        */
/*		char *argv[] not used                                        */
/* Returns:     int          standard 0 return for successful completion     */
/*****************************************************************************/
int main(int argc, char *argv[]){
	int sock, fd1, fd2;//Socket and recieved file descriptor info
	int port = 50000;//Server will open on port 50000 by default

	signal(SIGCHLD, child_waiter);//Handles zombie cleanup from forking
	signal(SIGPIPE,SIG_IGN);//Prevents crashes on unexpected client exit

	sock = make_server_socket(port);//Use black box to get a socket
	if(sock == -1) exit(1);//Error check the socket
	printf("SERVER START\n");

	while(1){//main server loop
		if(isChild){//We fork later. This makes sure forks don't loop.
			break;
		}

		fd1 = accept(sock, NULL, NULL);//Wait for two clients to
		fd2 = accept(sock, NULL, NULL);//connect

		if(fd1 == -1 || fd2 == -1){//Error check the clients
			if(errno != EINTR) strerror(errno);
			else break;
		}
		process_request(fd1, fd2);//Make the client's game
	}

	return 0;
}

void child_waiter(int signum){
	while(waitpid(-1, NULL, WNOHANG) > 0);//Cleanup zombie processes
}

/*****************************************************************************/
/* Function: process_request                                                 */
/* Purpose: creates two player structs, forks them into a game               */
/* Parameters:                                                               */
/*		int fd1      the socket file descriptor of player 1          */
/*		int fd2      the socket file descriptor of player 2          */
/* Returns:     void                                                         */
/*****************************************************************************/
void process_request(int fd1, int fd2){
	//Get first player
	p1Connect = read(fd1, p1, 100);//Read in player 1's username
	printf("Got First User: %s\n",p1);
	firstPlayer.fd = fd1;//Set as the  player 1 file descriptor

	//Get second player
	p2Connect = read(fd2, p2, 100);//Same for player 2
	printf("Got Second User: %s\n",p2);
	secondPlayer.fd = fd2;

	//make their game
	printf("Make Game\n");
	if(fork() == 0){//FORK HERE!~
		isChild = 1;//Set fork flag
		firstPlayer.playerName = p1;//Set the usernames in structs
		secondPlayer.playerName = p2;

		//Echo player names to both players
		char helloString[250] = "New game:\nPlayer 1 : ";
		strcat(helloString,p1);
		strcat(helloString,"\nPlayer 2 : ");
		strcat(helloString,p2);
		strcat(helloString,"\n");

		p1Connect = write(fd1,helloString,250);//Write to p1
		p2Connect = write(fd2,helloString,250);//Write to p2

		playPig(firstPlayer, secondPlayer);//Play their game
		printf("GAME FINISHED BETWEEN %s AND %s\n",p1,p2);
	}
	close(fd1);//Cleanup after ourselves
	close(fd2);
}

/*****************************************************************************/
/* Function: playPig                                                         */
/* Purpose: sits and loops waiting for the game between the players to end   */
/* Parameters:                                                               */
/*	        struct player p1     the player struct of the first player   */
/*		struct player p2     the player struct of the second player  */
/* Returns:     void                                                         */
/*****************************************************************************/
void playPig(struct player p1, struct player p2){
	int nextTurn = 0;//flag for finishing a game (passed up)
	srand((unsigned)time(NULL));//Seed the random roller for this game
	while(!nextTurn){//A game is a series of turns
		nextTurn = playTurn(&p1,&p2);//so play turns until finished
	}

}

/*****************************************************************************/
/* Function: playTurn                                                        */
/* Purpose: plays a number of rounds of pig, waiting for a one to be rolled  */
/*          or both players to hold. It then handles data updating           */
/*          checks if the game is done, communicates this info to the client */
/* Parameters:                                                               */
/*	        struct player *p1     the player struct of the first player  */
/*		struct player *p2     the player struct of the second player */
/* Returns:     int status           the game status. 0 for unfinished       */
/*                                                    1 for a victory        */
/*                                                    2 for a tie            */
/*****************************************************************************/
int playTurn(struct player *p1, struct player *p2){
	int nextRound = 0;//flag for finishing a turn (set below)
	int status;//Flag for finishing a game (passed up)
	char statusStr[300];//memory for info string to be passed to clients

	//Play a round
	while(!nextRound){//A turn is a series of rounds
		nextRound = playRound(p1,p2);//so play rounds until finished
	}

	//Update Total Scores
	p1->score += p1->tempPoints;//Total scores have round scores added to
	p2->score += p2->tempPoints;//them
	p1->tempPoints = 0;//round scores are reset
	p2->tempPoints = 0;

	//Reset player choices
	p1->choice = ROLL;//Players begin a turn rolling
	p1->rollString = "rolling";
	p2->choice = ROLL;
	p2->rollString = "rolling";

	//Check if game is done
	if((p1->score >= 100 || p2->score >= 100) && (p1->score != p2->score)){
		status = WIN;//The game has finished decisively
		if(p1->score > p2->score)
		{//p1 wins here. Client's will be sent a message as such
			sprintf(statusStr,"\n%s wins\nby a score of %d - %d\n",
			p1->playerName,p1->score,p2->score);
		} 
		else 
		{//p2 wins here. Client message is set
			sprintf(statusStr,"\n%s wins\nby a score of %d - %d\n",
			p2->playerName,p1->score,p2->score);
		}
	} else if((p1->score>=100 || p2->score>=100)&&(p1->score==p2->score)){
		status = TIE;//The game has finished in a tie.
		sprintf(statusStr,"\nTie game\nFinal score %d - %d\n",
		p1->score,p2->score);//Client message is set as such
	} else {//The game has not yet finished!
		status = CONT;//Set a message of the current scores
		sprintf(statusStr,"\nEnd of turn scores:\n%s: %d\n%s: %d\n",
		p1->playerName,p1->score,p2->playerName,p2->score);
	}
	
	//Check if players still connected
	if(p1Connect == 0 || p1Connect == -1){//If player 1 disconnected
		sprintf(statusStr,"\n%s Disconnected, %s wins by default!\n",
		p1->playerName,p2->playerName);//Send a message to player 2
		status = -1;//Set status that game exited early
		p2Connect = write(p2->fd,statusStr,300);
		p2Connect = write(p2->fd,&status,sizeof(int));
		return status;
	} else if(p2Connect == 0 || p2Connect == -1){//similar for p2 dc
		sprintf(statusStr,"\n%s Disconnected, %s wins by default!\n",
		p2->playerName,p1->playerName);
		status = -1;		
		p1Connect = write(p1->fd,statusStr,300);
		p1Connect = write(p1->fd,&status,sizeof(int));
		return status;
	}
//If everyone still connected
	p1Connect = write(p1->fd,statusStr,300);//Write the status
	p2Connect = write(p2->fd,statusStr,300);//to be read by user

	p1Connect = write(p1->fd,&status,sizeof(int));//Send status for
	p2Connect = write(p2->fd,&status,sizeof(int));//client program
	return status;
}

/*****************************************************************************/
/* Function: playRound                                                       */
/* Purpose: plays a single round of pig, rolling and adding to temp scores   */
/*          until both players hold or a one is rolled. Handles one rolling  */
/*          protocol, most of the messages seen by the player at the client  */
/* Parameters:                                                               */
/*	        struct player *p1    the player struct of the first player   */
/*		struct player *p2    the player struct of the second player  */
/* Returns:     int                  the turn status. 0 for unfinished       */
/*                                                    1 for both players held*/
/*                                                    2 for a one rolled     */
/*****************************************************************************/
int playRound(struct player *p1, struct player *p2){
	char writeBuf[250];//The memory used to send clients the end of round
			   //message
	int returnStatus = 0;//Status for turn end (passed up)
	int oneFlag = 0;//Flag that a one has been rolled
			//Necessary for client to know 
	//Roll the die!
	int curDie = (rand() % 6) + 1;

	//If 1 is rolled
	if(curDie == 1){
		returnStatus = 2;//Means a one was rolled up above
		oneFlag = 1;//Tells clients to handle a one roll
		p1Connect = write(p1->fd,&oneFlag,sizeof(int));//Lets clients
		p2Connect = write(p2->fd,&oneFlag,sizeof(int));//know
		
		if(p1->choice == ROLL && p2->choice == ROLL){//If both rolling
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d "
					"points banked\n%s had %d points "
					"banked\nAnd you both lost them!",
			p1->playerName,p1->tempPoints,p2->playerName,
			p2->tempPoints);//Set the message appropriately
			p1->tempPoints = 0;//Eliminate all the unsafe points
			p2->tempPoints = 0;
		}

		if(p1->choice == ROLL && p2->choice == HOLD){//p2 held
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d"
					" points banked\n%s had %d points "
					"banked\n%s lost them, but %s's are "
					"safe!",
		p1->playerName,p1->tempPoints,p2->playerName,p2->tempPoints,
		p1->playerName,p2->playerName);
			p1->tempPoints = 0;//Only player one loses points
		}

		if(p1->choice == HOLD && p2->choice == ROLL){//p1 held
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d "
					"points banked\n%s had %d points "
					"banked\n%s's are safe, but %s lost "
					"them!",
			p1->playerName,p1->tempPoints,p2->playerName,
			p2->tempPoints,p1->playerName,p2->playerName);
			p2->tempPoints = 0;//Only player two loses points
		}

	        if(p1->choice == HOLD && p2->choice == HOLD){//both held
			sprintf(writeBuf,"Uh oh! A one was rolled!\n%s had %d "
					"points banked\n%s had %d points "
					"banked\nAnd they're all safe!",
			p1->playerName, p1->tempPoints,p2->playerName,
			p2->tempPoints);//No points lost!
		}

		p1Connect = write(p1->fd,writeBuf,250);//Do the writes
		p2Connect = write(p2->fd,writeBuf,250);
		p1Connect = write(p1->fd,&returnStatus,sizeof(int));
		p2Connect = write(p2->fd,&returnStatus,sizeof(int));
		return returnStatus;
	}

	//Update temp scores
	if(p1->choice == ROLL){p1->tempPoints += curDie;}
	if(p2->choice == ROLL){p2->tempPoints += curDie;}

	//Announce current state
	p1Connect = write(p1->fd,&oneFlag,sizeof(int));//Lets clients know a 
	p2Connect = write(p2->fd,&oneFlag,sizeof(int));//one was not rolled
	sprintf(writeBuf,"The current roll is: %d\n%s has %d points banked and"
			" is %s\n%s has %d points banked and is %s\nCurrent "
			"score is:\n""%s: %d\n%s: %d\n",
			curDie,p1->playerName,p1->tempPoints,p1->rollString,
			p2->playerName,p2->tempPoints,p2->rollString,
			p1->playerName,p1->score,p2->playerName,p2->score);
	p1Connect = write(p1->fd,writeBuf,250);//Writes the big status message
	p2Connect = write(p2->fd,writeBuf,250);
	
	//Request rollers to hold
	if(p1->choice == ROLL){//Player one is rolling?
		p1Connect = read(p1->fd,&p1->choice,sizeof(int));//Get response
		if(p1->choice == ROLL){p1->rollString = "rolling";}
		else{p1->rollString = "holding";}
	}
	if(p2->choice == ROLL){//Player two is rolling?
		p2Connect = read(p2->fd,&p2->choice,sizeof(int));//Get response
		if(p2->choice == ROLL){p2->rollString = "rolling";}
		else{p2->rollString = "holding";}
	}
	if(p1->choice == HOLD && p2->choice == HOLD){//If both now holding
		returnStatus = 1;//Turn is over!
		p1Connect = write(p1->fd,&returnStatus,sizeof(int));
		p2Connect = write(p2->fd,&returnStatus,sizeof(int));
		return returnStatus;
	}

	p1Connect = write(p1->fd,&returnStatus,sizeof(int));//Otherwise
	p2Connect = write(p2->fd,&returnStatus,sizeof(int));//continue
	return returnStatus;
}

/*****************************************************************************/
/* Function: make_server_socket                                              */
/* Purpose: Black box magic kernel code to make a server socket appear       */
/* Parameters:                                                               */
/*	        int portnum           the port on which to open the socket   */
/*		int backlog           a black box parameter required by magic*/
/* Returns:     int sock_id           the socket number on the network       */
/*****************************************************************************/
int make_server_socket(int portnum){//who knows
	return make_server_socket_q(portnum, BACKLOG);
}

int make_server_socket_q(int portnum, int backlog){//not me
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
