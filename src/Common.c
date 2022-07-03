#include "Common.h"

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

void * mallocOrDie(size_t size)
{
  void * ptr = malloc(size);
  if (ptr == NULL)
  {
    printf("Malloc could not allocate...\n");
    exit(1);
  }
  return ptr;
}

void * callocOrDie(size_t count, size_t size)
{
  void * ptr = calloc(count, size);
  if (ptr == NULL)
  {
    printf("Calloc could not allocate...\n");
    exit(1);
  }
  return ptr;
}
