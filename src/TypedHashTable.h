#ifndef TYPED_HASH_TABLE_H_
#define TYPED_HASH_TABLE_H_

#include "Common.h"
#include "HashTable.h"

/////////////
//  TYPES  //
/////////////

typedef struct
{
  HashTable * ht;
  FreeDataFunction freeDataFunc;
  U4 keyLen;
  U4 valLen;
} TypedHashTable;

/////////////////////////////
//  FUNDTION DECLERATIONS  //
/////////////////////////////

TypedHashTable * TypedHashTable_init(U4 keyLen, U4 valLen, bool passByVal, FreeDataFunction freeDataFunc);
void TypedHashTable_free(TypedHashTable * tht);

void TypedHashTable_iterateKeys(TypedHashTable * tht, callbackFunction callBack, void * args);
void TypedHashTable_iterateVals(TypedHashTable * tht, callbackFunction callBack, void * args);
void TypedHashTable_iterateKV(TypedHashTable * tht, callbackFunction callBack, void * args);
bool TypedHashTable_insert(TypedHashTable * tht, void * key, void * val);
void * TypedHashTable_getRef(TypedHashTable * tht, void * key);
void * TypedHashTable_getVal(TypedHashTable * tht, void * key);
void TypedHashTable_remove(TypedHashTable * tht, void * key);
bool TypedHashTable_keyIn(TypedHashTable * tht, void * key);
bool TypedHashTable_valIn(TypedHashTable * tht, void * key);

#endif
