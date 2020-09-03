#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <process mname>\n", argv[0]);
		return EXIT_FAILURE;
	}
	int ps_to_grep[2], grep_to_wc[2], wc_to_parent[2];
	pipe(ps_to_grep);
	pipe(grep_to_wc);
	pipe(wc_to_parent);

	pid_t pid[3];
	if ((pid[0] = fork()) == 0) {
		// ps
		close(ps_to_grep[0]);
		dup2(ps_to_grep[1], STDOUT_FILENO);

		// Close of unrelated file descriptors
		close(grep_to_wc[0]);
		close(grep_to_wc[1]);
		close(wc_to_parent[0]);
		close(wc_to_parent[1]);

		execlp("ps", "ps", "-A", NULL);
	}

	if((pid[1] = fork()) == 0) {
		// grep
		close(ps_to_grep[1]);
		dup2(ps_to_grep[0], STDIN_FILENO);
		close(grep_to_wc[0]);
		dup2(grep_to_wc[1], STDOUT_FILENO);

		// Close of unrelated file descriptors
		close(wc_to_parent[0]);
		close(wc_to_parent[1]);

		execlp("grep", "grep", "-i", argv[1], NULL);
	}

	if((pid[2] = fork()) == 0) {
		// wc
		close(grep_to_wc[1]);
		dup2(grep_to_wc[0], STDIN_FILENO);
		close(wc_to_parent[0]);
		dup2(wc_to_parent[1], STDOUT_FILENO);

		// Close of unrelated file descriptors
		close(ps_to_grep[0]);
		close(ps_to_grep[1]);

		execlp("wc", "wc", "-l", NULL);
	}

	// parent
	close(wc_to_parent[1]);
	dup2(wc_to_parent[0], STDIN_FILENO);

	// Close of unrelated file descriptors
	close(ps_to_grep[0]);
	close(ps_to_grep[1]);
	close(grep_to_wc[0]);
	close(grep_to_wc[1]);

	char buf[128];
	ssize_t count = read(STDIN_FILENO, buf, sizeof(buf) - 1);
	if(count == -1) {
		perror("read()");
		exit(EXIT_FAILURE);
	}
	buf[count] = '\0';
	printf("Number of processes containing \"%s\": %d\n",
		argv[1], atoi(buf));

	// Wait for children
	wait(NULL);
	wait(NULL);
	wait(NULL);

	return EXIT_SUCCESS;
}