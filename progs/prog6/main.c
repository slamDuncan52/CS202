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

int appendFlag = 0;//Flag variables for command line options
int interuptFlag = 0;
void stdInGrab();

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: handles args, handles file IO, calls function to pull stdin      */
/* to a temp file                                                            */
/* Parameters:                                                               */
/*		int argc     number of arguments passed to program           */
/*		char *argv[] name of files to print to, additional options   */
/* Returns:     int          standard 0 return for successful completion     */
/*****************************************************************************/
int main(int argc, char *argv[]){
	/* Handle Args */
	int argCount = 1;//parallel to argc, counts up number of args seen
	int fileCount = -1;//counts number of files to copy to
	char *fileList[argc];//string array which holds the file names
	while(argCount < argc){
		if(!strcmp(argv[argCount], "-a")){//if we see the flags
			appendFlag = 1;
		} else if(!strcmp(argv[argCount], "-i")){
			interuptFlag = 1;//set them
		} else {//otherwise, assume a file name
			fileList[++fileCount] = argv[argCount];
		}
		argCount++;
	}
	/* Grab all the input */
	FILE *templateFile;//This will be the temp file created in stdInGrab();
	stdInGrab();//Call The function to fill that temp file
	templateFile = fopen("temp", "r");//open it for reading here
	/* Write to each file */
	FILE *currentFile;//This will be the current file to write to
			  //From fileList
	char nextChar;//This is the current character being copied/written
	while(fileCount >= 0){//for all given files
		if(appendFlag){//open them for write or append
			currentFile = fopen(fileList[fileCount--],"a");
		} else {
			currentFile = fopen(fileList[fileCount--],"w");
		}
		while(1){
			nextChar = fgetc(templateFile);//read from template
			if(nextChar == EOF){//get out if we reach the end
				break;
			}
			putc(nextChar, currentFile);//write to current outfile
		}
		fclose(currentFile);//close this given file
		rewind(templateFile);//set template to the beginning
	}
	fclose(templateFile);//close the template
	unlink("temp");//delete the template
	return 0;
}

/*****************************************************************************/
/* Function: stdInGrab                                                       */
/* Purpose: Creates a temporary file which has all stdin copied to it, as    */
/* well as to stdout                                                         */
/* Parameters:  none                                                         */
/* Returns:     void                                                         */
/*****************************************************************************/
void stdInGrab(){
	FILE *tempFile;//This is the temporary file which holds stdin
	tempFile = fopen("temp", "w");//open it for writing
	char curChar;//This is the current char read from stdin
	if(interuptFlag){//if flag set
		signal(SIGINT, SIG_IGN);//ignore Ctrl-C
	}
	/* Redirect stdin to my temp file */
	while((curChar = getchar()) != EOF){//Until user hits Ctrl-D
		putc(curChar, stdout);//grab character, put it to stdout
		putc(curChar,tempFile);//put it to our temporary file
	}
	fclose(tempFile);//close the temporary file
	return;
}
