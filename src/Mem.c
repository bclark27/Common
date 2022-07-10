#include "stdio.h"
#include "string.h"
#include "stdbool.h"

#include "Mem.h"

///////////////
//  DEFINES  //
///////////////

#undef TRACK_MEM

#define AFTER_VAL     ((unsigned int)0xbeefface)

#define getAfterPtr(p, s)   ((void *)(((unsigned char *)p) + s))
#define getAfterVal(p, s)   (*(unsigned int*)getAfterPtr(p, s))
#define totalMemSize(s)     (s + sizeof(AFTER_VAL))


/////////////
//  TYPES  //
/////////////

typedef struct
{
  struct MemTrackerNode * prev;
  struct MemTrackerNode * next;

  const char * fileName;
  int lineNum;

  int allocCount;
  size_t totalSize;
  size_t userMemSize;
  void * mem;

} MemTrackerNode;

typedef struct
{
  struct MemTrackerNode * head;
  unsigned long numActiveMallocs;
  size_t userBytesInUse;
} MemTracker;

///////////////////
//  PUBLIC VARS  //
///////////////////

static MemTracker memTracker;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static void * getNode(void * addr);
static void printNode(MemTrackerNode * node);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

void mem_init(void)
{
  memset(&memTracker, 0, sizeof(MemTracker));
  memTracker.head = NULL;
}

void mem_freeTracker(void)
{
  printf("===Freeing Mem Tracker\n");
  bool unfree = false;
  MemTrackerNode * this = (MemTrackerNode *)memTracker.head;
  while (this)
  {
    MemTrackerNode * next = (MemTrackerNode *)this->next;
    if (this->allocCount != 0)
    {
      if (!unfree) printf("Unfreed Memory:\n");
      unfree = true;
      printNode(this);
      free(this->mem);
    }
    free(this);
    this = next;
  }
  printf("\n");
}

void mem_print(void)
{
  printf("===Mem Report\n");
  printf("Known Allocs in Use: %lu\n", memTracker.numActiveMallocs);
  printf("User Bytes In Use: %zu\n", memTracker.userBytesInUse);
  MemTrackerNode * this = (MemTrackerNode *)memTracker.head;
  while (this)
  {
    printNode(this);
    this = (MemTrackerNode *)this->next;
  }
  printf("\n");
}

void * mem_malloc(size_t size, const char * fileName, int lineNum)
{
  return mem_calloc(1, size, fileName, lineNum);
}

void * mem_calloc(size_t nmemb, size_t size, const char * fileName, int lineNum)
{
  size_t callocSize = nmemb * size;

  void * addr;
  do
  {
    addr = malloc(totalMemSize(callocSize));
  } while (!addr);

  memset(addr, 0, totalMemSize(callocSize));
  MemTrackerNode * this = getNode(addr);

  if (this)
  {
    this->allocCount++;
  }
  else
  {
    this = malloc(sizeof(MemTrackerNode));
    this->prev = NULL;
    this->next = NULL;
    this->allocCount = 1;
  }

  this->fileName = fileName;
  this->lineNum = lineNum;
  this->mem = addr;
  this->totalSize = totalMemSize(callocSize);
  this->userMemSize = callocSize;


  *(unsigned int *)getAfterPtr(addr, callocSize) = AFTER_VAL;

  if (!memTracker.head)
  {
    memTracker.head = (struct MemTrackerNode *)this;
  }
  else
  {
    MemTrackerNode * next = (MemTrackerNode *)memTracker.head;
    next->prev = (struct MemTrackerNode *)this;
    this->next = (struct MemTrackerNode *)next;
    memTracker.head = (struct MemTrackerNode *)this;
  }

  memTracker.numActiveMallocs++;
  memTracker.userBytesInUse += callocSize;

  return addr;
}
void mem_free(void * ptr, const char * fileName, int lineNum)
{
  MemTrackerNode * this = getNode(ptr);

  if (!this)
  {
    printf("===Untracked memory freed at addr %p, alloced at: %s on line %d\n\n", ptr, fileName, lineNum);
    free(ptr);
    return;
  }

  this->allocCount--;

  bool doubleFree = false;
  if (this->allocCount < 0)
  {
    doubleFree = true;
  }

  bool overflowErr = false;
  if (getAfterVal(this->mem, this->userMemSize) != AFTER_VAL)
  {
    overflowErr = true;
  }

  if (doubleFree || overflowErr)
  {
    printf("===Free Error(s):\n");
    printf("Error Location: %s on line %d\n", fileName, lineNum);
    if (doubleFree) printf("Double free\n");
    if (overflowErr) printf("Memory overflow\n");
    printNode(this);
  }

  memTracker.numActiveMallocs--;
  memTracker.userBytesInUse -= this->userMemSize;

  free(this->mem);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static void * getNode(void * addr)
{
  MemTrackerNode * this = (MemTrackerNode *)memTracker.head;
  while (this && this->mem != addr)
  {
    this = (MemTrackerNode *)this->next;
  }

  return this;
}

static void printNode(MemTrackerNode * node)
{
  printf("Memory Block:\n");
  printf("Addr: %p\n", node->mem);
  printf("Size: %zu\n", node->userMemSize);
  printf("Alloc Count: %d\n", node->allocCount);
  printf("Alloced at: %s on line %d\n\n", node->fileName, node->lineNum);
}
