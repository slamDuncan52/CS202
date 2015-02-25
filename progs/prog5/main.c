/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 5                                                  */
/* Scrolls a file's contents ticker-tape style across the screen             */
/*****************************************************************************/
/* INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>

/* DEFINES */
#define MICROTOMILLI 1000

/* FUNCTION DECLARATIONS */
int scroller(char *inString);

/* GLOBAL VARIABLES */
int tickInterval = 1000;//The number of milliseconds between every advance of
//text on screen

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: handles args, reads file into char* buffer, calls curses function*/
/* Parameters:                                                               */
/*		int argc     number of arguments passed to program           */
/*		char *argv[] name of file to scroll, additional options      */
/* Returns:     int          standard 0 return for successful completion     */
/*****************************************************************************/
int main(int argc, char *argv[]){

	/* FUNCTION VARS */
	FILE *scrollFile;//The file from which text will be scrolled
	char *scrollText;//The buffer which the file's text is read into
	int fileLen = 0;//The size of our file, in bytes
	int argCount = 1;//counts up to argc for argument handling
	int infiniteFlag = 0;//set by shell option '-l' loops the string
	int charCheck = 0;//used to go through the string and sanitize it

	/* HANDLE ARGS */
	while(argCount < argc){
		if(!strcmp(argv[argCount], "-t")){//if '-t' was set
			//read the next argument as our new tickInterval
			tickInterval = (int)strtol(argv[++argCount],NULL,10);
		} else if(!strcmp(argv[argCount], "-l")){//if '-l' was set
			infiniteFlag = 1;//set the infinite flag
		} else {
			//otherwise, the argument must be our file name!
			scrollFile = fopen(argv[argCount], "r");
		}
		argCount++;
	}

	/* READ FILE INTO STRING */
	fseek(scrollFile, 0L, SEEK_END);//seek to the end of the file
	fileLen = ftell(scrollFile);//get position, which is the filesize
	rewind(scrollFile);//seek back to the beginning of the file
	scrollText = malloc(fileLen * (sizeof(char)));//allocate buffer space
	fread(scrollText, sizeof(char), fileLen, scrollFile);//read the file!

	/* SANITIZE THE STRING */
	while(scrollText[charCheck] != '\0'){
		//Newline and tab characters don't scroll well
		// so replace them with spaces!
		if(scrollText[charCheck]=='\n'||scrollText[charCheck]=='\t'){
			scrollText[charCheck] = ' ';
		}
		charCheck++;
	}

	/* SCROLL THE TEXT */
	do {
		scroller(scrollText);
	} while(infiniteFlag);//If '-l' was set, keep going
	return 0;
}

/*****************************************************************************/
/* Function: scroller                                                        */
/* Purpose: takes an input string, uses curses to scroll it across the center*/
/*          of the screen                                                    */
/* Parameters:                                                               */
/*		char *inString    the string which is to be scrolled         */
/* Returns:     int               standard 0 return for successful completion*/
/*****************************************************************************/
int scroller(char *inString){
/* FUNCTION VARS */
	int row;//cursor row and column
        int col;
        int curChar = 1;//the number of characters to display, is equal to
                        //screen width - current cursor column
        int strAdvance = 1;//the number of character to advance the string by
                           //once the screen is full. Is also the character
                           //position of the character displayed on the left
                           //edge of the screen
        int endStringFlag = 1;//while loop flag

	initscr();//start curses
	curs_set(0);//get rid of the cursor
	clear();//make sure things are fresh
	getmaxyx(stdscr, row, col);//get our overall size
	row = row/2;//set the middle row

	move(row, col);//move to our starting position

	while(endStringFlag){//until we see '\0'

		if(col > 0){//if we have not filled the screen yet
			move(row, --col);//keep moving left
			addnstr(inString, curChar++);//add the number of chars
                        //it's possible to see at the current column
			refresh();//display the new string

		} else {//once we fill the screen

			move(row, col);//keep moving to the middle left edge
                        //and add the max number of characters, advancing
                        //through the string now, rather than the screen
			addnstr(inString + (strAdvance++), curChar-1);
			refresh();//display the new string
		}
		if(inString[strAdvance] == '\0'){//once the end char is the
                                                //only one displayed
			endStringFlag = 0;//break the loop
		}
		usleep(MICROTOMILLI * tickInterval);//time interval
		clear();//clear the screen between writes!!!
	}
	endwin();//stop curses
	return 0;
}
