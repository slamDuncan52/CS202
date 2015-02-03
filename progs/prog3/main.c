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

int fileHandle(char *fileName);
struct utimbuf makeTime(char *fileName);

int main(int argc, char *argv[]){
	int fileCount = 0;
	char *fileNames[argc];
	while(argc > 1){
		if(!strcmp(argv[argc-1], "-t")){
			timeFlag = 1;
		} else if(!strcmp(argv[argc-1], "-a")){
			accessFlag = 1;
		} else if(!strcmp(argv[argc-1], "-m")){
			modFlag = 1;
		} else if(!strcmp(argv[argc-1], "-c")){
			noCreateFlag = 1;
		} else {
			fileNames[fileCount++] = argv[argc-1];
		}
		argc--;
	}
	if(accessFlag && modFlag){
		printf("Options \"-m\" and \"-a\" cannot be used simultaneously\n");
		return 1;
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
	returnTime.actime = time(0);
	returnTime.modtime = time(0);
	if(timeFlag){

	}

	if(accessFlag){

	} else if(modFlag){

	}

	return returnTime;
}
