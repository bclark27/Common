#include "HashSet.h"

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

HashSet * HashSet_init(U4 keyLen, bool passByRef, FreeDataFunction freeDataFunc, HashFunction keyHashFunc)
{
  HashSet * hs = calloc(1, sizeof(HashSet));
  hs->ht = HashTable_init(passByRef, false);

  hs->keyLen = keyLen;
  hs->freeDataFunc = freeDataFunc;
  hs->keyHashFunc = keyHashFunc;
  return hs;
}

void HashSet_free(HashSet * hs)
{
  if (!hs) return;
  HashTable_free(hs->ht);
  free(hs);
}

void HashSet_iterate(HashSet * hs, CallbackFunction callBack, void * args)
{
  if (!hs || !callBack) return;
  HashTable_iterateTableKeys(hs->ht, callBack, args);
}

bool HashSet_insert(HashSet * hs, void * key)
{
  if (!hs) return false;
  return HashTable_insert(hs->ht, key, hs->keyLen, hs->freeDataFunc, NULL, 0, NULL, hs->keyHashFunc);
}

void HashSet_remove(HashSet * hs, void * key)
{
  HashTable_remove(hs->ht, key, hs->keyLen, hs->keyHashFunc);
}

bool HashSet_keyIn(HashSet * hs, void * key)
{
  return HashTable_keyIn(hs->ht, key, hs->keyLen, hs->keyHashFunc);
}
