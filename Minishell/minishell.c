/*******************************************************************************
 * Name        : minishell.c
 * Author      : Alex Johnson
 * Date        : 4/15/20
 * Description : Creates a minishell inside of the terminal.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <wait.h>


#define BUFSIZE 256
#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT    "\x1b[0m"

sigjmp_buf jmpbuf;

void catch_signal(int sig) {
	write(STDOUT_FILENO, "\n", 1);
	// Jump back to main, don't return from the function
	// Give 1 back to the sigsetjmp to distinguish it from the first time
	// sigsetjmp returned.
	siglongjmp(jmpbuf, 1);
}

void printPrompt(char* directory) {
	printf("%s[%s%s%s]$ ", DEFAULT, BRIGHTBLUE, directory, DEFAULT);
}

int runCommand(char* command) {
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
		return -1;
	} 
	if(pid == 0) {
		// in child
		int num_of_words = 0;
		for(int k = 0; k < sizeof(command); k++) {
			if(command[k] == ' ') {
				num_of_words++;
			}
		}
		num_of_words += 2;
		char* array[num_of_words];

		int i = 0;
		array[i] = strtok(command, " ");
		while(array[i]) {
			array[++i] = strtok(NULL, " ");
		}

		array[i] = NULL;

		if(execvp(array[0], array) == -1) {
        	fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
        	return -1;
        }

        for(int h = 0; h < i; h++) {
        	free(array[h]);
        }
        return 0;
	} 
	if (pid > 0) {
		// in parent
		int wstatus;
	    pid_t wpid;
	    if((wpid = wait(&wstatus)) == -1) {
	    	fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
	    	return -1;
	    }
	    return 0;
	}

	return 0;
}

int main(int argc, char* argv[]) {
	if(argc != 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct sigaction action;

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = catch_signal;
	action.sa_flags = SA_RESTART;
	if(sigaction(SIGINT, &action, NULL) == -1) {
		fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}

	char buf[BUFSIZE];

	/* Save information about the calling enviroment, so you can return to this place later.
	   Returns 0 the first time. When it returns from siglong jump, it returns the value 
	   suplied in siglongjmp */
	sigsetjmp(jmpbuf, 1);
	do {
		char path[BUFSIZE];
		if(getcwd(path, BUFSIZE) == NULL) {
			fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
		printPrompt(path);
		fflush(stdout);

		ssize_t bytes_read = read(STDIN_FILENO, buf, BUFSIZE - 1);
		if (bytes_read > 0) {
			buf[bytes_read - 1] = '\0';
		}
		if(bytes_read == 1) {
			continue;
		}

		bool spacesDone = false;
		char newbuf[BUFSIZE];
		int i = 0;
		int j = 0;
		while(buf[i]) {
			if(buf[i] == ' ' && !spacesDone) {
				i++;
			} else {
				spacesDone = true;
				newbuf[j] = buf[i];
				i++;
				j++;
			}
		}

		newbuf[j] = '\0';

		if(strncmp(newbuf, "cd ", 3) == 0) {

			uid_t userID = getuid();
			struct passwd *user;
			if((user = getpwuid(userID)) == NULL) {
				fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
				return EXIT_FAILURE;
			}

			if((strcmp(newbuf, "cd") == 0) || 
				(strcmp(newbuf, "cd ~") == 0) || 
				(strcmp(newbuf, "cd ") == 0)) {
				// Move to home directory
				if(chdir(user->pw_dir) == -1) {
					fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", 
						user->pw_dir, strerror(errno));
				}
			} else {
				char *dir;
				if((dir = malloc(strlen(newbuf) * sizeof(char))) == NULL) {
					fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
				}
				int i = 3;
				int quotes = 0;
				bool works = true;
				while(newbuf[i]) {
					if((newbuf[i] == '"')) {
						quotes++;
						i++;
					} else if(newbuf[i-1] == ' ' && i != 3 && quotes != 1) {
						works = false;
						break;
					} else {
						dir[i-3-quotes] = newbuf[i];
						i++;
					}
				}
				dir[i-3-quotes] = '\0';
				if(works && ((quotes == 2) || (quotes == 0))) {
					if(chdir(dir) == -1) {
						fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", 
							dir, strerror(errno));
					}
				} else {
					fprintf(stderr, "Error: Too many arguments to cd.\n");
					continue;
				}
				free(dir);
			}
			continue;
		}
		if ((strcmp(newbuf, "exit") == 0) || (strcmp(newbuf, "exit ") == 0)) {
			break;
		}

		if(runCommand(newbuf) == -1) {
			break;
		}

		continue;


	} while (1);

	return EXIT_SUCCESS;
}