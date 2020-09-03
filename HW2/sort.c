/*******************************************************************************
 * Name        : sort.c
 * Author      : Alex Johnson
 * Date        : 2/20/20
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum {
    STRING,
    INT,
    DOUBLE
} elem_t;

/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind. 
 */

void printUsage() {
	printf("Usage: ./sort [-i|-d] [filename]\n");
	printf("   -i: Specifies the file contains ints.\n");
	printf("   -d: Specifies the file contains doubles.\n");
	printf("   filename: The file to sort\n");
	printf("   No flags defaults to sorting strings.\n");
}

void display_array(void *array, int size, int type, size_t elem_s) {
	void *p = array;
	for (size_t i = 0; i < size; ++i) {
		p = (char*)array + (i * elem_s);
		if (type == 0) {
			printf("%s\n", (char*)p);
		}
		else if (type == 1) {
			printf("%d\n", *(int*)p);
		} else {
			printf("%f\n", *(double*)p); 
		}
	}
}

void free_array(char** arr, int size) {
	for (int i = 0; i < size; ++i) {
		free(arr[i]);
	}
	free(arr);
}

int main(int argc, char **argv) {
	int opt, i_flag, d_flag;

	if(argc == 1) {
		printUsage();
		return EXIT_SUCCESS;
	} else if(argc > 3) {
		printf("Error: Too many arguments supplied.");
		printUsage();
		return EXIT_FAILURE;
	} else {
		while((opt = getopt(argc, argv, ":id:")) != -1) {
			switch (opt) {
				case 'i':
					i_flag = 1;
					break;
				case 'd':
					d_flag = 1;
					break;
				case '?':
					printf("Error: Unknown option '%s' recieved.\n", argv[1]);
					printUsage();
					break;
			}
		}
	}

	char* filename = argv[argc-1];
	char** arr;

	if(i_flag == 1) {
		arr = malloc(MAX_ELEMENTS * sizeof(int));
	} else if(d_flag == 1) {
		arr = malloc(MAX_ELEMENTS * sizeof(double));
	} else {
		arr = malloc(MAX_ELEMENTS * sizeof(char));
	}

	char buf[MAX_ELEMENTS + 2];
	FILE *fp = fopen(filename, "r");

	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open '%s'. %s.\n", 
			filename, strerror(errno));
		return EXIT_FAILURE;
	}

	int offset = 0;
	int length = 0;
	while (fgets(buf, MAX_ELEMENTS + 2, fp) != NULL) {
		char *s  = strchr(buf, '\n');
		if (s == NULL) {
			buf[strlen(buf)] = '\0';
		} else {
			*s = '\0';
		}
		arr[offset] = malloc(MAX_STRLEN * sizeof(char));
		arr[offset] = strdup(buf);
		offset++;
		length++;
	}

	fclose(fp);

	if (i_flag == 1) {
		int vals[offset];
		for (int i = 0; i < offset; ++i) {
			vals[i] = atoi(arr[i]);
		}
		quicksort(vals, offset, sizeof(int), int_cmp);
		display_array(vals, offset, INT, sizeof(int));
		free_array(arr, offset);
		return EXIT_SUCCESS;
	}
	else if (d_flag == 1) {
		double vals[offset];
		for (int i = 0; i < offset; ++i) {
			vals[i] = atof(arr[i]);
		}
		quicksort(vals, offset, sizeof(double), dbl_cmp);
		display_array(vals, offset, DOUBLE, sizeof(double));
		free_array(arr, offset);
		return EXIT_SUCCESS;
	} else {
		quicksort(arr, offset, sizeof(char*), str_cmp);
		for (int i = 0; i < offset; ++i) {
			printf("%s\n", arr[i]);
		}
		//display_array(arr, index, STRING, sizeof(char*));
		free_array(arr, offset);
		
		return EXIT_SUCCESS;
	}
}
