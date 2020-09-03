#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define BUFSIZE 128

int main() {
	struct dirent *de;

	DIR *dr = opendir(".");

	if(dr == NULL) {
		printf("Error: Cannot open current directory.\n");
		exit(EXIT_FAILURE);
	}

	char buf[PATH_MAX]; 
	char timebuf[BUFSIZE];
	struct stat b;

	while ((de = readdir(dr)) != NULL) {
		char *res = realpath(de->d_name, buf);
		if(!res) {
			fprintf(stderr, "Error: realpath() failed. %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if(!stat(buf, &b)) {
			strftime(timebuf, BUFSIZE, "%b %d %Y %H:%M", localtime(&b.st_mtime));
			printf("%s [%s]\n", buf, timebuf);
		} else {
			printf("%s\n", buf);
		}
		
	}

	closedir(dr);
	exit(EXIT_SUCCESS);
}