#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

#define BUFSIZE 128

sigjmp_buf jmpbuf;

void catch_signal(int sig) {
	write(STDOUT_FILENO, "\n", 1);
	// Jump back to main, don't return from the function
	// Give 1 back to the sigsetjmp to distinguish it from the first time
	// sigsetjmp returned.
	siglongjmp(jmpbuf, 1);
}

int main() {
	struct sigaction action;

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = catch_signal;
	action.sa_flags = SA_RESTART;
	if(sigaction(SIGINT, &action, NULL) == -1) {
		perror("sigaction()");
		return EXIT_FAILURE;
	}

	char buf[BUFSIZE];

	/* Save information about the calling enviroment, so you can return to this place later.
	   Returns 0 the first time. When it returns from siglong jump, it returns the value 
	   suplied in siglongjmp */
	sigsetjmp(jmpbuf, 1);
	do {
		printf("What is your name? ");
		fflush(stdout);

		ssize_t bytes_read = read(STDIN_FILENO, buf, BUFSIZE - 1);
		if (bytes_read > 0) {
			buf[bytes_read - 1] = '\0';
		}
		if(bytes_read == 1) {
			continue;
		}
		if (strncmp(buf, "exit", 4) == 0) {
			printf("Bye.\n");
			break;
		}
		printf("Hi, %s!\n", buf);
	} while (1);
	return EXIT_SUCCESS;
}