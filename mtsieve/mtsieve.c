/*******************************************************************************
 * Name        : mtsieve.c
 * Author      : Alex Johnson
 * Date        : 4/26/20
 * Description : MTSIEVE main file.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <sys/sysinfo.h>

typedef struct arg_struct {
    int start;
    int end;
} thread_args;

int sVal, eVal, tVal, retVal, total_num;
pthread_mutex_t lock;

int countThrees(int num) {
	int count = 0;
	while(num > 0) {
		if((num % 10) == 3) count++;
		num = floor(num / 10);
	}
	return count;
}

void simpleSieve(int limit, int *primes) {
	bool* hold = (bool *)malloc((limit + 1) * sizeof(bool));
	memset(hold, false, sizeof(bool) * limit);
	int index = 0;
	for(int i = 2; i < limit; i++) {
		if(hold[i] == false) {
			primes[index] = i;
			index++;
			for(int j = i; j <= limit; j += i) {
				hold[j] = true;
			}
		}
	} 
	free(hold);
} 

void* sieve(void *ptr) {
	thread_args *threadArg = (thread_args *)ptr;
	int low = threadArg->start;
	int high = threadArg->end;
	int low_primes[(int)floor(sqrt(high))];

	int retval2;

	memset(low_primes, 0, sizeof(low_primes));
	simpleSieve(floor(sqrt(high)), low_primes);

	int n = high - low + 1;

	bool* hold = (bool *)malloc((n+1) * sizeof(bool));
	memset(hold, false, sizeof(bool) * (n));

	for(int i = 0; i < sizeof(low_primes); i++) {
		if(low_primes[i] == 0) {
			break;
		}

		int lowerLimit = floor(low / low_primes[i]) * low_primes[i];
		if(lowerLimit < low) {
			lowerLimit += low_primes[i];
		}
		if(lowerLimit == low_primes[i]) {
			lowerLimit += low_primes[i];
		}
	
		for(int j = lowerLimit; j <= high; j+= low_primes[i]) {
			hold[j - low] = true;
		}
	}

	if((retval2 = pthread_mutex_lock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval2));
    }

	for(int i = low; i <= high; i++) {
		if(!hold[i-low]) {
			if(countThrees(i) > 1) {
				total_num++;
			}
		}
	}

	if((retval2 = pthread_mutex_unlock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval2));
    }

    free(hold);
	pthread_exit(NULL);
}

bool is_integer(char *input) {
	int start = 0, len = strlen(input);
	if(len >= 1 && input[0] == '-') {
		if(len < 2) {
			return false;
		}
		start = 1;
	}
	for(int i = start; i < len; i++) {
		if (!isdigit(input[i])) {
			return false;
		}
	}
	return true;
}

bool get_integer(char *input, int *value, char opt) {
	long long ll_i;
	if(sscanf(input, "%lld", &ll_i) != 1) {
		return false;
	}
	*value = (int)ll_i;
	if(ll_i != (long long)*value) {
		fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", opt);
		return false;
	}
	return true;
}

bool parse_arg(char* optarg, int *val, char opt) {
	if (optarg == NULL) {
		fprintf(stderr, "Error: Option -%c requires an argument.\n", opt);
		return false;
	}
	if(!is_integer(optarg)) {
		fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, opt);
		return false;
	} else if(!get_integer(optarg, val, opt)) {
		return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	int opt, s_flag = 0, e_flag = 0;
	while((opt = getopt(argc, argv, ":s:e:t:")) != -1) {
		switch (opt) {
			case 's':
				s_flag = 1;
				if(parse_arg(optarg, &sVal, 's') == false) {
					return EXIT_FAILURE;
				}
				break;
			case 'e':
				e_flag = 1;
				if(parse_arg(optarg, &eVal, 'e') == false) {
					return EXIT_FAILURE;
				}
				break;
			case 't':
				if(parse_arg(optarg, &tVal, 't') == false) {
					return EXIT_FAILURE;
				}
				break;
			case ':':     
				if (optopt == 'e' || optopt == 's' || optopt == 't') {         
					fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt); 
			  		return EXIT_FAILURE; 
			    }  
		    case '?':
		    	if (isprint(optopt)) {     
			        fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);   
			    } else {       
			        fprintf(stderr, "Error: Unknown option character '\\x%x'.\n",   
		              optopt);   
		        }    
		        return EXIT_FAILURE; 
		}
	}

	//if(argc < 7) {
	if(argc == 1) {
		printf("Usage: %s -s <starting value> -e <ending value> -t <num threads>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if(argv[optind] != NULL) {
		fprintf(stderr, "Error: Non-option argument '%s' supplied.\n", argv[optind]);
		return EXIT_FAILURE;
	}

	if(s_flag && sVal == 0) {
		fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
		return EXIT_FAILURE;
	}

	if(sVal < 2) {
		fprintf(stderr, "Error: Starting value must be >= 2.\n");
		return EXIT_FAILURE;
	}

	if(e_flag && eVal == 0) {
		fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
		return EXIT_FAILURE;
	}

	if(eVal < 2) {
		fprintf(stderr, "Error: Ending value must be >= 2.\n");
		return EXIT_FAILURE;
	}

	if(tVal < 1) {
		fprintf(stderr, "Error: Number of threads cannot be less than 1.\n");
		return EXIT_FAILURE;
	}

	int processors = get_nprocs();
	if(tVal > (2 * processors)) {
		fprintf(stderr, "Error: Number of threads cannot exceed twice the number of processors(%d).\n", processors);
		return EXIT_FAILURE;
	}

	int segments = 0;
	if((eVal - sVal + 1) < tVal) {
		segments = (eVal - sVal + 1);
	} else {
		segments = tVal;
	}

	printf("%d segment", segments);
	if(segments > 1) {
		printf("s");
	}
	printf(":\n");

	pthread_t threads[segments];
	thread_args targs[segments];

	if((retVal = pthread_mutex_init(&lock, NULL)) != 0) {
		fprintf(stderr, "Error: Cannot create mutex. %s.\n", strerror(retVal));
		return EXIT_FAILURE;
	}

	for(int i = 0; i < segments; i++) {
		int retval;
		int inc = (eVal - sVal) / tVal;
		if(i != 0) {
			targs[i].start = sVal + (inc * i) + 1;
		} else {
			targs[i].start = sVal + (inc * i);
		}
		if((i+1) == segments) {
			targs[i].end = sVal + (inc * (i+1)) + (eVal % (sVal + (inc * (i+1))));
		} else {
			targs[i].end = sVal + (inc * (i+1));
		}
		printf("[%d, %d]\n", targs[i].start, targs[i].end);
		if ((retval = pthread_create(&threads[i], NULL, sieve, &targs[i])) != 0) {
 			fprintf(stderr, "Error: Cannot create thread %d. "
 							"No more threads will be created. %s.\n", 
 							i, strerror(retval));
 			return EXIT_FAILURE;
 		}

	}

	for (int i = 0; i < segments; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Warning: Thread %d did not join properly.\n",
                    i);
        }
    }

	if ((retVal = pthread_mutex_destroy(&lock)) != 0) {
        fprintf(stderr, "Error: Cannot destroy mutex. %s.\n", strerror(retVal));
        return EXIT_FAILURE;
    }


	printf("Total primes between %d and %d with two or more '3' digits: %d\n", sVal, eVal, total_num);

	return EXIT_SUCCESS;
}