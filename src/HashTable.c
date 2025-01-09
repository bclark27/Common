#include "HashTable.h"

/////////////
//  TYPES  //
/////////////

typedef struct
{
  CallbackFunction func;
  void * args;
} hashLinkCallbackPackage;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static HashTableLink createHashLink(HashTable * ht, void * key, U4 keyLen, FreeDataFunction freeKeyFunc, void * val, U4 valLen, FreeDataFunction freeValFunc, HashFunction keyHashFunc);
static void freeHashLink(void * ptr);
static Link * keyAlreadyExists(HashTable * ht, void * key, U4 keyLen, U4 hashVal);
static void iterateValsHelper(void * hashLink, void * args);
static void iterateKeysHelper(void * hashLink, void * args);
static void iterateKVHelper(void * hashLink, void * args);
static U4 getHashVal(void * key, U4 keyLen, HashFunction keyHashFunc);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

HashTable * HashTable_init(bool keyPassByRef, bool valPassByRef)
{
  HashTable * ht = calloc(1, sizeof(HashTable));
  ht->table = calloc(DEFAULT_HASH_TABLE_LENGTH, sizeof(List));
  ht->size = DEFAULT_HASH_TABLE_LENGTH;
  ht->elementCount = 0;
  ht->valPassByRef = valPassByRef;
  ht->keyPassByRef = keyPassByRef;

  for (U4 i = 0; i < DEFAULT_HASH_TABLE_LENGTH; i++)
  {
    List_listObjInit(&ht->table[i], sizeof(HashTableLink), true, NULL, freeHashLink);
  }

  return ht;
}

void HashTable_free(HashTable * ht)
{
  if (ht == NULL) return;

  for (U4 i = 0; i < ht->size; i++)
  {
    List_clearList(&ht->table[i]);
  }

  free(ht->table);
  free(ht);
}

bool HashTable_insert(HashTable * ht, void * key, U4 keyLen, FreeDataFunction freeKeyFunc, void * val, U4 valLen, FreeDataFunction freeValFunc, HashFunction keyHashFunc)
{
  if (!ht || !key || keyLen == 0) return false;

  U4 hash = getHashVal(key, keyLen, keyHashFunc);

  U4 index = hash % ht->size;
  List * thisList = &(ht->table[index]);

  Link * link = keyAlreadyExists(ht, key, keyLen, hash);
  HashTableLink entry = createHashLink(ht, key, keyLen, freeKeyFunc, val, valLen, freeValFunc, keyHashFunc);

  if (link)
  {
    List_destroyLinkAndData(thisList, link);
  }
  else
  {
    ht->elementCount++;
  }

  List_queue(thisList, &entry);

  return true;
}

void * HashTable_getRef(HashTable * ht, void * key, U4 keyLen, U4 * valLen, HashFunction keyHashFunc)
{
  if (!ht || !key || keyLen == 0) return NULL;

  U4 hash = getHashVal(key, keyLen, keyHashFunc);

  Link * link = keyAlreadyExists(ht, key, keyLen, hash);

  if (link)
  {
    HashTableLink * hashLink = (HashTableLink * )link->data;
    if (!hashLink) return NULL;

    if (valLen) *valLen = hashLink->valLen;
    return hashLink->val;
  }

  return NULL;
}

void * HashTable_getVal(HashTable * ht, void * key, U4 keyLen, U4 * valLen, HashFunction keyHashFunc)
{
  if (!ht || !key || keyLen == 0) return NULL;

  U4 hash = getHashVal(key, keyLen, keyHashFunc);

  Link * link = keyAlreadyExists(ht, key, keyLen, hash);

  if (link)
  {
    HashTableLink * hashLink = (HashTableLink * )link->data;
    if (!hashLink || !hashLink->val) return NULL;

    if (valLen) *valLen = hashLink->valLen;
    void * ret = malloc(hashLink->valLen);
    memcpy(ret, hashLink->val, hashLink->valLen);
    return ret;
  }

  return NULL;
}

void HashTable_remove(HashTable * ht, void * key, U4 keyLen, HashFunction keyHashFunc)
{
  if (!ht || !key || keyLen == 0) return;

  U4 hash = getHashVal(key, keyLen, keyHashFunc);

  U4 index = hash % ht->size;

  List * thisList = &(ht->table[index]);
  Link * link = keyAlreadyExists(ht, key, keyLen, hash);

  if (link)
  {
    List_destroyLinkAndData(thisList, link);
    ht->elementCount--;
  }

}

bool HashTable_keyIn(HashTable * ht, void * key, U4 keyLen, HashFunction keyHashFunc)
{
  if (!ht || !key) return false;

  return keyAlreadyExists(ht, key, keyLen, getHashVal(key, keyLen, keyHashFunc)) ? true : false;
}

bool HashTable_valIn(HashTable * ht, void * key, U4 keyLen, HashFunction keyHashFunc)
{
  if (!ht || !key) return false;
  // TODO: implement
  return false;
}

void HashTable_iterateTableVals(HashTable * ht, CallbackFunction callBack, void * args)
{
  if (!ht || !callBack) return;

  hashLinkCallbackPackage package = {.func = callBack, .args = args};

  for (U4 i = 0; i < ht->size; i++)
  {
    List * list = &ht->table[i];
    List_iterateList(list, iterateValsHelper, &package);
  }
}

