#include <stdio.h>
#include "Common.h"
#include "CrossComm.h"
#include "MemoryPool.h"
#include "Entity.h"

// test comment

bool removeSome(void* data, void* state)
{
    int num = ((int*)data)[0];
    if (num % 1000 == 0)
        return 1;
    return 0;
}
bool addAll(void* data, void* state)
{
    ((U4*)state)[0] += ((int*)data)[0];
    return 0;
}
bool addAllBatch(void* data, void* state, U4 count, U4 size)
{
    void* curr = data;
    void* end = curr + (count * size);
    for (; data < end; data += size)
    {
        ((U4*)state)[0]++;

        int num = ((int*)data)[0];
    }

    return 0;
}

//#define USE_POOL
#define USE_BATCH

int main()
{
    clock_t start, end;
    int iterCount = 1000;
    int dataCount = 1000000;
    int dataSize = 80;
    U4 state;


#ifndef USE_POOL
    void* allData[dataCount];
    for (int i = 0; i < dataCount; i++)
    {
        // const int okok = 100;
        // void* tmps[okok];
        // for (int k = 0; k < okok; k++)
        //     tmps[k] = malloc(dataSize);
        allData[i] = malloc(dataSize);
    }

    start = clock();
    for (int k = 0; k < iterCount; k++)
    {
#ifdef USE_BATCH
        unsigned int batchSize = 60000;
        unsigned int inc = batchSize * dataSize;
        void* curr = allData;
        void* end = curr + (dataCount * dataSize);
        while (curr < end)
        {
            addAllBatch(curr, &state, MIN(batchSize, (end - curr) / dataSize), dataSize);
            curr += inc;
        }

#else
        for (int i = 0; i < dataCount; i++)
            addAll(allData[i], &state);
#endif
        
    }
    end = clock();
#else
    MemoryPool* mp = MemoryPool_init(dataSize);
    for (int i = 0; i < dataCount; i++)
    {
        MemoryPool_AddItemInitialData(mp, &i);
    }

    start = clock();
    for (int i = 0; i < iterCount; i++)
    {
#ifdef USE_BATCH
        MemoryPool_IterBatch(mp, addAllBatch, &state);
#else
        MemoryPool_Iter(mp, addAll, &state);
#endif

    }
    end = clock();
    // printf("%d\n", *(int*)MemoryPool_GetCurrentItemPtr(mp, 123));
    // printf("%d\n", *(int*)MemoryPool_GetCurrentItemPtr(mp, 124));
    // printf("%d\n", *(int*)MemoryPool_GetCurrentItemPtr(mp, 125));
#endif

    printf("TIME: %lf\n%d\n", (double)(end - start) / CLOCKS_PER_SEC, state);

    return 0;
}



