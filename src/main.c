#include<stdio.h>
#include "deamon.h"
#include<string.h>
#include "files.h"

static void usage(const char* pname) {
	printf("Usage: %s <source absolute path directory> <target absolute path directory>\n",pname);
	printf("       [-p file size grouping value ] [-t sleeptime value ] [-R synchronization recursive(no value)]\n");
	printf("Example: %s /directory1 /directory2 -R -t 300s -p 100k\n",pname);
}

static size_t getBytesSize(const char* arg) {

	switch(arg[strlen(arg)-1]) {
		case 'b':  return atoi(arg); break;
		case 'k':  return atoi(arg)*1024; break;
		case 'm':  return atoi(arg)*1024*1024; break;
		default:
		return -1;
	}
}

static size_t getSecondsTime(const char* arg) {

	switch(arg[strlen(arg)-1]) {
		case 's':  return atoi(arg); break;
		case 'm':  return atoi(arg)*60; break;
		case 'h':  return atoi(arg)*60*60; break;
		default:
		return -1;
	}
}

/////////////////////////////////////////////
SyncDeamon Deamon; //deamon global variable

static void sigterm_handler(int signum) {
	if(Deamon.syncflag == 0) exit(signum);
	Deamon.running = 0;
}
static void sigusr1_handler(int signum) { signum = signum; }


int main(int argc,char* argv[]) {


		initDeamon(&Deamon);
		signal(SIGTERM,sigterm_handler);
		signal(SIGUSR1,sigusr1_handler);

		if(argc < 3) {
			printf("Error. Incorrect number of parameters\n");
			usage(argv[0]);
			exit(1);
		}


		for(int i=1; i<argc; ++i) {
			switch(argv[i][0]) {
				case '-':
					switch(argv[i][1]) {
						case 'R':
							Deamon.R = 1; //setting Deamon structure
						break;

						case 'p':
						if(i<(argc-1)) {
							int ret = getBytesSize(argv[i+1]);
							if(ret == -1) { printf("\"%s\": bad format\n",argv[i+1]); usage(argv[0]); exit(1); }
							else { Deamon.p = ret; ++i;} //setting Deamon structure
						} else {
							printf("Error. Missing arguments\n"); usage(argv[0]); exit(1);
						}

						break;

						case 't':
						if(i<(argc-1)) {
							int ret = getSecondsTime(argv[i+1]);
							if(ret == -1) { printf("\"%s\": bad format\n",argv[i+1]); usage(argv[0]); exit(1); }
							else { Deamon.t = ret; ++i;} //setting Deamon structure
						} else {
							printf("Error. Missing arguments\n"); usage(argv[0]); exit(1);
						}

						break;

						default:
						printf("\"-%c\": unknown switch\n",argv[i][1]); usage(argv[0]); exit(1);

					}
				break;

				case '/':
					if(!checkFolderExist(argv[i])) {
						printf("\"%s\": directory does not exist\n",argv[i]); usage(argv[0]); exit(1);
					} else {
						if(i<(argc-1)) {
							if(!checkFolderExist(argv[i+1])) {
								printf("\"%s\": directory does not exist\n",argv[i+1]); usage(argv[0]); exit(1);
							} else { //setting Deamon structure
									strcpy(Deamon.sourcePath,argv[i]);
									strcpy(Deamon.targetPath,argv[i+1]);
									++i;
							}
						} else {
							printf("Error. Missing arguments\n"); usage(argv[0]); exit(1);
						}
					}
				break;

				default:
				printf("\"%s\": unknown parameter\n",argv[i]); usage(argv[0]); exit(1);

			}
		}

		ShowDeamon(&Deamon);

		if(runDeamon(&Deamon) == -1) {
			printf("Error.Filed to start the Deamon\n");
			exit(1);
		}



  return 0;
}
