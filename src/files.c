#include "files.h"
#include <utime.h>


#define fileNameLength 256

ssize_t getFileSize(const char* path) {//+
	struct stat buf;
	int ret = stat(path,&buf);
	if(!ret) {
		return buf.st_size;
	} else return ret;
}


int checkFolderExist(const char* path) {//+
	struct stat buf;
	int ret = lstat(path,&buf); if(ret == -1) return 0;
	if (S_ISDIR(buf.st_mode)) return 1;
	return 0;
}

int checkFileExist(const char* path) {//+
	struct stat buf;
	int ret = lstat(path,&buf); if(ret == -1) return 0;
	if (S_ISREG(buf.st_mode)) return 1;
	return 0;
}

int checkLinkExist(const char* path) {
	struct stat buf;
	int ret = lstat(path,&buf); if(ret == -1) return 0;
	if(S_ISLNK(buf.st_mode)) return 1;
	return 0;
}


int DateCpy(const char* file1,const char* file2) { //copy st_mtime from file1 to file2  //+
  struct stat buff1;
  struct stat buff2;

  int ret;
  ret = stat(file1,&buff1); if(ret == -1) return ret;
  ret = stat(file2,&buff2); if(ret == -1) return ret;

  struct utimbuf puttime;
  puttime.modtime = buff1.st_mtime;
  utime(file2, &puttime);

  return 0;

}

 int ModDateEqual(const char* file1,const char* file2) { //+
   struct stat buff1;
   struct stat buff2;

   int ret;
   ret = stat(file1,&buff1); if(ret == -1) return ret;
   ret = stat(file2,&buff2); if(ret == -1) return ret;

   if((long)buff1.st_mtime != (long)buff2.st_mtime) return 0;
   return 1;
}
