#include "HashTable.h"

///////////////
//  DEFINES  //
///////////////

#define DEFAULT_HASH_TABLE_LENGTH   256
#define HASH_SEED                   0x12fa
#define HASH_ROUNDS                 8
#define FEEDBACK_VAL                0x87654321
#define PAD_VAL                     0xc4

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

HashTableLink * createHashLink(HashTable * ht, unsigned char * key, unsigned int keyLen, void * data);
void freeHashLink(void *);
char compareHashTableLinks(void * item1, void * item2);
unsigned short hashGen(unsigned char * key, unsigned int keyLen);
void updateHashState(unsigned int * state);
Link * keyAlreadyExists(HashTable * ht, unsigned char * key, unsigned int keyLen);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

HashTable * HashTable_init(unsigned int dataLen, FreeDataFunction dataFreeFunc)
{
  HashTable * ht = (HashTable *)callocOrDie(1, sizeof(HashTable));
  ht->table = ArrayList_init(sizeof(List), DEFAULT_HASH_TABLE_LENGTH);
  ht->dataLen = dataLen;
  ht->dataFreeFunc = dataFreeFunc;

  for (int i = 0; i < DEFAULT_HASH_TABLE_LENGTH; i++)
  {
    List list;
    List_listObjInit(&list);
    ArrayList_append(ht->table, &list);
  }

  return ht;
}

void HashTable_free(HashTable * ht)
{
  for (int i = 0; i < DEFAULT_HASH_TABLE_LENGTH; i++)
  {
    List * list = ArrayList_get(ht->table, i);
    List_clearList(list, freeHashLink);
  }

  ArrayList_freeList(ht->table);
  free(ht);
}

char HashTable_insert(HashTable * ht, unsigned char * key, unsigned int keyLen, void * data)
{
  unsigned short hash = hashGen(key, keyLen);
  unsigned int index = hash % DEFAULT_HASH_TABLE_LENGTH;

  Link * link = keyAlreadyExists(ht, key, keyLen);
  List * thisList = (List *)ArrayList_get(ht->table, index);

  HashTableLink * entry = createHashLink(ht, key, keyLen, data);

  if (link)
  {
    _destroyLink(thisList, link, ht->dataFreeFunc);
  }

  List_queue(thisList, entry);

  return 1;
}

void * HashTable_get(HashTable * ht, unsigned char * key, unsigned int keyLen)
{
  Link * link = keyAlreadyExists(ht, key, keyLen);

  if (link)
  {
    HashTableLink * hashLink = (HashTableLink * )link->data;
    return hashLink->data;
  }
  return NULL;
}

void HashTable_remove(HashTable * ht, unsigned char * key, unsigned int keyLen)
{
  unsigned short hash = hashGen(key, keyLen);
  unsigned int index = hash % DEFAULT_HASH_TABLE_LENGTH;

  Link * link = keyAlreadyExists(ht, key, keyLen);

  if (link)
  {
    List * thisList = (List *)ArrayList_get(ht->table, index);
    _destroyLink(thisList, link, ht->dataFreeFunc);
  }
}

bool HashTable_in(HashTable * ht, unsigned char * key, unsigned int keyLen)
{
  return keyAlreadyExists(ht, key, keyLen) ? true : false;
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

HashTableLink * createHashLink(HashTable * ht, unsigned char * key, unsigned int keyLen, void * data)
{
  HashTableLink * hashEntry = (HashTableLink *)malloc(sizeof(HashTableLink));
  hashEntry->key = (unsigned char *)malloc(keyLen);
  hashEntry->data = malloc(ht->dataLen);
  memcpy(hashEntry->key, key, keyLen);
  memcpy(hashEntry->data, data, ht->dataLen);
  hashEntry->dataFreeFunc = ht->dataFreeFunc;
  hashEntry->keyLen = keyLen;

  return hashEntry;
}

void freeHashLink(void * ptr)
{
  HashTableLink * hashLink = (HashTableLink *)ptr;
  if (hashLink == NULL) return;
  if (hashLink->key) free(hashLink->key);
  if (hashLink->data && hashLink->dataFreeFunc) hashLink->dataFreeFunc(hashLink->data);
  free(hashLink);
}

unsigned short hashGen(unsigned char * key, unsigned int keyLen)
{
  if (key == NULL || keyLen == 0) return 0;

  unsigned int state = HASH_SEED;
  unsigned int temp;
  for (int i = 0; i < keyLen; i++)
  {
    // update the hash state
    temp = key[i] << 24;
    state = ((state ^ temp) & 0xff000000) | (state & 0x00ffffff);
    updateHashState(&state);
  }

  return state & 0xffff;
}

void updateHashState(unsigned int * state)
{
  // update the state (default is 8 iterations)
  for (int i = 0; i < HASH_ROUNDS; i++)
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

Link * keyAlreadyExists(HashTable * ht, unsigned char * key, unsigned int keyLen)
{
  unsigned short hash = hashGen(key, keyLen);
  unsigned int index = hash % DEFAULT_HASH_TABLE_LENGTH;

  List * thisList = (List *)ArrayList_get(ht->table, index);

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
