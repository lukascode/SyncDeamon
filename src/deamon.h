#ifndef DEAMON_H
#define DEAMON_H

#include "includes.h"
#include <time.h>
#include <string.h>
#include "files.h"

#define pSIZE 1024
#define PathLength 4095




typedef struct SyncDeamon {
	char sourcePath[PathLength];
  char targetPath[PathLength];
	int R; //synchronize recursively [default = 0]
	long p; //grouping value (mmap,read) [defaul = 10m]
	time_t t; //time for sleep [default = 5 minutes]
	int running; // running flag
	int syncflag; //flag synchronization
} SyncDeamon;


void initDeamon(SyncDeamon* deamon);
int runDeamon(SyncDeamon* deamon);
void ShowDeamon(SyncDeamon* deamon);





#endif //DEAMON_H ///:~
