#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "cgic.h"

char *correctName(char *p)
{
	char *delimPos = NULL;
	delimPos = strtok(p, "\\");
	while(NULL != (delimPos = strtok(NULL, "\\"))){
		p = delimPos;
	}
	return p;
}

int cgiMain() {
	cgiFilePtr file;
	char name[1024];
	char contentType[1024];
	char buffer[1024];
	int size;
	int got;
	char nameOnServer[1024]="/www/configs/";
	char *correctedName = NULL;
	int fd;

	//fprintf(cgiOut, "\nstart app\n");
	if (cgiFormFileName("file", name, sizeof(name)) != cgiFormSuccess) {
		fprintf(cgiOut, "\ncgiFormFileName failed:%s. \n",name);
		return -1;
	}
	//correct the name of file
	correctedName = correctName(name);
	cgiHtmlEscape(correctedName);
	cgiFormFileSize("file", &size);
	cgiFormFileContentType("file", contentType, sizeof(contentType));
	cgiHtmlEscape(contentType);
	if (cgiFormFileOpen("file", &file) != cgiFormSuccess) {
		return -1;
	}
	strcat(nameOnServer, correctedName);

	if((fd = open(nameOnServer, O_WRONLY|O_CREAT|O_TRUNC))<0){
		fprintf(cgiOut, "\nopen file failed:%s. \n",nameOnServer);
		return -1;
	}

	while (cgiFormFileRead(file, buffer, sizeof(buffer), &got) ==
		cgiFormSuccess)
	{
		//fprintf(cgiOut, "\nwrite bytes:%d. \n",got);
		if(got > 0){
			write(fd, buffer, got);
		//fprintf(cgiOut, "\n%s. \n",buffer);
		}else{
			usleep(100);
		}

	}
	
	usleep(2000);
	//fprintf(cgiOut, "\nend app\n");
	close(fd);
	cgiFormFileClose(file);

	return 0;
}





