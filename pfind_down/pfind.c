/*******************************************************************************
 * Name        : pfind.c
 * Author      : Alex Johnson
 * Date        : 3/5/20
 * Description : PFind main file.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "pfinder.h"

int main(int argc, char* argv[]) {
	if(argc < 2 || argc > 5) {
		printf("Usage: %s -d <directory> -p <permissions string> [-h]\n", argv[0]);
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

	if(findPerms(argv[2], argv[4]) == 0) return EXIT_SUCCESS;
	return EXIT_FAILURE;

}