#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int connect_to_server(char*, int);
void talkServer(int fd);

int main(){
	int fd;

	char * host = "syslab09";
	int port = 13000;
	fd = connect_to_server(host, port);
	printf("Connected\n");
	if(fd == -1) exit(1);
	talkServer(fd);
	close(fd);
	return 0;
}

void talkServer(int fd){
	printf("Sent.\n");
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
