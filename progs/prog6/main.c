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
#include <unistd.h>

#define BLOCKSIZE 1000

int appendFlag = 0;
int interuptFlag = 0;
void stdInGrab();

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
	FILE *templateFile;
	stdInGrab();
	templateFile = fopen("temp", "r");
	/* Write to each file */
	FILE *currentFile;
	char nextChar;
	while(fileCount >= 0){
		if(appendFlag){
			currentFile = fopen(fileList[fileCount--],"a");
		} else {
			currentFile = fopen(fileList[fileCount--],"w");
		}
		while(1){
			nextChar = fgetc(templateFile);
			if(nextChar == EOF){
				break;
			}
			putc(nextChar, currentFile);
		}	
		fclose(currentFile);
		rewind(templateFile);
	}
	fclose(templateFile);
	unlink("temp");
	return 0;
}

/*****************************************************************************/
/* Function: stdInGrab                                                       */
/* Purpose: Takes arbitrary amount of stdin, copies it to a single string    */
/* Parameters:                                                               */
/*		char** inputString     the memory address of our final string*/
/* Returns:     void                                                         */
/*****************************************************************************/
void stdInGrab(){
	FILE *tempFile;
	tempFile = fopen("temp", "w");
	int tempFileFD;
	char curChar;
	if(interuptFlag){
		signal(SIGINT, SIG_IGN);
	}
	/* Redirect stdin to my temp file */
	//tempFileFD = open("temp", O_WRONLY);
	//pipe(pipeArray);
	while((curChar = getchar()) != EOF){
		putc(curChar, stdout);	
		putc(curChar,tempFile);
	}
	/* Close the pipe */
	fclose(tempFile);
	return;
}
