/*************************************************************************/
/* Mitch Duncan                                                          */
/* Login: dunc0474                                                       */
/* CS202 Winter 2014                                                     */
/* Programming Assignment 0                                              */
/* Takes user input number and prints primes below or equal to the input */
/* up to the first MAX primes, at which point it can no longer rigorously*/
/* guarantee primality                                                   */
/*************************************************************************/

#include <stdio.h>

#define MAX 1000 //Maximum array size of stored primes
#define FALSE 0 //standard boolean declarations
#define TRUE 1

/**************************************************/
/*Function: main                                  */
/*Purpose: recieve input, compute and print primes*/
/*Parameters: void                                */
/*Returns: int (non-useful)                       */
/**************************************************/                           
int main(void){
  int array[MAX];//stores known primes to check later numbers
  int modulo;//checks remainders of the number currently 
             //being tested for primality
  int loop1;//loop variable of first for loop, also used as the current number 
            //being tested for primality
  int loop2;//loop variable of second for loop, also used to increment through 
            //array of known primes
  int maxCheck;//makes sure memory locations outside of declared array are not 
               //touched
  int input;//stores user input

printf("Enter value of N > ");
scanf("%d",&input);//take input
maxCheck = 0;
for(loop1=2;loop1<=input;loop1++){//start checking numbers
  for(loop2=0,modulo=TRUE;loop2<maxCheck && modulo;loop2++){ 
//check numbers less than current, break if evenly divisible
    modulo = (loop1%array[loop2]);
                }
  if (modulo){//if there was no evenly divisible number less
    printf("%20d\n",loop1);//print it
    //add it to the known prime array
    if(maxCheck < MAX) array[maxCheck++] = loop1;
    }
  }
return 0;
}
