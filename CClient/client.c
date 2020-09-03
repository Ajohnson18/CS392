#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "clientserver.h"

int main(int argc, char* argv[]) {
	int client_socket, bytes_recieved, ip_conversion, retval = EXIT_SUCCESS;
	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	char buf[BUFLEN];
	char *addr_str = "127.0.0.1";

	// Create a reliable stream socket using tcp
	if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Error: Failed to create socket. %s.\n",
			strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	int opt;
	if (setsockopt(server_addr, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) != 0) {
		fprintf(stderr, "Error: Failed to set socket options. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	// Convert character string to network address
	ip_conversion = inet_pton(AF_INET, addr_str, &server_addr.sin_addr);
	if(ip_conversion == 0) {
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", addr_str);
		retval = EXIT_FAILURE;
		goto EXIT;
	} else if(ip_conversion < 0) {
		fprintf(stderr, "Error: Failed to convert IP address. %s.\n", strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	// Construct the server address structure.
	memset(&server_addr, 0, addrlen); // Zero out structure
	server_addr.sin_family = AF_INET; // Internet address family
	//server_addr.sin_addr.s_addr = inet_addr(addr_str); // IP Address
	server_addr.sin_port = htons(PORT); // Server port, 16 bits

	if(connect(client_socket, (struct sockaddr *)&server_addr, 
		sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "Error: Failed to connect to server. %s.\n", 
			strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	memset(buf, 0, BUFLEN);
	for (int i = 1; i < argc; i++) {
		if((strlen(buf) + strlen(argv[i]) + 1) >= BUFLEN) {
			break;
		} 
		strncat(buf, argv[i], BUFLEN - 1);
		if (i != argc - 1) {
			strncat(buf, " ", BUFLEN - 1);
		}
	}

	printf("%s\n", buf);
	printf("Sending message to server at [%s:%d].\n",
		inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
	if (send(client_socket, buf, strlen(buf), 0) < 0) {
		fprintf(stderr, "Error: Failed to send message to server. %s.\n",
			strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	if((bytes_recieved = recv(client_socket, buf, BUFLEN - 1, 0)) < 0) {
		fprintf(stderr, "Error: Failed to recieve message from server. %s.\n",
			strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}

	buf[bytes_recieved] = '\0';
	printf("Recieved response from server: %s\n", buf);

	EXIT:
		if(fcntl(client_socket, F_GETFD) >= 0) {
			close(client_socket);
		}

		return retval;

}