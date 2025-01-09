#include "Heap.h"

///////////////
//  DEFINES  //
///////////////

#define RIGHT_IDX(i)    (2 * (i) + 2)
#define LEFT_IDX(i)     (2 * (i) + 1)
#define HEAP_IDX(h, i)  (((Heap*)(h))->elementSize * (i))
#define META_ADDR(a)    (a)
#define DATA_ADDR(a)    ((a) + sizeof(MetaData))

/////////////
//  TYPES  //
/////////////

typedef struct MetaData
{
  bool isNull;
} MetaData;

////////////////////
//  PRIVATE VARS  //
////////////////////

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

void freeNodesDataRefs(Heap * h, U4 rootIdx);
U4 getRightChildIdx(Heap * h, U4 rootIdx);
U4 getLeftChildIdx(Heap * h, U4 rootIdx);

//////////////////////
// PUBLIC FUNCTIONS //
//////////////////////

Heap * Heap_init(bool isMaxHeap, U4 dataLen, bool passByVal, CompareFunction compFunc, FreeDataFunction freeFunc)
{
  Heap * h = calloc(1, sizeof(Heap));

  h->elementSize = dataLen + sizeof(MetaData);
  h->heapSpaceBytes = h->elementSize * HEAP_DEFAULT_SPACE;

  h->heapSpace = calloc(HEAP_DEFAULT_SPACE, h->elementSize);

  h->compFunc = compFunc;
  h->freeFunc = freeFunc;
  h->dataLen = dataLen;

  h->passByVal = passByVal;
  h->isMaxHeap = isMaxHeap;

  return h;
}

void Heap_free(Heap * h)
{
  if (!h) return;

  if (!h->passByVal) freeNodesDataRefs(h, 0);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

void freeNodesDataRefs(Heap * h, U4 rootIdx)
{
  U4 heapIdx = HEAP_IDX(h, rootIdx);
  if (heapIdx >= h->heapSpaceBytes) return;

  void * nodeAddr = h->heapSpace + heapIdx;

  MetaData * meta = META_ADDR(nodeAddr);
  if (meta->isNull) return;

  if(h->freeFunc) h->freeFunc(DATA_ADDR(nodeAddr));
  else free(DATA_ADDR(nodeAddr));

  freeNodesDataRefs(h, RIGHT_IDX(rootIdx));
  freeNodesDataRefs(h, LEFT_IDX(rootIdx));
}

/* returns idx of right, -1 if child is null */
U4 getRightChildIdx(Heap * h, U4 rootIdx)
{
  U4 right = RIGHT_IDX(rootIdx);
  U4 heapIdx = HEAP_IDX(h, right);

  if (heapIdx >= h->heapSpaceBytes) return -1;

  MetaData * meta = h->heapSpace + heapIdx;

  if (meta->isNull) return -1;

  return right;
}

/* returns idx of right, -1 if child is null */
U4 getLeftChildIdx(Heap * h, U4 rootIdx)
{
  U4 left = LEFT_IDX(rootIdx);
  U4 heapIdx = HEAP_IDX(h, left);

  if (heapIdx >= h->heapSpaceBytes) return -1;

  MetaData * meta = h->heapSpace + heapIdx;

  if (meta->isNull) return -1;

  return left;
}
