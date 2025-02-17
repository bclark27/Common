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
#define SECTION_DATA_SIZE                       (SECTION_SIZE - sizeof(Section))
#define MAX_OBJ_SIZE                            SECTION_DATA_SIZE

#define SECTION_OBJ_COUNT(objSize)                      (SECTION_DATA_SIZE / (objSize))
#define GET_OBJ_PTR(sectionPtr, objSize, idx)           (((U1*)(sectionPtr)) + sizeof(Section) + (objSize) * (idx))
#define GET_OBJ_SECTION(objPtr)                         ((void*)((U8)(objPtr) & (U8)SECTION_PTR_MASK))
#define GET_OBJ_IDX(objPtr, objSize)                    (((void*)(objPtr) - ((void*)GET_OBJ_SECTION(objPtr) + sizeof(Section))) / (objSize))
#define SECTION_AVAILABLE_COUNT(sectionPtr, objSize)    (SECTION_OBJ_COUNT(objSize) - ((Section*)(sectionPtr))->inUseCount)
#define SECTION_IS_FULL(sectionPtr, objSize)            (SECTION_AVAILABLE_COUNT(sectionPtr, objSize) == 0)
#define SECTION_IS_EMPTY(sectionPtr)                    (((Section*)(sectionPtr))->inUseCount == 0)

/////////////
//  TYPES  //
/////////////


typedef struct Section
{
    struct Section * next;
    struct Section * prev;
    U4 inUseCount;

    /*

    objs are here

    */

} Section;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

Section * CreateSection(U4 objSize);
void* TryAllocateChunk(MemoryPool* mp, Section* section);
void ConsolodateSections(MemoryPool* mp, Section* a, Section* b);
void RemoveSectionFromList(MemoryPool* mp, Section* section);
void FullDefragmentation(MemoryPool* mp);

static inline void DeleteSectionItem(Section* section, U4 itemIdx, U4 objSize)
{
    U4 bytesToMoveDown = (section->inUseCount - (itemIdx + 1)) * objSize;
    U1* data = GET_OBJ_PTR(section, objSize, itemIdx);
    for (U4 i = 0; i < bytesToMoveDown; i++)
        data[i] = data[i + objSize];
    section->inUseCount--;
}

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

