#ifndef COMMON_H_
#define COMMON_H_

#define TRACK_MEM

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "Mem.h"

///////////////
//  DEFINES  //
///////////////

#define U1 unsigned char
#define U2 unsigned short
#define U4 unsigned int
#define U8 unsigned long

#define I1 char
#define I2 short
#define I4 int
#define I8 long

#define R4 float
#define R8 double
#define R16 long double

#define RAND_DOUBLE (((rand() / (double)RAND_MAX) * 2) - 1)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/////////////
//  TYPES  //
/////////////

typedef void (* FreeDataFunction)(void *);
typedef U1 (* CompareFunction)(void *, void *);
typedef void ( *callbackFunction)(void *, void *);

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

#endif
