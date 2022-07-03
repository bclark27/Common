#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include "Common.h"
#include "List.h"

/////////////
//  TYPES  //
/////////////

typedef struct
{
  void * key;
  U4 keyLen;
  FreeDataFunction freeKeyFunc;
  void * val;
  U4 valLen;
  FreeDataFunction freeDataFunc;
  bool passByVal;
} HashTableLink;

typedef struct
{
  List * table;
  U4 size;
  bool passByVal;
} HashTable;

/////////////////////////////
//  FUNDTION DECLERATIONS  //
/////////////////////////////

HashTable * HashTable_init(bool passByVal);
void HashTable_free(HashTable * ht);

bool HashTable_insert(HashTable * ht, void * key, U4 keyLen, void * val, U4 valLen, FreeDataFunction freeDataFunc, FreeDataFunction freeKeyFunc);
void * HashTable_getRef(HashTable * ht, void * key, U4 keyLen, U4 * valLen);
void * HashTable_getVal(HashTable * ht, void * key, U4 keyLen, U4 * valLen);
void HashTable_remove(HashTable * ht, void * key, U4 keyLen);
bool HashTable_keyIn(HashTable * ht, void * key, U4 keyLen);
bool HashTable_valIn(HashTable * ht, void * key, U4 keyLen);

#endif
