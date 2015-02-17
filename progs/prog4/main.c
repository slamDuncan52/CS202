#include <stdio.h>
#include <signal.h>

int countPrimes = 0;
int largestPrime = 0;

int testPrime(int current);
void int_handler();

int main(){
	signal(SIGINT,int_handler);
	int run = 2;
	while(1){
		testPrime(run++);
	}
	return 0;
}

int testPrime(int current){
	int tester;
	for(tester=2;tester < current;tester++){
		if(current % tester == 0) return 0;
	}
	if(current > largestPrime) largestPrime = current;
	countPrimes++;
	return 1;
}

void int_handler(){
	printf("Found %d primes\nLargest was %d\n",countPrimes, largestPrime);
}
