#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <strings.h>
#define PORTNUM 13000
#define HOSTLEN 256
#define oops(msg) {perror(msg) ; exit(1);}

int main(int ac, char *av[]){
	struct sockaddr_in saddr;
	struct hostent *hp;
	char hostname[HOSTLEN];
	int sock_id, sock_fd;
	FILE *sock_fp;
	time_t theTime;

	sock_id = socket(PF_INET, SOCK_STREAM, 0);
	if(sock_id ==-1) oops("socket");

	bzero( (void *)&saddr, sizeof(saddr));

	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(PORTNUM);
	saddr.sin_family = AF_INET;

	if(bind(sock_id, (struct sockaddr*)&saddr, sizeof(saddr)) != 0)
		oops("bind");

	if(listen(sock_id, 1) != 0) oops("listen");

	while(1){
		sock_fd = accept(sock_id, NULL, NULL);
		printf("Wow! Got a call!\n");
		if(sock_fd==-1) oops("accept");

		sock_fp = fdopen(sock_fd, "w");
		if(sock_fp == NULL) oops("fdopen");

		theTime = time(NULL);
		fprintf(sock_fp, "The time here is..");
		fprintf(sock_fp, "%s", ctime(&theTime));
		fclose(sock_fp);
	}
}
