#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

int main(){
	int sum = 0;
	int pid = fork();
	if(pid > 0){
		sum +=2;
		wait(NULL);
	} else if (pid == 0){
		sum +=3;
		exit(0);
	}
	printf("2 + 3 = %i\n", sum);
	return 0;
}
