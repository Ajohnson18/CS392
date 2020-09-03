/*******************************************************************************
 * Name        : pfinder.h
 * Author      : Alex Johnson
 * Date        : 3/5/20
 * Description : Pfinder h file.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#ifndef PFINDER_H_
#define PFINDER_H_
#include <sys/stat.h>

/* Function prototypes */
int findPerms(char* dir, char* perms);
void recDir(char* basePath, char* perm);
char* permission_string(struct stat *statbuf);

#endif