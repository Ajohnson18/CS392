/*******************************************************************************
 * Name        : spfind.c
 * Author      : Alex Johnson
 * Partner	   : Andrew Johnson
 * Date        : 3/31/20
 * Description : SPfind implementation.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFSIZE 16384

bool testPerms(char *perms) {
	if(strlen(perms) != 9) {
		return false;
	}

	for(int i = 0; i < strlen(perms); i++) {
		if((i == 0 || i == 3 || i == 6) && ((perms[i] != '-') && (perms[i] != 'r'))) {
			fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", perms);
			return false;
		} else if ((i == 1 || i == 4 || i == 7) && ((perms[i] != '-') && (perms[i] != 'w'))) {
			fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", perms);
			return false;
		} else {
			if((i == 2 || i == 5 || i == 8) && ((perms[i] != '-') && (perms[i] != 'x'))) {
			fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", perms);
			return false;
			}
		}
	}
	return true;
}

int main(int argc, char* argv[]) {

	if (argc == 1 || argc == 6) {
		fprintf(stderr, "Usage: %s -d <directory> -p <permissions string> [-h]\n", argv[0]);
		return EXIT_FAILURE;
	}

	int opt, d_flag = 0, p_flag = 0;
	while((opt = getopt(argc, argv, ":d:p:h")) != -1) {
		switch (opt) {
			case 'h':
				printf("Usage: %s -d <directory> -p <permissions string> [-h]\n", argv[0]);
				return EXIT_SUCCESS;
			case 'd':
				d_flag = 1;
				break;
			case 'p':
				p_flag = 1;
				break;
			case '?':
				fprintf(stderr, "Error: Unknown option '-%c' received.\n", optopt);
				return EXIT_FAILURE;
		}
	}

	if(d_flag == 0) {
		printf("Error: Required argument -d <directory> not found.\n");
	   	return EXIT_FAILURE;
	}

	if(p_flag == 0) {
		printf("Error: Required argument -p <permissions string> not found.\n");
	 	return EXIT_FAILURE;
	}

	bool valid = testPerms(argv[4]);
	
	struct stat statbuf;
	if (stat(argv[2], &statbuf) < 0) {
		fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", argv[2],
		    strerror(errno));
		return EXIT_FAILURE;
	}

	int pfind_sort[2], sort_parent[2];
	pipe(pfind_sort);
	pipe(sort_parent);

	pid_t pid[2];
	if((pid[0] = fork()) == 0) {
		// pfind
		close(pfind_sort[0]);
		dup2(pfind_sort[1], STDOUT_FILENO);

		// close unrelated fds
		close(sort_parent[0]);
		close(sort_parent[1]);

		char* args[] = {"./pfind", "-d", argv[2], "-p", argv[4], NULL};
		if(execvp(args[0], args) == -1) {
			fprintf(stderr, "Error: pfind failed.\n");
			return EXIT_FAILURE;
		}
	}

	if((pid[1] = fork()) == 0) {
		// sort
		close(pfind_sort[1]);
		dup2(pfind_sort[0], STDIN_FILENO);
		close(sort_parent[0]);
		dup2(sort_parent[1], STDOUT_FILENO);

		if(execlp("sort", "sort", "-i", NULL) == -1) {
			fprintf(stderr, "Error: sort failed.\n");
			return EXIT_FAILURE;
		}
	}

	// parent
	close(sort_parent[1]);
	dup2(sort_parent[0], STDIN_FILENO);

	// close unwanteds
	close(pfind_sort[0]);
	close(pfind_sort[1]);

	char *buf;
	int countmatches = 0;

	buf = malloc(sizeof(char) * (BUFSIZE+1));
	while (1) {
		ssize_t count = read(sort_parent[0], buf, sizeof(buf));
		char* t;
		for(t = buf; *t != '\0'; t++) {
			if (*t == '\n') countmatches++;
		}
		if(count == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				fprintf(stderr, "Error: read() failed to read in file descriptors.");
				exit(EXIT_FAILURE);
			}
		} else if (count == 0) {
			break;
		} else {
			write(STDOUT_FILENO, buf, count);
			buf[count] = '\0';
		}
	}

	if (valid) printf("Total Matches: %d\n", countmatches-1);

	int status1, status2;

	int w1 = waitpid(pid[0], &status1, WUNTRACED | WCONTINUED);
	if(w1 == -1) {
		fprintf(stderr, "Error: Waitpid() failed to wait for the pid.\n");
		exit(EXIT_FAILURE);
	}
	int w2 = waitpid(pid[1], &status2, WUNTRACED | WCONTINUED);
	if(w2 == -1) {
		fprintf(stderr, "Error: Waitpid() failed to wait for the pid.\n");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}