#include "HashSet.h"

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

HashSet * HashSet_init(U4 keyLen, bool passByVal, FreeDataFunction freeDataFunc)
{
  HashSet * hs = calloc(1, sizeof(HashSet));
  hs->ht = HashTable_init(passByVal);

  hs->keyLen = keyLen;
  hs->freeDataFunc = freeDataFunc;

  return hs;
}

void HashSet_free(HashSet * hs)
{
  if (!hs) return;
  HashTable_free(hs->ht);
  free(hs);
}

void HashSet_iterate(HashSet * hs, callbackFunction callBack, void * args)
{
  if (!hs || !callBack) return;
  HashTable_iterateTableKeys(hs->ht, callBack, args);
}

bool HashSet_insert(HashSet * hs, void * key)
{
  if (!hs) return false;
  return HashTable_insert(hs->ht, key, hs->keyLen, NULL, 0, NULL, hs->freeDataFunc);
}

void HashSet_remove(HashSet * hs, void * key)
{
  HashTable_remove(hs->ht, key, hs->keyLen);
}

bool HashSet_keyIn(HashSet * hs, void * key)
{
  return HashTable_keyIn(hs->ht, key, hs->keyLen);
}
