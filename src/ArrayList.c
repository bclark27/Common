#include "ArrayList.h"

////////////////////
//  PRIVATE VARS  //
////////////////////

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

void incresseSize(ArrayList * al);

//////////////////////
// PUBLIC FUNCTIONS //
//////////////////////

ArrayList * ArrayList_init(U4 elementSize, U4 initSize)
{
  ArrayList * al = calloc(1, sizeof(ArrayList));
  al->elementSize = elementSize;
  al->data = (unsigned char *)calloc(initSize, elementSize);
  al->listLen = initSize;
  al->elementCount = 0;

  return al;
}

void ArrayList_freeList(ArrayList * al)
{
  if (al == NULL) return;

  if (al->data) free(al->data);
  free(al);
}

void ArrayList_append(ArrayList * al, void * data)
{
  if (al == NULL || data == NULL) return;

  if (al->elementCount >= al->listLen)
  {
    incresseSize(al);
  }

  memcpy(al->data + (al->elementCount * al->elementSize), data, al->elementSize);

  al->elementCount++;
}

bool ArrayList_insert(ArrayList * al, U4 index, void * data)
{
  if (al == NULL || data == NULL || index >= al->listLen) return 0;

  memcpy(al->data + (index * al->elementSize), data, al->elementSize);
  return 1;
}

void * ArrayList_get(ArrayList * al, U4 index)
{
  if (al == NULL || index >= al->listLen) return NULL;
  return (void *)(al->data + index * al->elementSize);
}

void ArrayList_delete(ArrayList * al, U4 index)
{
  if (al == NULL || index >= al->listLen) return;

  memset(al->data + (index * al->elementSize), 0, al->elementSize);
}

void ArrayList_eraseList(ArrayList * al)
{
  if (al == NULL) return;
  memset(al->data, 0, al->elementSize * al->listLen);
  al->elementCount = 0;
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

void incresseSize(ArrayList * al)
{
  unsigned char * p = calloc(DEFAULT_LIST_LEN + al->listLen, al->elementSize);
  memcpy(p, al->data, al->elementSize * al->listLen);
  unsigned char * temp = al->data;
  al->data = p;
  al->listLen += DEFAULT_LIST_LEN;
  free(temp);
}
