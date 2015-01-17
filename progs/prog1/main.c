#include <stdio.h>

#define MAX_DIGITS 100

int testPalindrome(char input[]);
char* intToArray(int input, char inArray[]);
char * convertToBinary(int input, char inArray[]);

int main(int argc, char **argv){

int userInput,i,total=0;
char intArray[MAX_DIGITS], binArray[MAX_DIGITS];
printf("Enter max number > ");
scanf("%d",&userInput);
	for(i=0;i<userInput;i++){
	intToArray(i, intArray);
	if(testPalindrome(intArray)){
		convertToBinary(i, binArray);
		if(testPalindrome(binArray)){
			printf("%d, %s\n",i,binArray);
			total += i;
			}
		}
	}
printf("Total: %d\n",total);
	return 0;
}

char * intToArray(int input, char inArray[]){
int position = 0;
while(input > 0){
inArray[position++] = (input%10) + '0';
input = input / 10;
}
inArray[position] = '\0';
return inArray;
}

int testPalindrome(char input[]){
int length = 0, counter = 0;
while(input[counter++] != '\0'){
length++;
}	
for(counter = 0; counter < length; counter++){
if(input[counter] != input[(length-1)-counter]){
return 0;
}
}
return 1;
	}

char *  convertToBinary(int input, char inArray[]){
int backPosition = MAX_DIGITS-1;
int frontPosition = 0;
while(input > 0){
inArray[backPosition--] = (input%2) + '0';
input = input/ 2;
}
backPosition += 1;
while(backPosition < MAX_DIGITS){
inArray[frontPosition++] = inArray[backPosition++];
}
inArray[frontPosition] = '\0';
return inArray;
}
