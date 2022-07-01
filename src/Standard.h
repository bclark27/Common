#ifndef STANDARD_H_
#define STANDARD_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define RAND_DOUBLE (((rand() / (double)RAND_MAX) * 2) - 1)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

void * mallocOrDie(size_t size);
void * callocOrDie(size_t count, size_t size);

#endif
