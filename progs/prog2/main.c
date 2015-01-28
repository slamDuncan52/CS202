#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

int showInfo(struct utmp *record, char *namePoint,int count, int nameFlag, int countFlag, struct utmp logoutArray[]);
int isNumeric (const char * s);
void testIfLogin(struct utmp *record, struct utmp logoutArray[]);

int current = 0;
int numRecords = 0;
int logoutCount = 0;
time_t lastBoot = 0;
struct utmp swapRecord;

int main(int argc, char *argv[]){
	/* open utmp */
	struct utmp current_record;
	int utmpFile;
	int reclen = sizeof(current_record);
	int countFlag = 1;
	int nameFlag = 1;
	int count = -1;
	char *namePoint;
	time_t beginTime;
	count = -1;
	if(argc == 1){
		namePoint = "";
		count = -1;
	} else if(argc == 2){
		nameFlag = 0;
		namePoint = argv[1];
		count = -1;
	} else if(argc == 3){
		countFlag = 0;
		namePoint = "";
		if(isNumeric(argv[1])){
			count = (int)strtol(argv[1], NULL, 10);
		} else {
			count = (int)strtol(argv[2], NULL, 10);
		}
	} else if(argc == 4){
		nameFlag = 0;
		countFlag = 0;
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

	if((utmpFile = open(WTMP_FILE, O_RDONLY)) == -1){
		perror(WTMP_FILE);
		exit(1);
	}
	/* read record */
	while((read(utmpFile, &current_record, reclen) == reclen)){
		numRecords++;
	}

	struct utmp logoutArray[numRecords];
	close(utmpFile);
	utmpFile = open(WTMP_FILE, O_RDONLY);

	for(numRecords;numRecords > 0 && (current < count || countFlag);numRecords--){
		lseek(utmpFile,(reclen * (numRecords-1)),SEEK_SET);
		read(utmpFile, &current_record, reclen);
		beginTime = current_record.ut_time;
		showInfo(&current_record, namePoint, count, nameFlag, countFlag, logoutArray);
	}
	printf("\n");
	printf("wtmp begins %s", ctime(&beginTime));
	/* close utmp */
	close(utmpFile);

	return 0;
}

int showInfo(struct utmp *record, char *namePoint,int count, int nameFlag, int countFlag, struct utmp logoutArray[]){
	if(record->ut_type == BOOT_TIME || record->ut_type == USER_PROCESS){
		if((nameFlag || !strcmp(record->ut_user, namePoint)) && (countFlag || current < count)){
			printf("%-8.8s ", record->ut_user);
			if(record->ut_line[0] == '~' && record->ut_line[1] == '\0'){
				printf("system boot  ");
			} 
			else {
				printf("%-12.12s ", record->ut_line);
			}
			printf("%-16.16s ", record->ut_host);
			time_t curTime = record->ut_time;
			printf("%-16.16s ", ctime(&curTime));
			testIfLogin(record, logoutArray);
			printf("\n");
			current++;
		}
	}
	if(record->ut_type == DEAD_PROCESS){
		swapRecord = *record;
		logoutArray[logoutCount] = swapRecord;
		time_t curTime = logoutArray[logoutCount].ut_time;
		logoutCount++;
	}
}

void testIfLogin(struct utmp *record, struct utmp logoutArray[]){
	int count;
	long overallTime;
	if(record->ut_type == BOOT_TIME){
		time_t thisBoot = record->ut_time;
		if(lastBoot == 0){
			thisBoot = time(0);
			overallTime = (long)difftime(thisBoot, record->ut_time);
			printf("- %4.5s", ctime(&thisBoot)+11);
		} else {
			overallTime = (long)difftime(lastBoot,thisBoot);
			printf("- %4.5s", ctime(&lastBoot)+11);
		}
		printf("  (%.2ld:%-.2ld)",overallTime/3600, (overallTime/60)%60);
		lastBoot = record->ut_time;
		return;
	}
	if(!strcmp(record->ut_user, "root")){
		time_t thisTime = record->ut_time;
		printf("- %-4.5s  (00:00)", ctime(&thisTime)+11);
		return;
	}
	for(count = logoutCount-1; count >= 0;count--){
		if(!strcmp(record->ut_user, logoutArray[count].ut_user) && !(strcmp(record->ut_line, logoutArray[count].ut_line))){
			time_t logoutTime = logoutArray[count].ut_time;
			time_t loginTime = record->ut_time;
			overallTime = (long)difftime(logoutTime, loginTime);
			printf("- %-4.5s", ctime(&logoutTime)+11);
			printf("  (%.2ld:%-.2ld)", overallTime/3600, (overallTime/60)%60);  
			return;
		}
	}
	printf("  still logged in");
	return;
}

int isNumeric (const char * s)
{
	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;
	char * p;
	strtod (s, &p);
	return *p == '\0';
}
