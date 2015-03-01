#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE 1000

int appendFlag = 0;
int interuptFlag = 0;
char * stdInGrab();

int main(int argc, char *argv[]){
	/* Handle Args */
	int argCount = 1;
	int fileCount = -1;
	char *fileList[argc];
	while(argCount < argc){
		if(!strcmp(argv[argCount], "-a")){
			appendFlag = 1;
		} else if(!strcmp(argv[argCount], "-i")){
			interuptFlag = 1;
		} else {
			fileList[++fileCount] = argv[argCount];
		}
		argCount++;
	}
	/* Grab all the input */
	char *inputString = stdInGrab();

	/* Write to each file */
	FILE *currentFile;
	while(fileCount >= 0){
		if(appendFlag){
			currentFile = fopen(fileList[fileCount--], "a");
		} else {
			currentFile = fopen(fileList[fileCount--],"w");
		}	
		fputs(inputString,currentFile);
		close(currentFile);
	}
	return 0;
}

char * stdInGrab(){
	int numOfLines = 1;
	char *returnString;
	char *sizeSwap;
	char lineBuffer[BLOCKSIZE];
	if(interuptFlag){
		signal(SIGINT, SIG_IGN);
	}

	sizeSwap = malloc(numOfLines * sizeof(char) * BLOCKSIZE);
	returnString = malloc(numOfLines * sizeof(char) * BLOCKSIZE);
	while(fgets(lineBuffer, BLOCKSIZE, stdin) != NULL){
		if((strlen(lineBuffer) + strlen(returnString)) > (BLOCKSIZE * numOfLines)){
			strcpy(sizeSwap, returnString);
			returnString = malloc(numOfLines* sizeof(char) * BLOCKSIZE);
			strcpy(returnString, sizeSwap);
			sizeSwap = malloc(numOfLines * sizeof(char) * BLOCKSIZE);
		}
		strcat(returnString, lineBuffer);
		numOfLines++;
	}
	printf("%s",returnString);
	return returnString;
}
