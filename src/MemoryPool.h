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
    size_t objSize;
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

// iter over all the elements
void MemoryPool_Iter(MemoryPool* mp, bool (*process)(void *));

/*
 * Return Codes
 */
#define MP_SUCCESS 0
#define MP_ERR_MEMPOOL_NULL -1
#define MP_ERR_ITEM_NULL -2
#define MP_ERR_ITEM_NOT_IN_MEMPOOL -3

#endif