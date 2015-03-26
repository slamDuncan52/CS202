#include <signal.h>
#include <unistd.h>
#include <stdio.h>
/* This program creates an un-interruptable clock, which
 * * prints a "tick" every second. Once started, the clock
 * * cannot be stopped by any signal. */
int main (int argc, char *argv[]) {
	int signum;
	for (signum = 0; signum < _NSIG; signum++)
		signal(signum, SIG_IGN);
	while(1) {
		sleep(1);
		printf("tick ...\n");
	}
}
