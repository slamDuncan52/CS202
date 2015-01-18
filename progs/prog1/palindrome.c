/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 1                                                  */
/* Identifies double-base palindromes and prints them to standard output, up */
/* to a user defined limit (to an absolute max of 2^100)                     */
/*****************************************************************************/
#include <stdio.h>

#define MAX_DIGITS 100

char* convertToArray(int inputNum, char inArray[], int base);
int testPalindrome(char input[]);

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: grab user input, handle program flow for each number being tested*/
/* Parameters:                                                               */
/*   int argc                 not used                                       */
/*   char **argv              not used                                       */
/* Returns:  int              standard 0 return for successful completion    */
/*****************************************************************************/
int main(int argc, char **argv){

	int userInput,current,total=0;
	/* userInput: maximum on range of numbers to test for palindrome
	   current: the current number being tested for being a palindrome
	   total: running sum of double-base palindromic numbers */ 
	char intArray[MAX_DIGITS];
	/* intArray: holds the current test number as a string array of 
	             its digits for easy palindrome testing */ 
	printf("Enter max number > ");//prompt user for upper limit 
	scanf("%d",&userInput);//grab that input
	for(current=0;current<userInput;current++){//for every int 0 to limit
		convertToArray(current, intArray, 10);//convert int to string
		if(testPalindrome(intArray)){//test if the string is palindrome
			convertToArray(current, intArray, 2);
			// if so, now convert int to binary string
			if(testPalindrome(intArray)){
				//test binary string for palindrome
				printf("%d %s\n",current,intArray);//if so, print
				total += current;//add to total
			}
		}
	}
	printf("Sum: %d\n",total);//print the total
	return 0;
}

/*****************************************************************************/
/* Function: convertToArray                                                  */
/* Purpose: convert an integer to an array of its digits, in a given base,   */
/*          as a string                                                      */
/* Parameters:                                                               */
/*   int inputNum                 the number to be converted                 */
/*   char inArray[]            the string where the digits will be placed    */
/*   int base                  the base to convert the number to             */
/* Returns:  char*             the string with the number placed in it       */
/*****************************************************************************/
char* convertToArray(int inputNum, char inArray[], int base){
	int backPosition = MAX_DIGITS-1;
	int frontPosition = 0;
	/* backPosition: keeps track of the beginning of the string 
	                 at the back of the array
	   frontPosition: keeps track of the beginning of the string 
	                  at the front of the array
	 */
	if(inputNum == 0){//0 is a boundary case, handle it special
		inArray[0] = '0';
		inArray[1] = '\0';
		return inArray;//just return string "0"
	}
	while(inputNum > 0){//put the rightmost digit of the input
		//at the rightmost unused slot of the string array.
		inArray[backPosition--] = (inputNum%base) + '0';
		inputNum = inputNum/base;//eliminate that digit from input
	}
	backPosition += 1;//adjust for overstep at the end
	while(backPosition < MAX_DIGITS){//move the string to the front
		                         //of the array
		inArray[frontPosition++] = inArray[backPosition++];
	}
	inArray[frontPosition] = '\0';//end the string properly
	return inArray;
}

/*****************************************************************************/
/* Function: testPalindrome                                                  */
/* Purpose: takes a string and returns whether that string is a palindrome   */
/* Parameters:                                                               */
/*   char input[]             the string to be tested                        */
/* Returns:  int              1 for palindrome, 0 for not a palindrome       */
/*****************************************************************************/
int testPalindrome(char input[]){
	int length = 0, counter = 0;
	/*length: the length of the input string
	  counter: loop variable which keeps track of our place in the string*/
	while(input[length] != '\0'){//find end of string, counting length
		length++;
	}	
	for(counter = 0; counter < length; counter++){
		//starting at the ends and moving in
		//check palindromic pairs of characters for equality
		if(input[counter] != input[(length-1)-counter]){
			//if they're ever unequal, return failure
			return 0;
		}
	}
	//if they're all equal return success
	return 1;
}
