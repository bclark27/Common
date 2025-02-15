#include "MemoryPool.h"

///////////////
//  DEFINES  //
///////////////

#define SECTION_ALIGNMENT                       (L2_CACHE_SIZE)
#define SECTION_SIZE                            (SECTION_ALIGNMENT)
#define SECTION_PTR_MASK                        (~((U8)(SECTION_SIZE - 1)))
#define MIN_OBJ_SIZE                            4
#define NO_METADATA_MAX_OBJ_COUNT               (SECTION_SIZE / MIN_OBJ_SIZE)
#define IN_USE_BIT_MASK_SIZE                    ((NO_METADATA_MAX_OBJ_COUNT / 8) + 1)
#define SECTION_METADATA_STRUCT_SIZE            (sizeof(SectionMetaData*) + sizeof(SectionMetaData*) + sizeof(U4) + sizeof(U4))
#define SECTION_DATA_SIZE                       (SECTION_SIZE - SECTION_METADATA_STRUCT_SIZE)
#define MAX_OBJ_SIZE                            SECTION_DATA_SIZE

#define SECTION_OBJ_COUNT(objSize)              (SECTION_DATA_SIZE / (objSize))
#define GET_OBJ_PTR(sectionPtr, objSize, idx)   (((U1*)(sectionPtr)) + SECTION_METADATA_STRUCT_SIZE + (objSize) * (idx))
#define GET_OBJ_SECTION(objPtr)                 ((void*)((U8)(objPtr) & (U8)SECTION_PTR_MASK))
#define GET_OBJ_IDX(objPtr, objSize)            (((void*)(objPtr) - ((void*)GET_OBJ_SECTION(objPtr) + SECTION_METADATA_STRUCT_SIZE)) / (objSize))

/////////////
//  TYPES  //
/////////////


typedef struct SectionMetaData
{
    struct SectionMetaData * next;
    struct SectionMetaData * prev;
    U4 inUseCount;

    /*

    objs are here

    */

} SectionMetaData;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

SectionMetaData * CreateSectionMetaData(size_t objSize);
void* TryAllocateChunk(MemoryPool* mp, SectionMetaData* section);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

MemoryPool * MemoryPool_init(size_t objSize)
{
    if (objSize > SECTION_SIZE)
        return NULL;

    MemoryPool * mp = malloc(sizeof(MemoryPool));
    mp->objSize = objSize;
    mp->firstSection = NULL;
    return mp;
}

int MemoryPool_AddItem(MemoryPool* mp)
{
    return MemoryPool_AddItemInitialData(mp, NULL);
}

int MemoryPool_AddItemInitialData(MemoryPool* mp, void* initData)
{
    if (!mp)
        return MP_ERR_MEMPOOL_NULL;

    SectionMetaData* curr = mp->firstSection;
    void* allocatedObj = NULL;
    while (curr && !allocatedObj)
    {
        allocatedObj = TryAllocateChunk(mp, curr);
        curr = curr->next;
    }
    
    if (!allocatedObj)
    {
        curr = CreateSectionMetaData(mp->objSize);
        allocatedObj = TryAllocateChunk(mp, curr);

        if (mp->firstSection)
        {
            SectionMetaData* prevFirst = mp->firstSection;
            prevFirst->prev = curr;
            curr->next = prevFirst;
            mp->firstSection = curr;
        }
        else
        {
            mp->firstSection = curr;
        }
    }
    
    if (initData)
        memcpy(allocatedObj, initData, mp->objSize);
    
    return MP_SUCCESS;
}

int MemoryPool_RemoveItem(MemoryPool* mp, void* item)
{
    if (!mp)
        return MP_ERR_MEMPOOL_NULL;

    if (!item)
        return MP_ERR_MEMPOOL_NULL;

    SectionMetaData* section = GET_OBJ_SECTION(item);

    bool found = false;
    SectionMetaData* curr = mp->firstSection;
    while (curr)
    {
        found = curr == section;
        if (found)
            break;
        curr = curr->next;
    }

    if (!found)
        return MP_ERR_ITEM_NOT_IN_MEMPOOL;

    U4 itemIdx = GET_OBJ_IDX(item, mp->objSize);
    if (itemIdx >= section->inUseCount)
        return MP_ERR_ITEM_NOT_IN_MEMPOOL;

    U4 bytesToMoveDown = (section->inUseCount - (itemIdx + 1)) * mp->objSize;
    U1* data = (U1*)item;
    for (U4 i = 0; i < bytesToMoveDown; i++)
        data[i] = data[i + mp->objSize];
    
    section->inUseCount--;
}

void MemoryPool_Iter(MemoryPool* mp, bool (*process)(void *))
{
    if (!mp)
        return;

    size_t s = mp->objSize;
    SectionMetaData* section = mp->firstSection;
    while (section)
    {
        void* currData = SECTION_METADATA_STRUCT_SIZE + ((void*)section);
        for (U4 i = 0; i < section->inUseCount; i++)
        {
            bool keepGoing = process(currData);
            if (!keepGoing)
                return;

            currData += s;
        }
        section = section->next;
    }
}

/////////////////////////
//  Private Functions  //
/////////////////////////

SectionMetaData * CreateSectionMetaData(size_t objSize)
{
    SectionMetaData* section = NULL;
    int ret = posix_memalign((void**)&section, SECTION_SIZE, SECTION_SIZE);
    if (ret != 0)
        return NULL;

    section->next = NULL;
    section->prev = NULL;
    section->inUseCount = 0;
    
    return section;
}

void* TryAllocateChunk(MemoryPool* mp, SectionMetaData* section)
{
    U4 currentlyInUse = section->inUseCount;
    U4 maxCount = SECTION_OBJ_COUNT(mp->objSize);

    if (currentlyInUse >= maxCount)
    return NULL;

    section->inUseCount++;
    return GET_OBJ_PTR(section, mp->objSize, section->inUseCount - 1);
}