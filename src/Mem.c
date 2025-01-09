#include "stdio.h"
#include "string.h"
#include "stdbool.h"

#include "Mem.h"

///////////////
//  DEFINES  //
///////////////

#undef malloc
#undef calloc
#undef realloc
#undef free

#define AFTER_VAL           ((unsigned int)0xbeefface)

#define getAfterPtr(p, s)   ((void *)(((unsigned char *)p) + s))
#define getAfterVal(p, s)   (*(unsigned int*)getAfterPtr(p, s))
#define totalMemSize(s)     (s + sizeof(AFTER_VAL))

#define UNTRACKED_FLAG      0x1
#define DOUBLE_FREE_FLAG    0x2
#define MEM_OVERFLOW_FLAG   0x4

/////////////
//  TYPES  //
/////////////

typedef struct
{
  struct MemTrackerNode * next;
  struct MemTrackerNode * prev;

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

static void * getNode(void * addr) __attribute__((hot));
static bool addAlloc(void * addr, size_t size, const char * fileName, int lineNum) __attribute__((hot));
static unsigned char removeAlloc(void * addr, const char * fileName, int lineNum) __attribute__((hot));
static void printRemoveAllocFlags(unsigned char flags);
static void printNode(MemTrackerNode * node);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

void mem_init(void)
{
  memset(&memTracker, 0, sizeof(MemTracker));
  memTracker.head = NULL;
  printf("===Mem Tracker Init\n");
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
      if (!unfree) printf("Unfreed Memory: %zu bytes in use\n", memTracker.userBytesInUse);
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

  addAlloc(addr, callocSize, fileName, lineNum);

  return addr;
}

void * mem_realloc(void *ptr, size_t size, const char * fileName, int lineNum)
{
  MemTrackerNode * this = getNode(ptr);
  void * newPtr = realloc(ptr, totalMemSize(size));

  if (this)
  {
    size_t preSize;
    this->allocCount = 1;
    this->fileName = fileName;
    this->lineNum = lineNum;
    this->mem = newPtr;
    this->totalSize = totalMemSize(size);
    preSize = this->userMemSize;
    this->userMemSize = size;
    *(unsigned int *)getAfterPtr(newPtr, size) = AFTER_VAL;
    memTracker.userBytesInUse += size - preSize;
  }
  else
  {
    addAlloc(newPtr, size, fileName, lineNum);
  }

  return newPtr;
}

void mem_free(void * ptr, const char * fileName, int lineNum)
{
  /*
  MemTrackerNode * this = getNode(ptr);

  if (!this)
  {
    printf("===Untracked memory freed at addr %p, freed at: %s on line %d\n\n", ptr, fileName, lineNum);
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
  this->mem = NULL;
  */

  unsigned char flags = removeAlloc(ptr, fileName, lineNum);

  if (!flags)
  {
    free(ptr);
    return;
  }

  printf("===Free Error: %s on line %d\n", fileName, lineNum);
  printRemoveAllocFlags(flags);

  free(ptr);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static void * getNode(void * addr)
{
  MemTrackerNode * this = (MemTrackerNode *)memTracker.head;
  while (this && (this->mem != addr))
  {
    this = (MemTrackerNode *)this->next;
  }

  return this;
}

static bool addAlloc(void * addr, size_t size, const char * fileName, int lineNum)
{
  MemTrackerNode * this = getNode(addr);

  if (!this)
  {
    do
    {
      this = malloc(sizeof(MemTrackerNode));
    } while (this == NULL);

    memset(this, 0, sizeof(MemTrackerNode));
  }

  size_t preSize;

  this->allocCount = 1;
  this->fileName = fileName;
  this->lineNum = lineNum;
  this->mem = addr;
  this->totalSize = totalMemSize(size);
  preSize = this->userMemSize;
  this->userMemSize = size;
  *(unsigned int *)getAfterPtr(addr, size) = AFTER_VAL;

  memTracker.userBytesInUse += size - preSize;
  memTracker.numActiveMallocs++;

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

  return true;
}

static unsigned char removeAlloc(void * addr, const char * fileName, int lineNum)
{
  unsigned char flags = 0;
  MemTrackerNode * this = getNode(addr);

  if (!this)
  {
    flags |= UNTRACKED_FLAG;
    return flags;
  }

  this->allocCount--;

  if (this->allocCount < 0)
  {
    flags |= DOUBLE_FREE_FLAG;
  }

  if (getAfterVal(this->mem, this->userMemSize) != AFTER_VAL)
  {
    flags |= MEM_OVERFLOW_FLAG;
  }

  memTracker.numActiveMallocs--;
  memTracker.userBytesInUse -= this->userMemSize;

  if (this == (MemTrackerNode *)memTracker.head)
  {
    memTracker.head = this->next;
  }
  else
  {
    MemTrackerNode * p = (MemTrackerNode *)this->prev;
    MemTrackerNode * n = (MemTrackerNode *)this->next;

    if (n)
    {
      p->next = (struct MemTrackerNode *)n;
      n->prev = (struct MemTrackerNode *)p;
    }
    else
    {
      p->next = NULL;
    }
  }

  free(this);

  return flags;
}

static void printRemoveAllocFlags(unsigned char flags)
{
  if (flags & UNTRACKED_FLAG) printf("| Untracked memory freed\n");
  if (flags & DOUBLE_FREE_FLAG) printf("| Double free\n");
  if (flags & MEM_OVERFLOW_FLAG) printf("| Memory overflow\n");
}

static void printNode(MemTrackerNode * node)
{
  printf("| Memory Block:\n");
  printf("| Addr: %p\n", node->mem);
  printf("| Size: %zu\n", node->userMemSize);
  printf("| Alloc Count: %d\n", node->allocCount);
  printf("| Alloced at: %s on line %d\n| \n", node->fileName, node->lineNum);
}
