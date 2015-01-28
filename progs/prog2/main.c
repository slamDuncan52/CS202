/*****************************************************************************/
/* Mitch Duncan                                                              */
/* Login ID: dunc0474                                                        */
/* CS-202, Winter 2015                                                       */
/* Programming Assignment 2                                                  */
/* Prints a list of user logins, with time and date stamp                    */
/*****************************************************************************/

/* INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

/* FUNCTION DECLARATIONS */
int isNumeric (const char * s);
void checkOptions(int argc, char *argv[]);
int showInfo(struct utmp *record, struct utmp logoutArray[]);
void testIfLogin(struct utmp *record, struct utmp logoutArray[]);

/* GLOBAL VARIABLES */
int current = 0;//count of entries handled, based on number printed
int numRecords = 0;//the total number of entries in the wtmp file
int logoutCount = 0;//the total number of logout entries seen
time_t lastBoot = 0;//the time of the previous system boot
int count = -1;//the user-set count option
char *namePoint;//the user-set name option
int countFlag = 1;//flag for count option. 0 IS ENABLED, 1 IS DISABLED
int nameFlag = 1;//flag for name option. 0 IS ENABLED, 1 is DISABLED
struct utmp swapRecord;//a dummy utmp struct used as swap space
/*****************************************************************************/
/* Function: main                                                            */
/* Purpose: takes care of wtmp file handling, and program flow               */
/* Parameters:                                                               */
/*   int argc              used to count options passed to the program       */
/*   char **argv           contains those options (if any)                   */
/* Returns:  int              standard 0 return for successful completion    */
/*****************************************************************************/
int main(int argc, char *argv[]){

	/* open utmp */
	struct utmp current_record;//the current wtmp entry being handled
	int utmpFile;//FD of the system wtmp file
	int reclen = sizeof(current_record);//size var of utmp structs
	time_t beginTime;//time of earliest entry in the wtmp file

	checkOptions(argc, argv);//Check what options, if any, the user passed
				//in. The global flags are set in here

/* If the file cannot be opened, bail out*/
	if((utmpFile = open(WTMP_FILE, O_RDONLY)) == -1){
		perror(WTMP_FILE);
		exit(1);
	}
	/* First, count all of the records */
	while((read(utmpFile, &current_record, reclen) == reclen)){
		numRecords++;
	}
/* Create our array of logouts, with unncessary oversize */
/* Memory is cheap, right? */
	struct utmp logoutArray[numRecords];//holds all logout wtmp entries
	
	close(utmpFile);//this is hacky. Reset the file by closing
	utmpFile = open(WTMP_FILE, O_RDONLY);//and re-opening

	for(numRecords;numRecords > 0 //For all the records
     && (current < count || countFlag);numRecords--){//less than count, if set
		/* We seek and read from the end records first */
		/* because wtmp starts at the oldest records */
		/* but we want to print the newest first */
		lseek(utmpFile,(reclen * (numRecords-1)),SEEK_SET);
		read(utmpFile, &current_record, reclen);
		//This is set every loop. on the last one
		//it will hold the time of the oldest entry
		beginTime = current_record.ut_time;
		//here is where we actually print info! :D
		showInfo(&current_record, logoutArray);
	}
	printf("\n");//print out the time of the oldest entry
	printf("wtmp begins %s", ctime(&beginTime));
	
	/* close utmp */
	close(utmpFile);

	return 0;
}

/*****************************************************************************/
/* Function: isNumeric                                                       */
/* Purpose: check if a given string is entirely numbers                      */
/* Parameters:                                                               */
/*   char * inString          the string to be checked                       */
/* Returns:  int              1 on numeric, 0 on failure                     */
/*****************************************************************************/
int isNumeric (const char * inString)
{//This function is probably overkill, but hey, do it right or go home
//Make sure it's a real string, with contiguous digits
	if (inString == NULL || *inString == '\0' || isspace(*inString))
		return 0;
	char * returnString;//if the string all numbers, should hold '\0'
	//try to convert the string to a long
	strtol (inString, &returnString, 10);
	return *returnString == '\0';//return result
}

/*****************************************************************************/
/* Function: checkOptions                                                    */
/* Purpose: checks input options, regardless of order                        */
/* Parameters:                                                               */
/*   int argc              used to count options passed to the program       */
/*   char *argv[]          contains those options (if any)                   */
/*   int nameFlag          flag to set if name was invoked                   */
/*   int countFlag         flag to set if count was invoked                  */
/* Returns:  int              standard 0 return for successful completion    */
/*****************************************************************************/
void checkOptions(int argc, char *argv[]){
	if(argc == 1){//no arguments
		namePoint = "";//so set nothing
		count = -1;
	} else if(argc == 2){//one arg, MUST BE NAME
		nameFlag = 0;//SET NAME FLAG
		namePoint = argv[1];//set name string
		count = -1;//no count
	} else if(argc == 3){//two args, MUST BE COUNT
		countFlag = 0;//SET COUNT FLAG
		namePoint = "";//no name
		if(isNumeric(argv[1])){//check where the number actually is
			count = (int)strtol(argv[1], NULL, 10);//set the count
		} else {
			count = (int)strtol(argv[2], NULL, 10);
		}
	} else if(argc == 4){//three args, MUST BE BOTH. UGH.
		nameFlag = 0;//SET BOTH FLAGS
		countFlag = 0;
/* Format is, check if the count number is at this argv, set count.
Then check which argv contains -n, other must be name */
		if(isNumeric(argv[1])){
			count = (int)strtol(argv[1], NULL, 10);
			if(strcmp(argv[3],"-n")){
				namePoint = argv[3];
			} else {
				namePoint = argv[2];
			}
		} else if(isNumeric(argv[2])) {
			count = (int)strtol(argv[2], NULL, 10);
			if(strcmp(argv[3],"-n")){
				namePoint = argv[3];

			} else {
				namePoint = argv[1];
			}
		} else {
			count = (int)strtol(argv[3], NULL, 10);
			if(strcmp(argv[2],"-n")){
				namePoint = argv[2];
			} else {
				namePoint = argv[1];
			}
		}
	}
}

