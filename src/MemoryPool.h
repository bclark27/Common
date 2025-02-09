#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_


#include "Common.h"

#include "EntityProperty.h"
#include "List.h"

/////////////
//  TYPES  //
/////////////

typedef struct MemoryPool
{
    size_t dataSize;
    void* firstSection;
} MemoryPool;


/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

// init the pool. elements in the pool will all be of size dataSize
MemoryPool * MemoryPool_init(size_t dataSize);

// frees the pool
void MemoryPool_free(MemoryPool * mp);

// allocates memory for a new item. returns success or error code
int MemoryPool_AddItem(MemoryPool* mp);
int MemoryPool_AddItemInitialData(MemoryPool* mp, void* initData);

// using this pointer to the item in the pool, remove it. if it doesnt exist then do nothing or return error code
int MemoryPool_RemoveItem(MemoryPool* mp, void* item);

// returns a pointer to the first data. send this pointer into the iternext function to get the next
void* MemoryPool_InitIter(MemoryPool* mp);

// gets the pointer to the next item in the pool. returns null when no more tiems in the pool
void*  MemoryPool_IterNext(void* currData);

/*
 * Return Codes
 */
#define MP_SUCCESS 0
#define MP_ERR_MEMPOOL_NULL -1

#endif