#include <stdio.h>
#include <stdlib.h>
#include "add.h"

#define PI 3.14159
#define E  2.71828

int main() {
	int int_result = add_ints(PI, E);
	printf("%f + %f = %d\n", PI, E, int_result);

	double dbl_result = add_doubles(PI, E);
	printf("%f + %f = %f\n", PI, E, dbl_result);

	return EXIT_SUCCESS;
}