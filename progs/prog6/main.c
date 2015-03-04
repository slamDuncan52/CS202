/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 6                                                  */
/* Takes std in, copies it to files provided as arguments                    */
/*****************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define BLOCKSIZE 1000

int appendFlag = 0;
int interuptFlag = 0;
void stdInGrab(char** fileList, int fileCount);

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: handles args, handles file IO, calls function to pull stdin      */
/* Parameters:                                                               */
/*		int argc     number of arguments passed to program           */
/*		char *argv[] name of files to print to, additional options   */
/* Returns:     int          standard 0 return for successful completion     */
/*****************************************************************************/
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
	stdInGrab(fileList, fileCount);
	/* Write to each file */
	return 0;
}

/*****************************************************************************/
/* Function: stdInGrab                                                       */
/* Purpose: Takes arbitrary amount of stdin, copies it to a single string    */
/* Parameters:                                                               */
/*		char** inputString     the memory address of our final string*/
/* Returns:     void                                                         */
/*****************************************************************************/
void stdInGrab(char** fileList, int fileCount){
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
		printf("%s",lineBuffer);
	}
	FILE *currentFile;
	while(fileCount >= 0){
		if(appendFlag){
			currentFile = fopen(fileList[fileCount--],"a");
		} else {
			currentFile = fopen(fileList[fileCount--],"w");
		}
		fputs(returnString,currentFile);
		close(currentFile);
	}
	return;
}
