/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 3                                                  */
/* Modifies file access times, creates new files if necessary                */
/*****************************************************************************/

/* INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* GLOBAL VARS */
int timeFlag = 0;//input flag for custom time
int accessFlag = 0;//input flag for "only change access time"
int modFlag = 0;//input flag for "only change mod time"
int noCreateFlag = 0;//input flag for do not create nonexistant files
char timeString[16];//holds custom times. long enough for longest format
struct tm customTime;//will hold the info from the timeString[16]

/* FUNCTION DECLARATIONS */
int fileHandle(char *fileName);
struct utimbuf makeTime(char *fileName);
struct tm timeFormat(char * input);

/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: grabs arguments, checks flags, handles program flow              */
/* Parameters:                                                               */
/*   int argc           used to count options and filespassed to the program */
/*   char **argv        contains those options and filenames                 */
/* Returns:  int              standard 0 return for successful completion    */
/*****************************************************************************/
int main(int argc, char *argv[]){
	int fileCount = 0;//counts arguments that are files, not options
	char *fileNames[argc];//max files is all of the arguments 
	//well minus the first calling argument, but what's a char pointer
	//between friends?
	int argCount = 1;//opposite argc, counts up the number of arguments
	while(argCount < argc){//for each argument besides the first
		if(!strcmp(argv[argCount], "-t")){//if the arg is a flag
			strcpy(timeString, argv[++argCount]);
			//the next arg MUST be the timeString!!!
			timeFlag = 1;//set the flag
		} else if(!strcmp(argv[argCount], "-a")){
			accessFlag = 1;//similarly...
		} else if(!strcmp(argv[argCount], "-m")){
			modFlag = 1;
		} else if(!strcmp(argv[argCount], "-c")){
			noCreateFlag = 1;
		} else {//otherwise it's a file to be touched
			fileNames[fileCount++] = argv[argCount];
			//add it to the list...
		}
		argCount++;//prevents infinite loops ;P
	}
	if(accessFlag && modFlag){//setting both m and a is a contradiction...
           printf("Options \"-m\" and \"-a\" cannot be used simultaneously\n");
		return 1;
	}
	if(timeFlag){//if we have a time String
		customTime = timeFormat(timeString);//generate some unix time
	}	
	for(fileCount;fileCount > 0;fileCount--){//now we can do the real work
		fileHandle(fileNames[fileCount-1]);
		//touch each file!
	}
	return 0;//Success!
}

/*****************************************************************************/
/* Function: fileHandle                                                      */
/* Purpose: given a filename, it checks existence, creates it if needed      */
/*          then calls a method to grabs times for it, then sets those times */
/* Parameters:                                                               */
/*   char *fileName     the name of the file to be touched                   */
/* Returns:  int              standard 0 return for successful completion    */
/*****************************************************************************/
int fileHandle(char *fileName){
	if(!(access(fileName, F_OK ) != -1)){//if file does not exist
		if(noCreateFlag){//unless the user says no new files... 
			return 0;
		}
		creat(fileName, S_IRWXU);//make that file! 
				//(with reasonable permissions!)
	}
	struct utimbuf thisTime = makeTime(fileName);//get the requested times
	utime(fileName, &thisTime);//set this files times!
	return 0;//return happy ^_^
}

/*****************************************************************************/
/* Function: makeTime                                                        */
/* Purpose: given a filename, it checks the option flags and creates times   */
/*          appropriately. Finally, it returns that time up to fileHandle    */
/* Parameters:                                                               */
/*   char *fileName           the name of the file to be given times         */
/* Returns:  struct utimbuf   the unix standard mod and access time struct   */
/*****************************************************************************/
struct utimbuf makeTime(char *fileName){
	struct utimbuf returnTime;//struct to store our final time info
	struct stat fileStats;//struct which has the file's CURRENT time info
	stat(fileName, &fileStats);//get that current info!
	returnTime.actime = time(0);//start by setting current time
	returnTime.modtime = time(0);//as the return values
	if(timeFlag){//if they have their own time in mind...
		returnTime.actime = mktime(&customTime);//give it to 'em!
		returnTime.modtime = mktime(&customTime); 
	}

	if(accessFlag){//if they want access time changed only
		returnTime.modtime = fileStats.st_mtime;
		//set our return mod time to the current mod time
	} 
	if(modFlag){//if they want mod time changed only
		returnTime.actime = fileStats.st_atime;
		//set our return access time to the current access time
	}

	return returnTime;//send it back up!
}
 
/*****************************************************************************/
/* Function: timeFormat                                                      */
/* Purpose: given a timeString (as specified by the assignment), this creates*/
/*          the tm object to be converted to unix epoch time in makeTime     */
/* Parameters:                                                               */
/*   char *input              the time string containing date and time info  */
/* Returns:  struct tm        the unix standard date and time struct         */
/*****************************************************************************/
struct tm timeFormat(char * input){
	struct tm returnTime;//struct to hold our formatted time info
	int length = strlen(input);//length tells us which kind of time string
	                           //they passed in
	int offset = 0;//and what offset to use based on year format
	if(length == 8 || length == 11){
	//strings of length 8 or 11 have no year in them, 
	//and the difference is based off having seconds or not
		returnTime.tm_year = 115;//unix years are offset from 1900 -_-
		offset = 0;//no year means no offset
	} else if(length == 10 || length == 13){
	//length 10 or 13 implies the last two digits of a year
		returnTime.tm_year = (input[0]-'0' <= 1) ?//test
		100 + 10*(input[0]-'0') + input[1]-'0': //if true
		10*(input[0]-'0') + input[1]-'0';//if false
		//we check if the year could have occured within the current
		//century. If so, set it as so. Otherwise, do not add 100
		//to leave it in the 1900's
		offset = 2;//set the offset for the rest of the string
		//to account for the two year digits
	} else if(length == 12 || length == 15){
		//length 12 or 15 implies a full four digit year
		returnTime.tm_year = 1000*(input[0]-'0') + 
				     100*(input[1]-'0') + 
				     10*(input[2]-'0') + 
				     input[3]-'0'-1900;
		//format the digits of the year properly then subtract 1900
		//yay for unix years...
		offset = 4;//full four digit year offset
	}
	//month, day of the month, hour, and minute are identical REGARDLESS
	//of format passed outside of their offset based on (lack of) year
	returnTime.tm_mon = (10*(input[offset]-'0') + input[offset+1]-'0')-1;
	returnTime.tm_mday = 10*(input[offset+2]-'0') + input[offset+3]-'0';
	returnTime.tm_hour = (10*(input[offset+4]-'0') + input[offset+5]-'0'); 
	returnTime.tm_min = (10*(input[offset+6]-'0') + input[offset+7]-'0');

	if(strchr(input, '.') == NULL){//If there is no decimal
		returnTime.tm_sec = 0;//they passed no seconds. Set it to 0
	} else {//otherwise
	returnTime.tm_sec = (10*(input[offset+9]-'0') + input[offset+10]-'0');
		//seconds will be the last thing in the string
	}
	return returnTime;//send it back up
}
