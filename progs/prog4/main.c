/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 4                                                  */
/* Brute forces prime numbers, prints progress on interrupt signal           */
/*****************************************************************************/
/* INCLUDES */
#include <stdio.h>
#include <signal.h>

/* GLOBAL VARS */
int countPrimes = 0;//number of primes found
int largestPrime = 0;//value of largest prime found

/* FUNCTION */
int testPrime(int current);
void SIGINT_handler();

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: begins testing, assigns signal to signal handler                 */
/* Parameters:                                                               */
/* Returns:  int              standard 0 return for successful completion    */
/*****************************************************************************/
int main(){
//when we see an interrupt ('^C'), intercept and handle it with this function
	signal(SIGINT,SIGINT_handler);
	
        int run = 2;//this is the current number to test for primality
	while(1){
		testPrime(run++);
	}//test every number FOREVER!!!
	return 0;
	
}

/*****************************************************************************/
/* Function: testPrime                                                       */
/* Purpose: brute force tests a number for primality, also keeps track of    */
/*          the largest prime found and total number of primes found         */
/* Parameters:                                                               */
/*   int current      the current number to be tested for primality          */
/* Returns:  int              0 return for current is prime                  */
/*                            1 return for current not prime                 */
/*****************************************************************************/
int testPrime(int current){
	int tester;//this is the number current will be divided by to see if
                   //current is prime
	for(tester=2;tester < current;tester++){
                //testing every number from 2 to current - 1
                //if the modulo is ever 0, then tester divides current 
                //so current is not prime!
		if(current % tester == 0) return 0;
	}
        //We made it out of the loop, so current is prime
        //Now we test to see if it's the largest prime
	if(current > largestPrime) largestPrime = current;

	countPrimes++;//increment the total number of primes seen
	return 1;
}

/*****************************************************************************/
/* Function: SIGINT_handler                                                  */
/* Purpose: prints current prime finding progress                            */
/* Parameters:                                                               */
/* Returns: void                                                             */
/*****************************************************************************/
void SIGINT_handler(){
	printf("Found %d primes\nLargest was %d\n",countPrimes, largestPrime);
}
