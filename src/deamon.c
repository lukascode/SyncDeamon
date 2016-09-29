#include "deamon.h"


static void GeneratePath(char* path,const char* tail);
static int copyFile_mmap(const char* source,const char* target);
static int copyFile_read(const char* source,const char* target);
static int isDefaultPointer(const char* tail);
static int deleteFolder(const char* path);
static void deleteReduntantsFromTarget(const char* source, const char* target);
static void Synchronize(const char* source, const char* target,SyncDeamon* deamon);

static void checkSTpaths(SyncDeamon* deamon);

static void copyFilewithErrors_read(const char* spath,const char* tpath);
static void copyFilewithErrors_mmap(const char* spath,const char* tpath);
static int createDirectorywithErrors(const char* path,mode_t mode);
static void deleteFilewithErrors(const char* path);
static void deleteFolderwithErrors(const char* path);
static int deleteDirwithErrors(const char* path);



int runDeamon(SyncDeamon* deamon) {
	pid_t pid;
	pid = fork();
	if(pid == -1)
		return -1;
	else if(pid != 0)
		exit(EXIT_SUCCESS);

		//creating session
		if(setsid() == -1)
			return -1;

		if(chdir("/") == -1)
			return -1;

			close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

			deamon->running = 1;

			//open logs file descriptor
			openlog(NULL,LOG_PID,LOG_USER);


			//main loop
			while(deamon->running) {


					sleep(deamon->t);
					syslog(LOG_INFO,"DEMON WOKE UP\n");
					deamon->syncflag = 1;
					checkSTpaths(deamon); Synchronize(deamon->sourcePath,deamon->targetPath,deamon);
					deamon->syncflag = 0;
					syslog(LOG_INFO,"TIME TO SLEEP\n");

				}

				//close logs file descriptor
				closelog();

				return 0;
}

static void checkSTpaths(SyncDeamon* deamon) {
	if(!checkFolderExist(deamon->sourcePath) ||
	!checkFolderExist(deamon->targetPath)) {
		syslog(LOG_INFO,"Error. Incorrect source or target path\n");
		syslog(LOG_INFO,"Now the demon exits\n");
		exit(1);
	}
}


void initDeamon(SyncDeamon* deamon) {//default values
	deamon->R = 0;
	deamon->p = 10240; //10kB
	deamon->t = (time_t)300; //5minutes
	deamon->running = 0;
	deamon->syncflag = 0;
}

void ShowDeamon(SyncDeamon* deamon) {//+
	printf("Deamon parameters:\n");
	printf("SyncDeamon { \n");
	printf("  sourcePath: %s\n",deamon->sourcePath);
	printf("  targetPath: %s\n",deamon->targetPath);
	printf("  R: %d \n",deamon->R);
	printf("  p: %d bytes\n",(int)deamon->p);
	printf("  t: %d seconds\n",(int)deamon->t);
	printf("}\n");
}


//----------------------------------------------------------------------------
/*errors handle*/
static void copyFilewithErrors_read(const char* spath,const char* tpath) {
	if(copyFile_read(spath,tpath)==-1) {
		syslog(LOG_INFO,"CopyFile_read failed from \"%s\" to \"%s\": %s",spath,tpath,strerror(errno));
	} else {
		syslog(LOG_INFO,"CopyFile_read succesfull from \"%s\" to \"%s\"",spath,tpath);
		DateCpy(spath,tpath);
	}
}

static void copyFilewithErrors_mmap(const char* spath,const char* tpath) {
	if(copyFile_mmap(spath,tpath)==-1) {
		syslog(LOG_INFO,"CopyFile_mmap failed from \"%s\" to \"%s\": %s",spath,tpath,strerror(errno));
	} else {
		syslog(LOG_INFO,"CopyFile_mmap succesfull from \"%s\" to \"%s\"\n",spath,tpath);
		DateCpy(spath,tpath);
	}
}

static int createDirectorywithErrors(const char* path,mode_t mode) {
	if(mkdir(path,mode) == -1) {
	syslog(LOG_INFO,"Creating directory \"%s\" failed:%s",path,strerror(errno));
	return -1;
	}
	syslog(LOG_INFO,"Creating directory \"%s\" succesfull\n",path);
	return 1;
}

static void deleteFilewithErrors(const char* path) {
	if(unlink(path)==-1)
	syslog(LOG_INFO,"Deleting file \"%s\" failed:%s",path,strerror(errno));
	else
	syslog(LOG_INFO,"Deleting file \"%s\" succesfull\n",path);
}

static void deleteFolderwithErrors(const char* path) {
	int ret = deleteFolder(path);
	if(ret == -1)
	syslog(LOG_INFO,"deleteFolder \"%s\" failed:%s",path,strerror(errno));
	else if(ret == 0)
	syslog(LOG_INFO,"deleteFolder \"%s\" : not fully deleted\n",path);
	else
	syslog(LOG_INFO,"deleteFolder \"%s\" succesfull\n",path);
}

static int deleteDirwithErrors(const char* path) {
	if(rmdir(path)==-1) {
	syslog(LOG_INFO,"deleteDirectory \"%s\" failed: %s",path,strerror(errno));
	return -1;
	}
	syslog(LOG_INFO,"deleteDirectory \"%s\" succesfull\n",path);
	return 1;
}
//--------------------------------------------------------------------------------------------

