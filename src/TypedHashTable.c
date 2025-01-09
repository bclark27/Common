#include "TypedHashTable.h"

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

TypedHashTable * TypedHashTable_init(U4 keyLen, bool keyPassByRef, FreeDataFunction freeKeyFunc, U4 valLen, bool valPassByRef, FreeDataFunction freeValFunc, HashFunction keyHashFunc)
{
  TypedHashTable * tht = calloc(1, sizeof(TypedHashTable));
  if (keyPassByRef) printf("super weird\n");
  tht->ht = HashTable_init(keyPassByRef, valPassByRef);

  tht->freeKeyFunc = freeKeyFunc;
  tht->freeValFunc = freeValFunc;
  tht->keyHashFunc = keyHashFunc;
  tht->keyLen = keyLen;
  tht->valLen = valLen;
  tht->valPassByRef = valPassByRef;
  tht->keyPassByRef = keyPassByRef;

  return tht;
}

void TypedHashTable_free(TypedHashTable * tht)
{
  if (!tht) return;
  HashTable_free(tht->ht);
  free(tht);
}

void TypedHashTable_iterateKeys(TypedHashTable * tht, CallbackFunction callBack, void * args)
{
  if (!tht || !callBack) return;
  HashTable_iterateTableKeys(tht->ht, callBack, args);
}

void TypedHashTable_iterateVals(TypedHashTable * tht, CallbackFunction callBack, void * args)
{
  if (!tht || !callBack) return;
  HashTable_iterateTableVals(tht->ht, callBack, args);
}

void TypedHashTable_iterateKV(TypedHashTable * tht, CallbackFunction callBack, void * args)
{
  if (!tht || !callBack) return;
  HashTable_iterateTableKV(tht->ht, callBack, args);
}

bool TypedHashTable_insert(TypedHashTable * tht, void * key, void * val)
{
  if (!tht) return false;
  return HashTable_insert(tht->ht, key, tht->keyLen, tht->freeKeyFunc, val, tht->valLen, tht->freeValFunc, tht->keyHashFunc);
}

void * TypedHashTable_getRef(TypedHashTable * tht, void * key)
{
  if (!tht) return NULL;
  return HashTable_getRef(tht->ht, key, tht->keyLen, NULL, tht->keyHashFunc);
}

void * TypedHashTable_getVal(TypedHashTable * tht, void * key)
{
  if (!tht) return NULL;
  return HashTable_getVal(tht->ht, key, tht->keyLen, NULL, tht->keyHashFunc);
}

void TypedHashTable_remove(TypedHashTable * tht, void * key)
{
  if (!tht) return;
  HashTable_remove(tht->ht, key, tht->keyLen, tht->keyHashFunc);
}

bool TypedHashTable_keyIn(TypedHashTable * tht, void * key)
{
  if (!tht) return false;
  return HashTable_keyIn(tht->ht, key, tht->keyLen, tht->keyHashFunc);
}

bool TypedHashTable_valIn(TypedHashTable * tht, void * key)
{
  if (!tht) return false;
  return HashTable_valIn(tht->ht, key, tht->keyLen, tht->keyHashFunc);
}
