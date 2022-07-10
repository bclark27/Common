#include "TypedHashTable.h"

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

TypedHashTable * TypedHashTable_init(U4 keyLen, U4 valLen, bool passByVal, FreeDataFunction freeDataFunc)
{
  TypedHashTable * tht = calloc(1, sizeof(TypedHashTable));
  tht->ht = HashTable_init(passByVal);

  tht->freeDataFunc = freeDataFunc;
  tht->keyLen = keyLen;
  tht->valLen = valLen;

  return tht;
}

void TypedHashTable_free(TypedHashTable * tht)
{
  if (!tht) return;
  HashTable_free(tht->ht);
  free(tht);
}

void TypedHashTable_iterateKeys(TypedHashTable * tht, callbackFunction callBack, void * args)
{
  if (!tht || !callBack) return;
  HashTable_iterateTableKeys(tht->ht, callBack, args);
}

void TypedHashTable_iterateVals(TypedHashTable * tht, callbackFunction callBack, void * args)
{
  if (!tht || !callBack) return;
  HashTable_iterateTableVals(tht->ht, callBack, args);
}

void TypedHashTable_iterateKV(TypedHashTable * tht, callbackFunction callBack, void * args)
{
  if (!tht || !callBack) return;
  HashTable_iterateTableKV(tht->ht, callBack, args);
}

bool TypedHashTable_insert(TypedHashTable * tht, void * key, void * val)
{
  if (!tht) return false;
  return HashTable_insert(tht->ht, key, tht->keyLen, val, tht->valLen, tht->freeDataFunc, NULL);
}

void * TypedHashTable_getRef(TypedHashTable * tht, void * key)
{
  if (!tht) return NULL;
  return HashTable_getRef(tht->ht, key, tht->keyLen, NULL);
}

void * TypedHashTable_getVal(TypedHashTable * tht, void * key)
{
  if (!tht) return NULL;
  return HashTable_getVal(tht->ht, key, tht->keyLen, NULL);
}

void TypedHashTable_remove(TypedHashTable * tht, void * key)
{
  if (!tht) return;
  HashTable_remove(tht->ht, key, tht->keyLen);
}

bool TypedHashTable_keyIn(TypedHashTable * tht, void * key)
{
  if (!tht) return false;
  return HashTable_keyIn(tht->ht, key, tht->keyLen);
}

bool TypedHashTable_valIn(TypedHashTable * tht, void * key)
{
  if (!tht) return false;
  return HashTable_valIn(tht->ht, key, tht->keyLen);
}
