#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include "Standard.h"
#include "List.h"
#include "ArrayList.h"

/////////////
//  TYPES  //
/////////////

typedef struct HashTableLink
{
  unsigned char * key;
  unsigned int keyLen;
  void * data;
  FreeDataFunction dataFreeFunc;
} HashTableLink;

typedef struct HashTable
{
  ArrayList * table;
  unsigned int dataLen;
  FreeDataFunction dataFreeFunc;
} HashTable;

/////////////////////////////
//  FUNDTION DECLERATIONS  //
/////////////////////////////

HashTable * HashTable_init(unsigned int dataLen, FreeDataFunction dataFreeFunc);
void HashTable_free(HashTable * ht);

char HashTable_insert(HashTable * ht, unsigned char * key, unsigned int keyLen, void * data);
void * HashTable_get(HashTable * ht, unsigned char * key, unsigned int keyLen);
void HashTable_remove(HashTable * ht, unsigned char * key, unsigned int keyLen);
bool HashTable_in(HashTable * ht, unsigned char * key, unsigned int keyLen);

#endif
