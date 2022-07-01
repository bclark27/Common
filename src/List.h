#ifndef LIST_H_
#define LIST_H_

#include "Standard.h"

/////////////
//  TYPES  //
/////////////

typedef void (* FreeDataFunction)(void *);
typedef char (* CompareFunction)(void *, void *);

typedef struct Link
{
  void * data;
  struct Link * next;
  struct Link * prev;
} Link;

typedef struct List
{
  unsigned int len;
  struct Link * head;
  struct Link * tail;
} List;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

List * List_listInit(void);
void List_listObjInit(List * list);

void * List_findByProperty(List * list, void * property, CompareFunction compFunc);

void List_sortList(List * list, CompareFunction compFunc);

void List_destroyList(List * list, FreeDataFunction freeFunc);
void List_destroyListDynamic(List * list);

void List_clearList(List * list, FreeDataFunction freeFunc);
void List_clearListDynamic(List * list);

char List_queue(List * list, void * data);
void * List_dequeue(List * list);

void List_deleteItem(List * list, void * data, FreeDataFunction freeFunc);
void List_deleteItemDynamic(List * list, void * data);

char List_inList(List * list, void * data);

void _destroyLink(List * list, Link * link, FreeDataFunction freeFunc);
void _destroyLinkDynamic(List * list, Link * link);

#endif