MemoryPool * MemoryPool_init(U4 objSize)
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

    Section* curr = mp->firstSection;
    void* allocatedObj = NULL;
    while (curr && !allocatedObj)
    {
        allocatedObj = TryAllocateChunk(mp, curr);
        curr = curr->next;
    }
    
    if (!allocatedObj)
    {
        curr = CreateSection(mp->objSize);
        allocatedObj = TryAllocateChunk(mp, curr);

        if (mp->firstSection)
        {
            Section* prevFirst = mp->firstSection;
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

void* MemoryPool_GetCurrentItemPtr(MemoryPool* mp, U4 idx)
{
    if (!mp)
        return NULL;

    U4 i = 0;
    Section* curr = mp->firstSection;
    while (curr)
    {
        if (i <= idx &&
            idx < i + curr->inUseCount)
            break;
        
        i += curr->inUseCount;
        curr = curr->next;
    }

    if (curr == NULL)
        return NULL;

    return GET_OBJ_PTR(curr, mp->objSize, idx - i);
}

int MemoryPool_RemoveItem_Ptr(MemoryPool* mp, void* item)
{
    if (!mp)
        return MP_ERR_MEMPOOL_NULL;

    if (!item)
        return MP_ERR_MEMPOOL_NULL;

    Section* section = GET_OBJ_SECTION(item);

    bool found = false;
    Section* curr = mp->firstSection;
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

    DeleteSectionItem(section, itemIdx, mp->objSize);

    return MP_SUCCESS;
}

int MemoryPool_RemoveItem_Idx(MemoryPool* mp, U4 idx)
{
    if (!mp)
        return MP_ERR_MEMPOOL_NULL;

    return MemoryPool_RemoveItem_Ptr(mp, MemoryPool_GetCurrentItemPtr(mp, idx));
}

void MemoryPool_Iter(MemoryPool* mp, MemoryPool_IterCallback process, void * state)
{
    if (!mp)
        return;

    U4 objSize = mp->objSize;
    Section* section = mp->firstSection;
    while (section)
    {
        void* currObj = sizeof(Section) + ((void*)section);
        void* endObj = currObj + (section->inUseCount * objSize);
        while (currObj < endObj)
        {
            if (process(currObj, state)) return;
            currObj += objSize;
        }
        section = section->next;
    }
}
void MemoryPool_IterBatch(MemoryPool* mp, MemoryPool_IterBatchCallback process, void* state)
{
    if (!mp)
        return;

    U4 objSize = mp->objSize;
    Section* section = mp->firstSection;
    while (section)
    {
        if (process(
            sizeof(Section) + ((void*)section), 
            state, 
            section->inUseCount, 
            objSize)) 
            return;

        section = section->next;
    }
}

void MemoryPool_Defragment(MemoryPool* mp)
{
    FullDefragmentation(mp);
}

void MemoryPool_GetStats(MemoryPool* mp, MemoryPoolStats* stats)
{
    if (!mp || !stats)
        return;

    memset(stats, 0, sizeof(MemoryPoolStats));

    stats->objSize = mp->objSize;
    stats->maxObjPerSection = SECTION_OBJ_COUNT(mp->objSize);

    Section* curr = mp->firstSection;
    while (curr)
    {
        stats->sectionCount++;

        if (SECTION_IS_FULL(curr, mp->objSize))
            stats->fullSectionCount++;
        if (SECTION_IS_EMPTY(curr))
            stats->emptySectionCount++;

        stats->totalObjInUseCount += curr->inUseCount;
        stats->totalObjAvailableCount += stats->maxObjPerSection - curr->inUseCount;
        curr = curr->next;
    }

    stats->totalMemEfficiency = (double)(stats->totalObjInUseCount * stats->objSize) / (double)(stats->sectionCount * SECTION_SIZE);
}

/////////////////////////
//  Private Functions  //
/////////////////////////

Section * CreateSection(U4 objSize)
{
    Section* section = NULL;
    int ret = posix_memalign((void**)&section, SECTION_SIZE, SECTION_SIZE);
    if (ret != 0)
        return NULL;

    section->next = NULL;
    section->prev = NULL;
    section->inUseCount = 0;
    
    return section;
}

void* TryAllocateChunk(MemoryPool* mp, Section* section)
{
    U4 currentlyInUse = section->inUseCount;
    U4 maxCount = SECTION_OBJ_COUNT(mp->objSize);

    if (currentlyInUse >= maxCount)
    return NULL;

    section->inUseCount++;
    return GET_OBJ_PTR(section, mp->objSize, section->inUseCount - 1);
}

void ConsolodateSections(MemoryPool* mp, Section* a, Section* b)
{
    if (!a || !mp)
        return;

    U4 availableSpaceInA = SECTION_AVAILABLE_COUNT(a, mp->objSize);
    if (!b || !availableSpaceInA || SECTION_IS_EMPTY(b))
        return;

    U4 elementsToMove = MIN(availableSpaceInA, b->inUseCount);
    void* dest = GET_OBJ_PTR(a, mp->objSize, a->inUseCount);
    void* src = GET_OBJ_PTR(b, mp->objSize, b->inUseCount - elementsToMove);
    
    memcpy(dest, src, mp->objSize * elementsToMove);

    a->inUseCount += elementsToMove;
    b->inUseCount -= elementsToMove;
}

void RemoveSectionFromList(MemoryPool* mp, Section* section)
{
    if (!section || !mp)
        return;

    Section* prev = section->prev;
    Section* next = section->next;

    section->prev = NULL;
    section->next = NULL;

    if (!prev && !next)
    {
        mp->firstSection = NULL;
        return;
    }

    if (!prev)
    {
        mp->firstSection = next;
        next->prev = NULL;
        return;
    }

    if (!next)
    {
        prev->next = NULL;
        return;
    }

    prev->next = next;
    next->prev = prev;
}

void FullDefragmentation(MemoryPool* mp)
{
    Section* curr = mp->firstSection;

    while (curr)
    {
        while (!SECTION_IS_FULL(curr, mp->objSize) && curr->next)
        {
            ConsolodateSections(mp, curr, curr->next);
            if (SECTION_IS_EMPTY(curr->next))
            {
                Section* empty = curr->next;
                RemoveSectionFromList(mp, empty);
                free(empty);
            }
        }

        curr = curr->next;
    }
}