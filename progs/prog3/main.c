#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int timeFlag = 0;
int accessFlag = 0;
int modFlag = 0;
int noCreateFlag = 0;
char timeString[16];
struct tm customTime;

int fileHandle(char *fileName);
struct utimbuf makeTime(char *fileName);
struct tm timeFormat(char * input);

int main(int argc, char *argv[]){
	int fileCount = 0;
	char *fileNames[argc];
	int argCount = 1;
	while(argCount < argc){
		if(!strcmp(argv[argCount], "-t")){
			strcpy(timeString, argv[++argCount]);
			timeFlag = 1;
		} else if(!strcmp(argv[argCount], "-a")){
			accessFlag = 1;
		} else if(!strcmp(argv[argCount], "-m")){
			modFlag = 1;
		} else if(!strcmp(argv[argCount], "-c")){
			noCreateFlag = 1;
		} else {
			fileNames[fileCount++] = argv[argCount];
		}
		argCount++;
	}
	if(accessFlag && modFlag){
		printf("Options \"-m\" and \"-a\" cannot be used simultaneously\n");
		return 1;
	}
	if(timeFlag){
		customTime = timeFormat(timeString);
	}	
	for(fileCount;fileCount > 0;fileCount--){
		fileHandle(fileNames[fileCount-1]);	
	}
	return 0;
}

int fileHandle(char *fileName){
	if(!(access(fileName, F_OK ) != -1)){
		if(noCreateFlag){
			return 0;
		}
		creat(fileName, S_IRWXU);
	}
	struct utimbuf thisTime = makeTime(fileName);
	utime(fileName, &thisTime);
}

struct utimbuf makeTime(char *fileName){
	struct utimbuf returnTime;
	struct stat fileStats;
	stat(fileName, &fileStats);
	returnTime.actime = time(0);
	returnTime.modtime = time(0);
	if(timeFlag){
		returnTime.actime = mktime(&customTime);
		returnTime.modtime = mktime(&customTime); 
	}

	if(accessFlag){
		returnTime.modtime = fileStats.st_mtime;
	} 
	if(modFlag){
		returnTime.actime = fileStats.st_atime;
	}

	return returnTime;
}

struct tm timeFormat(char * input){
	struct tm returnTime;
	int length = strlen(input);
	int offset = 0;
	if(length == 8 || length == 11){
		returnTime.tm_year = 115;
		offset = 0;
	} else if(length == 10 || length == 13){
		returnTime.tm_year = (input[0]-'0' <= 1) ? 100 + 10*(input[0]-'0') + input[1]-'0': 10*(input[0]-'0') + input[1]-'0';
		offset = 2;
	} else if(length == 12 || length == 15){
		returnTime.tm_year = 1000*(input[0]-'0') + 100*(input[1]-'0') + 10*(input[2]-'0') + input[3]-'0'-1900;
		offset = 4;
	}
	returnTime.tm_mon = (10*(input[offset]-'0') + input[offset+1]-'0')-1;
	returnTime.tm_mday = 10*(input[offset+2]-'0') + input[offset+3]-'0';
	returnTime.tm_hour = (10*(input[offset+4]-'0') + input[offset+5]-'0')+1; 
	returnTime.tm_min = (10*(input[offset+6]-'0') + input[offset+7]-'0');
	if(strchr(input, '.') == NULL){
		returnTime.tm_sec = 0;
	} else {
		returnTime.tm_sec = (10*(input[offset+9]-'0') + input[offset+10]-'0');
	}
	return returnTime;
}
