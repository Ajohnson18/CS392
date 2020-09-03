/*******************************************************************************
 * Name        : pfinder.c
 * Author      : Alex Johnson
 * Date        : 3/5/20
 * Description : pfinder file to test permissions against files in a directory.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "pfinder.h"

int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};

char* permission_string(struct stat *statbuf) {
	char* perms_s = malloc(10 * sizeof(char));
	int permission_value;
	perms_s[9] = '\0';
	for(int i = 0; i < 9; i+=3) {
		permission_value = statbuf->st_mode;
		if(permission_value & perms[i]) {
			perms_s[i] = 'r';
		} else {
			perms_s[i] = '-';
		}
		if(permission_value & perms[i + 1]) {
			perms_s[i + 1] = 'w';
		} else {
			perms_s[i + 1] = '-';
		}
		if(permission_value & perms[i + 2]) {
			perms_s[i + 2] = 'x';
		} else {
			perms_s[i + 2] = '-';
		}
	}
	return perms_s;
}

void recDir(char* basePath, char* perm) {


	struct dirent *entry;
	char path[PATH_MAX];
	DIR *dir;

	if((dir = opendir(basePath)) == NULL) {
		fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", 
			path, strerror(errno));
		exit (0);
	}

	if(!dir) return;

	while((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		strcpy(path, basePath);
		strcat(path, "/");
		strcat(path, entry->d_name);

		struct stat statbuf;
		if (stat(path, &statbuf) < 0) {
		    fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", path,
		       	strerror(errno));
		}
		char *perms = permission_string(&statbuf);
		if(strcmp(perms, perm) == 0) printf("%s\n", path);
		free(perms);

		if(entry->d_type == DT_DIR) {
			recDir(path, perm);
		}
	}

	closedir(dir);

}

int findPerms(char* direct, char* perms) {

	char path[PATH_MAX];
	if(realpath(direct, path) == NULL) {
		fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", 
			direct, strerror(errno));
		return EXIT_FAILURE;
	}

	DIR *dir;
	if((dir = opendir(direct)) == NULL) {
		fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", 
			path, strerror(errno));
		return EXIT_FAILURE;
	}

	free(dir);

	if(strlen(perms) != 9) {
		fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", perms);
		return EXIT_FAILURE;
	}

	for(int i = 0; i < strlen(perms); i++) {
		if((i == 0 || i == 3 || i == 6) && ((perms[i] != '-') && (perms[i] != 'r'))) {
			fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", perms);
			return EXIT_FAILURE;
		} else if ((i == 1 || i == 4 || i == 7) && ((perms[i] != '-') && (perms[i] != 'w'))) {
			fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", perms);
			return EXIT_FAILURE;
		} else {
			if((i == 2 || i == 5 || i == 8) && ((perms[i] != '-') && (perms[i] != 'x'))) {
			fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", perms);
			return EXIT_FAILURE;
			}
		}
	}

	recDir(path, perms);
	return EXIT_SUCCESS;

}