#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define INFILE "forkwithfile.c"
#define BUFSIZE 128
int main(int argc, char *argv[]) {
	pid_t pid;
	if ((pid = fork()) < 0) {

	} else if (pid > 0) {
		int status;
		do {
			int w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
			if (w == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}

			if (WIFEXITED(status)) {
				printf("Child process %ld exited, status=%d.\n", (long)pid, WEXITSTATUS(status));
			} else if (WIFSIGNALED(status)) {
				printf("Child process %ld killed by signal %d.\n", (long)pid, WTERMSIG(status));
			} else if (WIFSTOPPED(status)) {
				printf("Child process %ld stopped by signal %d.\n", (long)pid, WSTOPSIG(status));				
			} else if (WIFCONTINUED(status)) {
				printf("Child process %ld continued.\n", (long)pid);
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		printf("All done!\n");
	} else {
		execv("../pfind/pfind" argv);
	}
	return EXIT_SUCCESS;
}
