#include "MemoryPool.h"

///////////////
//  DEFINES  //
///////////////

#define DEFAULT_SECTION_PAGES       16

/////////////
//  TYPES  //
/////////////


typedef struct SectionMetaData
{
    struct SectionMetaData * nextSection;
    struct SectionMetaData * prevSection;
    size_t sectionSize;
    size_t chunkSize;
    U2 chunkMaxCount;
    U2 chunkCount;

    /*

    Chunks are here

    */


} SectionMetaData;

typedef struct ChunkMetaData
{
    SectionMetaData * nextSection; // null if no next srction
    SectionMetaData * mySection;
    void* nextChunk; // null if no next chunk

    /*
     * ------------------
     *
     * data is here
     * 
     * ------------------
     */

} ChunkMetaData;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

SectionMetaData * CreateSectionMetaData(size_t chunkSize, SectionMetaData * prev, SectionMetaData * next);
ChunkMetaData* TryAllocateChunk(SectionMetaData * section);
void SetNextSection(SectionMetaData * curr, SectionMetaData * next);
void ResetChunkData(SectionMetaData * section);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

MemoryPool * MemoryPool_init(size_t dataSize)
{
    MemoryPool * mp = malloc(sizeof(MemoryPool));
    mp->dataSize = dataSize;
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
    ChunkMetaData* allocatedChunk = NULL;
    while (curr && !allocatedChunk)
    {
        allocatedChunk = TryAllocateChunk(curr);
        curr = curr->nextSection;
    }
    
    if (!allocatedChunk)
    {
       if (mp->firstSection)
       {
            curr = CreateSectionMetaData(mp->dataSize, NULL, mp->firstSection);
            SetNextSection(curr, mp->firstSection);
            mp->firstSection = curr;
            allocatedChunk = TryAllocateChunk(curr);
       }
       else
       {
            curr = CreateSectionMetaData(mp->dataSize, NULL, NULL);
            allocatedChunk = TryAllocateChunk(curr);
            mp->firstSection = curr;
       }
    }
    
    if (initData)
    {
        void* dest = ((U1*)allocatedChunk) + sizeof(ChunkMetaData);
        memcpy(dest, initData, mp->dataSize);
        // printf("%d\n", *((int*)dest));
    }

    return MP_SUCCESS;
}

void* MemoryPool_InitIter(MemoryPool* mp)
{
    if (!mp || !mp->firstSection)
        return NULL;

    return ((U1*)mp->firstSection) + sizeof(SectionMetaData) + sizeof(ChunkMetaData);
}

void* MemoryPool_IterNext(void* currData)
{
    if (!currData)
        return NULL;
    
    ChunkMetaData* curr = (ChunkMetaData*)(currData - sizeof(ChunkMetaData));
    ChunkMetaData* next = curr->nextChunk;
    
    if (!next)
    {
        if (curr->nextSection)
        {
            next = (ChunkMetaData*)(((U1*)curr->nextSection) + sizeof(SectionMetaData));
        }
    }

    if (next)
        return ((U1*)next) + sizeof(ChunkMetaData);

    return NULL;
}

/////////////////////////
//  Private Functions  //
/////////////////////////

SectionMetaData * CreateSectionMetaData(size_t dataSize, SectionMetaData * prev, SectionMetaData * next)
{
    // Determine the system page size.
    size_t pageSize = (size_t)sysconf(_SC_PAGESIZE);
    if (pageSize == 0)
        pageSize = 4096;  // fallback



    size_t sectionSize = DEFAULT_SECTION_PAGES * pageSize;
    size_t chunkSize = dataSize + sizeof(ChunkMetaData);
    size_t smallestSectionSize = sizeof(SectionMetaData) + chunkSize;

    if (sectionSize < smallestSectionSize)
        sectionSize = smallestSectionSize;

    SectionMetaData* section = NULL;
    int ret = posix_memalign((void**)&section, pageSize, sectionSize);
    if (ret != 0)
        return NULL;

    section->chunkMaxCount = (sectionSize - sizeof(SectionMetaData)) / chunkSize;
    section->chunkSize = chunkSize;
    section->sectionSize = sectionSize;
    section->nextSection = next;
    section->prevSection = prev;

    ResetChunkData(section);

    return section;
}

ChunkMetaData* TryAllocateChunk(SectionMetaData * section)
{
    if (!section || section->chunkCount >= section->chunkMaxCount)
        return NULL;

    section->chunkCount++;
    ResetChunkData(section);
    
    if (section->chunkCount == 1)
        ResetChunkData(section->prevSection);
    
    return (ChunkMetaData*)(((U1*)section) + sizeof(SectionMetaData) + ((section->chunkCount - 1) * section->chunkSize));
}

void SetNextSection(SectionMetaData * curr, SectionMetaData * next)
{
    next->prevSection = curr;
    curr->nextSection = next;
    ResetChunkData(curr);
}

void ResetChunkData(SectionMetaData * section)
{
    if (!section)
        return;

    U1* baseChunkPtr = ((U1*)section) + sizeof(SectionMetaData);
    for (int i = 0; i < section->chunkMaxCount; i++)
    {
        ChunkMetaData* c = (ChunkMetaData*)(baseChunkPtr + (i * section->chunkSize));
        ChunkMetaData* c_n = (ChunkMetaData*)(baseChunkPtr + ((i + 1) * section->chunkSize));
        
        if (!section->nextSection)
        {
            c->nextSection = NULL;
        }
        else
        {
            c->nextSection = section->nextSection->chunkCount ? section->nextSection : NULL;
        }
        
        c->mySection = section;

        if (i == section->chunkMaxCount - 1 ||
            i >= section->chunkCount - 1)
        {
            c->nextChunk = NULL;
        }
        else
        {
            c->nextChunk = c_n;
        }
    }
}