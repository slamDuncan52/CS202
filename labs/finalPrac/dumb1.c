#include <unistd.h>
#include <stdio.h>
/* This program executes two separate programs in order,
 * * without any arguments supplied to either program
 * * For example, running this program like this:
 * * a.out ls date
 * * will first run "ls", then run "date". */
int main (int argc, char *argv[]) {
	if (argc != 3) { /* check proper usage */
		printf("usage: %s prog1 prog2\n", argv[0]);
		return -1;
	}
	execvp(argv[1], NULL); /* run the first one */
	execvp(argv[2], NULL); /* and then the other */
	return 0;
}
