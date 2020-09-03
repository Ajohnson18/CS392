/*******************************************************************************
 * Name        : chatclient.c
 * Author      : Alex Johnson
 * Date        : 5/3/20
 * Description : chatclient implementation.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util.h"

int client_socket = -1; 
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin() {
	int status = get_string(outbuf, BUFLEN-1);
	if(status == 2) {
		fprintf(stderr, "Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
	}
	if (send(client_socket, outbuf, strlen(outbuf), 0) < 0) {        
    	fprintf(stderr, "Error: Failed to send message to server. %s.\n",                
    		strerror(errno));        
    	return -1; 
    }
    if(strcmp(outbuf, "bye") == 0) {
    	printf("Goodbye.\n");
    	return 1;
    }
    return 0;
}	

int handle_client_socket() {
	int bytes_recvd;
    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN-1, 0)) < 0) {        
    	fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n",                
    		strerror(errno));        
    	return -1;  
    }

    if(bytes_recvd == 0) {
    	fprintf(stderr, "\nConnection to server has been lost.\n");
    	return -2;
    }

    inbuf[bytes_recvd] = '\0';

    if(strcmp(inbuf, "bye") == 0) {
    	printf("\nServer initiated shutdown.\n");
    	return -3;
    }

    printf("\n%s\n", inbuf);
	return 0;
}

int main(int argc, char* argv[]) {
	int ip_conversion, retval, port;
	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if(argc != 3) {
		fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	ip_conversion = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);    
	if (ip_conversion == 0) {        
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);        
		retval = EXIT_FAILURE;
        goto EXIT;    
    } else if (ip_conversion < 0) {        
    	fprintf(stderr, "Error: Failed to convert IP address. %s.\n",                
    		strerror(errno));        
    	retval = EXIT_FAILURE;        
    	goto EXIT;    
    }

    if(!parse_int(argv[2], &port, "port number")) {
    	retval = EXIT_FAILURE;
    	goto EXIT;
    }

    if(port < 1024 || port > 65535) {
    	fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
    	retval = EXIT_FAILURE;
    	goto EXIT;
    }

    do {        
    	printf("Enter your username: ");        
    	fflush(stdout);        
    	ssize_t bytes_read = read(STDIN_FILENO, username, BUFLEN - 1);  
    	if(bytes_read > MAX_NAME_LEN) {
    		printf("Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
    		continue;
    	}     
    	if (bytes_read == 1) {           
    	 	continue;        
    	}
    	if (bytes_read > 0) {            
    		username[bytes_read-1] = '\0';        
    	}        
    	break; 
    } while (true);

    printf("Hello, %s. Let's try to connect you to a server.\n", username);

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {        
    	fprintf(stderr, "Error: Failed to create socket. %s.\n",                
    		strerror(errno));        
    	retval = EXIT_FAILURE;        
    	goto EXIT;    
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {        
    	fprintf(stderr, "Error: Failed to connect to server. %s.\n",                
    		strerror(errno));        
    	retval = EXIT_FAILURE;        
    	goto EXIT;    
    }

    int bytes_recvd;
    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN-1, 0)) < 0) {        
    	fprintf(stderr, "Error: Failed to receive message from server. %s.\n",                
    		strerror(errno));        
    	retval = EXIT_FAILURE;        
    	goto EXIT;    
    }

    inbuf[bytes_recvd] = '\0';

    if(bytes_recvd == 0) {
    	fprintf(stderr, "All connections are busy. Try again later.\n");
    	retval = EXIT_FAILURE;
    	goto EXIT;
    }

    printf("\n%s\n\n", inbuf);

    if (send(client_socket, username, strlen(username), 0) < 0) {        
    	fprintf(stderr, "Error: Failed to send username to server. %s.\n",                
    		strerror(errno));        
    	retval = EXIT_FAILURE;        
    	goto EXIT;    
    }

    fd_set sockSet;
    bool running = true;
    while (running) {
    	FD_ZERO(&sockSet);
    	FD_SET(STDIN_FILENO, &sockSet);
    	FD_SET(client_socket, &sockSet);

    	printf("[%s]: ", username);        
    	fflush(stdout);        

    	select(client_socket+1, &sockSet, NULL, NULL, NULL);

    	if(FD_ISSET(0, &sockSet)) {
    		int status = handle_stdin();
    		if(status == -1) {
    			retval = EXIT_FAILURE;
    			goto EXIT;
    		} else if(status == 0) {
    			continue;
    		} else {
    			retval = EXIT_SUCCESS;
    			goto EXIT;
    		}
    	}

    	if(FD_ISSET(client_socket, &sockSet)) {
    		int status = handle_client_socket();
    		if(status == -3) {
    			retval = EXIT_SUCCESS;
    			goto EXIT;
    		} else if(status == -2) {
    			retval = EXIT_FAILURE;
    			goto EXIT;
    		} else {
    			continue;
    		}
    	}
    }



EXIT:    
	if (fcntl(client_socket, F_GETFD) >= 0) {        
		close(client_socket);    
	}    
	return retval;
}