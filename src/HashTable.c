#include "HashTable.h"

///////////////
//  DEFINES  //
///////////////

#define HASH_SEED                   0x12fa
#define HASH_ROUNDS                 8
#define FEEDBACK_VAL                0x87654321
#define PAD_VAL                     0xc4

/////////////
//  TYPES  //
/////////////

typedef struct
{
  callbackFunction func;
  void * args;
} hashLinkCallbackPackage;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static HashTableLink createHashLink(HashTable * ht, void * key, U4 keyLen, void * val, U4 valLen, FreeDataFunction freeDataFunc, FreeDataFunction freeKeyFunc);
static void freeHashLink(void * ptr);
static U4 hashGen(void * key, U4 keyLen);
static void updateHashState(U4 * state);
static Link * keyAlreadyExists(HashTable * ht, void * key, U4 keyLen);
static void iterateValsHelper(void * hashLink, void * args);
static void iterateKeysHelper(void * hashLink, void * args);
static void iterateKVHelper(void * hashLink, void * args);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

HashTable * HashTable_init(bool passByVal)
{
  HashTable * ht = calloc(1, sizeof(HashTable));
  ht->table = calloc(DEFAULT_HASH_TABLE_LENGTH, sizeof(List));
  ht->size = DEFAULT_HASH_TABLE_LENGTH;
  ht->passByVal = passByVal;
  ht->elementCount = 0;

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

bool HashTable_insert(HashTable * ht, void * key, U4 keyLen, void * val, U4 valLen, FreeDataFunction freeDataFunc, FreeDataFunction freeKeyFunc)
{
  if (!ht || !key || keyLen == 0) return false;

  U4 hash = hashGen(key, keyLen);
  U4 index = hash % ht->size;
  List * thisList = &(ht->table[index]);

  Link * link = keyAlreadyExists(ht, key, keyLen);
  HashTableLink entry = createHashLink(ht, key, keyLen, val, valLen, freeDataFunc, freeKeyFunc);

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

void * HashTable_getRef(HashTable * ht, void * key, U4 keyLen, U4 * valLen)
{
  if (!ht || !key || keyLen == 0) return NULL;

  Link * link = keyAlreadyExists(ht, key, keyLen);

  if (link)
  {
    HashTableLink * hashLink = (HashTableLink * )link->data;
    if (!hashLink) return NULL;

    if (valLen) *valLen = hashLink->valLen;
    return hashLink->val;
  }

  return NULL;
}

void * HashTable_getVal(HashTable * ht, void * key, U4 keyLen, U4 * valLen)
{
  if (!ht || !key || keyLen == 0) return NULL;

  Link * link = keyAlreadyExists(ht, key, keyLen);

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

void HashTable_remove(HashTable * ht, void * key, U4 keyLen)
{
  if (!ht || !key || keyLen == 0) return;

  U4 hash = hashGen(key, keyLen);
  U4 index = hash % ht->size;

  List * thisList = &(ht->table[index]);
  Link * link = keyAlreadyExists(ht, key, keyLen);

  if (link)
  {
    List_destroyLinkAndData(thisList, link);
    ht->elementCount--;
  }

}

bool HashTable_keyIn(HashTable * ht, void * key, U4 keyLen)
{
  if (!ht || !key) return false;
  return keyAlreadyExists(ht, key, keyLen) ? true : false;
}

bool HashTable_valIn(HashTable * ht, void * key, U4 keyLen)
{
  if (!ht || !key) return false;
  // TODO: implement
  return false;
}

void HashTable_iterateTableVals(HashTable * ht, callbackFunction callBack, void * args)
{
  if (!ht || !callBack) return;

  hashLinkCallbackPackage package = {.func = callBack, .args = args};

  for (U4 i = 0; i < ht->size; i++)
  {
    List * list = &ht->table[i];
    List_iterateList(list, iterateValsHelper, &package);
  }
}

void HashTable_iterateTableKeys(HashTable * ht, callbackFunction callBack, void * args)
{
  if (!ht || !callBack) return;

  hashLinkCallbackPackage package = {.func = callBack, .args = args};

  for (U4 i = 0; i < ht->size; i++)
  {
    List * list = &ht->table[i];
    List_iterateList(list, iterateKeysHelper, &package);
  }
}

void HashTable_iterateTableKV(HashTable * ht, callbackFunction callBack, void * args)
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

static HashTableLink createHashLink(HashTable * ht, void * key, U4 keyLen, void * val, U4 valLen, FreeDataFunction freeDataFunc, FreeDataFunction freeKeyFunc)
{
  HashTableLink hashEntry;

  hashEntry.freeKeyFunc = freeKeyFunc;
  hashEntry.keyLen = keyLen;
  hashEntry.key = malloc(keyLen);
  memcpy(hashEntry.key, key, keyLen);

  hashEntry.passByVal = ht->passByVal;
  hashEntry.freeDataFunc = freeDataFunc;
  hashEntry.valLen = valLen;

  if (val && valLen)
  {
    if (ht->passByVal)
    {
      hashEntry.val = malloc(valLen);
      memcpy(hashEntry.val, val, valLen);
    }
    else
    {
      hashEntry.val = val;
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
    if (hashLink->freeKeyFunc)
    {
      hashLink->freeKeyFunc(hashLink->key);
    }
    else
    {
      free(hashLink->key);
    }
  }

  if (hashLink->val && hashLink->passByVal)
  {
    if (hashLink->freeDataFunc)
    {
      hashLink->freeDataFunc(hashLink->val);
    }
    else
    {
      free(hashLink->val);
    }
  }

  free(hashLink);
}

static void updateHashState(U4 * state)
{
  // update the state (default is 8 iterations)
  for (U4 i = 0; i < HASH_ROUNDS; i++)
  {
    // if the lowest bit is 1, shift and xor with feedback
    if (*state & 1)
    {
      *state = (*state >> 1) ^ FEEDBACK_VAL;
    }
    // else just shift
    else
    {
      *state = (*state >> 1);
    }
  }
}

static U4 hashGen(void * key, U4 keyLen)
{
  if (key == NULL || keyLen == 0) return 0;

  U4 state = HASH_SEED;
  U4 temp;
  U1 * charKey = (U1 *)key;
  for (int i = 0; i < keyLen; i++)
  {
    // update the hash state
    temp = ((U4)charKey[i]) << 24;
    state = ((state ^ temp) & 0xff000000) | (state & 0x00ffffff);
    updateHashState(&state);
  }

  return state;
}

static Link * keyAlreadyExists(HashTable * ht, void * key, U4 keyLen)
{
  U4 hash = hashGen(key, keyLen);
  U4 index = hash % ht->size;

  List * thisList = &(ht->table[index]);

  for(Link * link = thisList->head; link; link = link->next)
  {
    HashTableLink * hashLink = (HashTableLink *)(link->data);

    if (hashLink->keyLen != keyLen) continue;

    if (memcmp(key, hashLink->key, keyLen) == 0)
    {
      return link;
    }
  }

  return NULL;
}

static void iterateValsHelper(void * hashLink, void * args)
{
  HashTableLink * l = hashLink;
  hashLinkCallbackPackage * package = args;
  if (!package->func) return;

  package->func(l->val, package->args);
}

static void iterateKeysHelper(void * hashLink, void * args)
{
  HashTableLink * l = hashLink;
  hashLinkCallbackPackage * package = args;
  if (!package->func) return;

  package->func(l->key, package->args);
}

static void iterateKVHelper(void * hashLink, void * args)
{
  HashTableLink * l = hashLink;
  hashLinkCallbackPackage * package = args;
  if (!package->func) return;

  HashKVPair kv = {.key = l->key, .val = l->val};

  package->func(&kv, package->args);
}
