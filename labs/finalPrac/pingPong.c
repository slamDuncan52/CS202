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
	int pipeArr[2];
	int isChild = fork();
	pipe(pipeArr);
	while(1){
		if(isChild){
			read(pipeArr[0],forkPrint,sizeof(ping));
			printf("%s\n",forkPrint);
			write(pipeArr[1],pong,sizeof(pong));
			sleep(1);
		} else {
			write(pipeArr[1],ping,sizeof(ping));
			read(pipeArr[0],printOut,sizeof(pong));
			printf("%s\n",printOut);
			sleep(1);
		}

	}
}
