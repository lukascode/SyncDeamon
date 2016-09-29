#ifndef FILES_H
#define FILES_H

#include "includes.h"

ssize_t getFileSize(const char* path);
int checkFolderExist(const char* path);
int checkFileExist(const char* path);
int checkLinkExist(const char* path);
int ModDateEqual(const char* file1,const char* file2);
int DateCpy(const char* file1,const char* file2);




#endif //FILES_H ///:~
