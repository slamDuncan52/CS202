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


int main(){
	char ping[4] = "ping";
	char pong[4] = "pong";
	char forkPrint[4];
	char printOut[4];
	int pingPipe[2];
	int pongPipe[2];
	pipe(pingPipe);
	pipe(pongPipe);
	int isChild = fork();
	while(1){
		if(isChild == 0){
			read(pingPipe[0],forkPrint,sizeof(ping));
			printf("%s\n",forkPrint);
			write(pongPipe[1],pong,sizeof(pong));
			sleep(1);
		} else {
			write(pingPipe[1],ping,sizeof(ping));
			read(pongPipe[0],printOut,sizeof(pong));
			printf("%s\n",printOut);
			sleep(1);
		}

	}
}