static void Synchronize(const char* source, const char* target,SyncDeamon* deamon) {//++
	DIR* dir = opendir(source); if(!dir) return;
	struct dirent* entry;
	while((entry = readdir(dir))!=NULL) {

		 if(isDefaultPointer(entry->d_name)) continue; //ignore folders: /.. and /.


		 char* spath = (char*)malloc(PathLength*sizeof(char)); strcpy(spath,source);
		 char* tpath = (char*)malloc(PathLength*sizeof(char)); strcpy(tpath,target);

		GeneratePath(spath,entry->d_name);
		GeneratePath(tpath,entry->d_name);

		if(checkFileExist(spath)) { //regular file and not symbolic link file
			if(checkFileExist(tpath)) {
					if(!ModDateEqual(spath,tpath)) {
						if(getFileSize(spath) <= deamon->p) { //deamon->p  --> mmap() or read()
								copyFilewithErrors_read(spath,tpath);
						} else {
								copyFilewithErrors_mmap(spath,tpath);
						}
					}
			} else {
				if(getFileSize(spath) <= deamon->p) { //deamon->p  --> mmap() or read()
					copyFilewithErrors_read(spath,tpath);
				} else {
					copyFilewithErrors_mmap(spath,tpath);
				}
			}
		} else if(checkFolderExist(spath) && deamon->R) { //folder and Deamon is set to recursively mode
				if(checkFolderExist(tpath)) {
						Synchronize(spath,tpath,deamon);
				} else {

					if(createDirectorywithErrors(tpath,0777))
							Synchronize(spath,tpath,deamon);
				}
		}

		free(spath);
		free(tpath);

	}
	closedir(dir);
	deleteReduntantsFromTarget(source,target);
}

//delete files from target which doesn't exist in source
static void deleteReduntantsFromTarget(const char* source, const char* target) {//+
	DIR* dir = opendir(target); if(!dir) return;
	struct dirent* entry;

	while((entry = readdir(dir))!=NULL) {

		if(isDefaultPointer(entry->d_name)) continue; //ignore folders: /.. and /.

		char* spath = (char*)malloc(PathLength*sizeof(char)); strcpy(spath,source);
		char* tpath = (char*)malloc(PathLength*sizeof(char)); strcpy(tpath,target);

	 GeneratePath(spath,entry->d_name);
	 GeneratePath(tpath,entry->d_name);

	 //deleting file when it doesn't exist in source or it's symbolic link file
	 if((checkFileExist(tpath) && !checkFileExist(spath)) || checkLinkExist(tpath)) {
		 deleteFilewithErrors(tpath);
	 } else if(checkFolderExist(tpath) && !checkFolderExist(spath) ) {
		 deleteFolderwithErrors(tpath);
	 }

	 free(spath);
 	 free(tpath);
	}
	closedir(dir);

}

static int deleteFolder(const char* path) { //recursive //+
	DIR* dir = opendir(path); if(!dir) return -1;
	struct dirent* entry;

	while((entry = readdir(dir))!=NULL) {

		if(isDefaultPointer(entry->d_name)) continue; //ignore folders: /.. and /.

		char* tmppath = (char*)malloc(PathLength*sizeof(char)); strcpy(tmppath,path);
		GeneratePath(tmppath,entry->d_name);
		if(checkFolderExist(tmppath)) {

			deleteFolderwithErrors(tmppath);

		} else if(checkFileExist(tmppath) || checkLinkExist(tmppath)) {

			deleteFilewithErrors(tmppath);

		}
	}
	closedir(dir);
	if(deleteDirwithErrors(path)==-1) return 0;
	return 1;
}

static int copyFile_read(const char* source,const char* target) {//+
	int fs = open(source,O_RDONLY);
	if(fs == -1) return fs;
	int ft = open(target,O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,0666);
	if(ft == -1) return ft;

	char buffer[pSIZE]; ssize_t ret;
		while((ret = read(fs,buffer,pSIZE)) > 0) {
			if(ret == -1) {
				if(errno == EINTR) continue;
				return ret;
			}
			write(ft,buffer,ret);
		}

	close(fs);
	close(ft);
	return 0;

}


static int copyFile_mmap(const char* source,const char* target) {//+
	struct stat sb;
	char* p; int fs,ft;

	fs = open(source,O_RDONLY);
	if(fs == -1) return fs;
	ft = open(target,O_WRONLY | O_CREAT | O_TRUNC,0666);
	if(ft == -1) return ft;
	if(fstat(fs,&sb) == -1) return -1;
	if(!S_ISREG(sb.st_mode)) return -1;
	p = mmap(0,sb.st_size,PROT_READ,MAP_SHARED,fs,0);
	if(p == MAP_FAILED) return -1;
	close(fs);
	write(ft,p,sb.st_size);
	close(ft);
	munmap(p, sb.st_size);
	return 0;
}

/*static const char* getCurDate() {
time_t t = time(NULL);
struct tm *tm = localtime(&t);
return asctime(tm);
}*/

static void GeneratePath(char* path, const char* tail) {//+
			size_t len = strlen(path);
			if(path[len-1] == '/') {
				strcpy(path+len,tail);
			} else {
			path[len] = '/';
			strcpy(path+len+1,tail);
		}
}

static int isDefaultPointer(const char* tail) {//+
	char buf1[]={".."}; char buf2[]={"."};
	if((strcmp(tail,buf1) == 0) || (strcmp(tail,buf2)==0)) return 1;
	return 0;
	}
