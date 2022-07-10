#ifndef MEM_H_
#define MEM_H_

#include <stdlib.h>

///////////////
//  DEFINES  //
///////////////

#ifdef TRACK_MEM

#define malloc(s)       mem_malloc(s, __FILE__, __LINE__)
#define calloc(n, s)    mem_calloc(n, s, __FILE__, __LINE__)
#define free(p)         mem_free(p, __FILE__, __LINE__)

#endif


/////////////////////////////
//  FUNDTION DECLERATIONS  //
/////////////////////////////

void mem_init(void);
void mem_freeTracker(void);
void mem_print(void);
void * mem_malloc(size_t size, const char * fileName, int lineNum);
void * mem_calloc(size_t nmemb, size_t size, const char * fileName, int lineNum);
void mem_free(void * ptr, const char * fileName, int lineNum);

#endif
