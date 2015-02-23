#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>

int scroller(char *inString);

int tickInterval = 1000;

int main(int argc, char *argv[]){
	// handle args
	FILE *scrollFile;
	char *scrollText;
	int fileLen = 0;
	int argCount = 1;
	int infiniteFlag = 0;
	int once = 1;
	while(argCount < argc){
		if(!strcmp(argv[argCount], "-t")){
			tickInterval = (int)strtol(argv[++argCount], NULL, 10);
		} else if(!strcmp(argv[argCount], "-l")){
			infiniteFlag = 1;
		} else {
			scrollFile = fopen(argv[argCount], "r");
		}
		argCount++;
	}
	//read file into string
	fseek(scrollFile, 0L, SEEK_END);
	fileLen = ftell(scrollFile);
	rewind(scrollFile);
	scrollText = malloc(fileLen * (sizeof(char)));
	fread(scrollText, sizeof(char), fileLen, scrollFile);	
	// scroll the text
	while(infiniteFlag || once){
		scroller(scrollText);
		once = 0;
	}
	return 0;
}

int scroller(char *inString){
	int row, col, curChar = 1, strAdvance = 1, endStringFlag = 1;
	initscr();
	clear();
	getmaxyx(stdscr, row, col);
	row = row/2;

	move(row, col);

	while(endStringFlag){
		if(col > 0){
			move(row, --col);
			addnstr(inString, curChar);
			refresh();
			curChar++;
		} else {
			move(row, col);
			addnstr(inString + strAdvance, curChar-1);
			refresh();
			strAdvance++;
		}
		if(inString[strAdvance] == '\0'){
			endStringFlag = 0;
		}
		usleep(1000 * tickInterval);
		clear();
	}
	endwin();
}