/*****************************************************************************/
/* Function: showInfo                                                        */
/* Purpose: given a wtmp entry, prints the appropriate data for it           */
/* Parameters:                                                               */
/*  struct utmp *record        The entry to be handled. Format as in utmp.h  */
/*  struct utmp logoutArray[]  contains wtmp entries which are logouts       */
/* Returns:  int              standard 0 return for successful completion    */
/*****************************************************************************/
int showInfo(struct utmp *record, struct utmp logoutArray[]){
/* We only care about boots, user logins, and logouts
   boots and logins are handled together, logouts seperately */
	if(record->ut_type == BOOT_TIME || record->ut_type == USER_PROCESS){
	/*If name is set, only proceed if name matches, otherwise, just go*/
		if((nameFlag || !strcmp(record->ut_user, namePoint))){
			printf("%-8.8s ", record->ut_user);//Print the username
		      if(record->ut_line[0]=='~' && record->ut_line[1]=='\0'){
				printf("system boot  ");//boots have no tty,
					//but they should read system boot
			} 
			else {//logins have a tty, so print it
				printf("%-12.12s ", record->ut_line);
			}
			printf("%-16.16s ", record->ut_host);//print the host
			time_t curTime = record->ut_time;//use ctime format
			printf("%-16.16s ", ctime(&curTime));//print time
			testIfLogin(record, logoutArray);//check if logout
							//for this login
			printf("\n");
			current++;//update number of entries processed
		}
	}
	if(record->ut_type == DEAD_PROCESS){
/* We found a logout process! */
		swapRecord = *record;//use that swap space
		logoutArray[logoutCount] = swapRecord;//swap into logout array
		logoutCount++;//increment logouts seen
	}
}

/*****************************************************************************/
/* Function: testIfLogin                                                     */
/* Purpose: For a wtmp entry, tests whether it is a login or logout entry    */
/*          then either prints appropriate info, or adds to logout Array     */
/*          If boot entry, handle special with own subroutine                */
/* Parameters:                                                               */
/*  struct utmp *record        contains wtmp entry to be tested              */
/*  struct utmp logoutArray[]  contains wtmp entries which are logouts       */
/* Returns:  void                                                            */
/*****************************************************************************/
void testIfLogin(struct utmp *record, struct utmp logoutArray[]){
	int count;//loop counter for logout comparison
	long overallTime;//stores difference between login and logout times
	/*Boots are handled special, their time differences are measure from
		the time of the last boot */
	if(record->ut_type == BOOT_TIME){
		time_t thisBoot = record->ut_time;
		if(lastBoot == 0){//if the latest boot in utmp
			thisBoot = time(0);//the boot is going until now
			//measure the time difference
                      overallTime = (long)difftime(thisBoot, record->ut_time);
			printf("- %4.5s", ctime(&thisBoot)+11);//print now time
		} else {//otherwise, there was a previous boot
			//measure the time difference
			overallTime = (long)difftime(lastBoot,thisBoot);
			printf("- %4.5s", ctime(&lastBoot)+11);//print last
		}
		//print the time difference
             printf("  (%.2ld:%-.2ld)",overallTime/3600, (overallTime/60)%60);
		lastBoot = record->ut_time;//set the lastBoot to this one
		return;
	}
	if(!strcmp(record->ut_user, "root")){//invoked on sudo commands
		time_t thisTime = record->ut_time;//just print no time elapse
		printf("- %-4.5s  (00:00)", ctime(&thisTime)+11);
		return;
	}
/*SO. we have a login entry. We know how many logouts we have. Compare each
logout to our current login. Starting from the last (oldest) logout seen 
since old logouts happen before new ones. Sounds trivial, but it matters */
	for(count = logoutCount-1; count >= 0;count--){
// Compare on ut_user (username) and ut_line (tty name)
		if(!strcmp(record->ut_user, logoutArray[count].ut_user) 
		&& !(strcmp(record->ut_line, logoutArray[count].ut_line))){
// When a match is found, compute the overall logged in time
			time_t logoutTime = logoutArray[count].ut_time;
			time_t loginTime = record->ut_time;
			overallTime = (long)difftime(logoutTime, loginTime);
//Print out the logout time, and overall time logged in
			printf("- %-4.5s", ctime(&logoutTime)+11);
            printf("  (%.2ld:%-.2ld)", overallTime/3600, (overallTime/60)%60);
			return;
		}
	}
//If a match was never found, we are STILL logged in!
	printf("  still logged in");
	return;
}
