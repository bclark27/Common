#ifndef LIST_H_
#define LIST_H_

#include "Common.h"

/////////////
//  TYPES  //
/////////////

typedef struct Link
{
  void * data;
  struct Link * next;
  struct Link * prev;
} Link;

typedef struct List
{
  U4 len;
  struct Link * head;
  struct Link * tail;

  U4 dataLen;
  bool passByVal;
  CompareFunction compFunc;
  FreeDataFunction freeFunc;

} List;

typedef struct
{
  List * list;
  Link * curr;
} ListItr;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

List * List_listInit(U4 dataLen, bool passByVal, CompareFunction compFunc, FreeDataFunction freeFunc);
void List_listObjInit(List * list, U4 dataLen, bool passByVal, CompareFunction compFunc, FreeDataFunction freeFunc);
ListItr List_getItr(List * list);
void * List_getNextRef(ListItr * itr);
void * List_getNextVal(ListItr * itr);
void List_sortList(List * list);
void List_destroyList(List * list);
void List_clearList(List * list);
void List_queue(List * list, void * data);
void * List_dequeue(List * list);
void * List_getVal(List * list, U4 index);
void * List_getRef(List * list, U4 index);
void * List_removeItem(List * list, U4 index);
void List_deleteItem(List * list, U4 index);
void List_destroyLink(List * list, Link * link);
void List_destroyLinkAndData(List * list, Link * link);

#endif
