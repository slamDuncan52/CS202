#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

void on_alarm(int signum){
	signal(SIGALRM, on_alarm);
	printf("Tick\n");
}

int main(){
	signal(SIGALRM, on_alarm);
	alarm(1);
	while(1){
		pause();
	}
}