void HashTable_iterateTableKeys(HashTable * ht, CallbackFunction callBack, void * args)
{
  if (!ht || !callBack) return;

  hashLinkCallbackPackage package = {.func = callBack, .args = args};

  for (U4 i = 0; i < ht->size; i++)
  {
    List * list = &ht->table[i];
    List_iterateList(list, iterateKeysHelper, &package);
  }
}

void HashTable_iterateTableKV(HashTable * ht, CallbackFunction callBack, void * args)
{
  if (!ht || !callBack) return;

  hashLinkCallbackPackage package = {.func = callBack, .args = args};

  for (U4 i = 0; i < ht->size; i++)
  {
    List * list = &ht->table[i];
    List_iterateList(list, iterateKVHelper, &package);
  }
}



/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static HashTableLink createHashLink(HashTable * ht, void * key, U4 keyLen, FreeDataFunction freeKeyFunc, void * val, U4 valLen, FreeDataFunction freeValFunc, HashFunction keyHashFunc)
{
  HashTableLink hashEntry;

  hashEntry.freeKeyFunc = freeKeyFunc;
  hashEntry.keyLen = keyLen;
  hashEntry.keyHashFunc = keyHashFunc;
  hashEntry.keyPassByRef = ht->keyPassByRef;

  if (ht->keyPassByRef)
  {
    hashEntry.key = malloc(sizeof(void *));
    memcpy(hashEntry.key, &key, sizeof(void *));
  }
  else
  {
    hashEntry.key = malloc(keyLen);
    memcpy(hashEntry.key, key, keyLen);
  }

  hashEntry.freeValFunc = freeValFunc;
  hashEntry.valLen = valLen;
  hashEntry.valPassByRef = ht->valPassByRef;

  if (val && valLen)
  {
    if (ht->valPassByRef)
    {
      hashEntry.val = malloc(sizeof(void *));
      memcpy(hashEntry.val, &val, sizeof(void *));
    }
    else
    {
      hashEntry.val = malloc(valLen);
      memcpy(hashEntry.val, val, valLen);
    }
  }
  else
  {
    hashEntry.valLen = 0;
    hashEntry.val = NULL;
  }

  return hashEntry;
}

static void freeHashLink(void * ptr)
{
  HashTableLink * hashLink = (HashTableLink *)ptr;
  if (hashLink == NULL) return;

  if (hashLink->key)
  {
    if (hashLink->keyPassByRef)
    {
      if (hashLink->freeKeyFunc)
      {
        hashLink->freeKeyFunc(*(void **)hashLink->key);
      }
      else
      {
        free(*(void **)hashLink->key);
      }

      free(hashLink->key);
    }
    else
    {
      if (hashLink->freeKeyFunc)
      {
        hashLink->freeKeyFunc(hashLink->key);
      }
      else
      {
        free(hashLink->key);
      }
    }
  }

  if (hashLink->val)
  {
    if (hashLink->valPassByRef)
    {
      if (hashLink->freeValFunc)
      {
        hashLink->freeKeyFunc(*(void **)hashLink->val);
      }
      else
      {
        free(*(void **)hashLink->val);
        free(hashLink->val);
      }
    }
    else
    {
      if (hashLink->freeValFunc)
      {
        hashLink->freeValFunc(hashLink->val);
      }
      else
      {
        free(hashLink->val);
      }
    }
  }

  free(hashLink);
}


static Link * keyAlreadyExists(HashTable * ht, void * key, U4 keyLen, U4 hashVal)
{
  U4 index = hashVal % ht->size;

  List * thisList = &(ht->table[index]);

  for(Link * link = thisList->head; link; link = link->next)
  {
    HashTableLink * hashLink = (HashTableLink *)(link->data);

    if (hashLink->keyLen != keyLen) continue;

    if (hashLink->keyPassByRef)
    {
      if (memcmp(*(void **)key, *(void **)hashLink->key, keyLen) == 0)
      {
        return link;
      }
    }
    else
    {
      if (memcmp(key, hashLink->key, keyLen) == 0)
      {
        return link;
      }
    }
  }

  return NULL;
}

static void iterateValsHelper(void * hashLink, void * args)
{
  HashTableLink * l = hashLink;
  hashLinkCallbackPackage * package = args;

  if (l->valPassByRef)
  {
    package->func(*(void **)l->val, package->args);
  }
  else
  {
    package->func(l->val, package->args);
  }
}

static void iterateKeysHelper(void * hashLink, void * args)
{
  HashTableLink * l = hashLink;
  hashLinkCallbackPackage * package = args;

  if (l->keyPassByRef)
  {
    package->func(*(void **)l->key, package->args);
  }
  else
  {
    package->func(l->key, package->args);
  }
}

static void iterateKVHelper(void * hashLink, void * args)
{
  HashTableLink * l = hashLink;
  hashLinkCallbackPackage * package = args;

  HashKVPair kv = {.key = l->key, .val = l->val};

  if (l->keyPassByRef)
  {
    kv.key = *(void **)l->key;
  }

  if (l->valPassByRef)
  {
    kv.val = *(void **)l->val;
  }

  package->func(&kv, package->args);
}

static U4 getHashVal(void * key, U4 keyLen, HashFunction keyHashFunc)
{
  if (keyHashFunc)
  {
    return keyHashFunc(key, keyLen);
  }
  else
  {
    return FastHash_hash(key, keyLen);
  }
}
