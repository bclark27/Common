#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "List.h"

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static Link * createLink();
static void freeLink(Link * link);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

List * List_listInit()
{
  List * list = malloc(sizeof(List));
  if(!list) return NULL;
  memset(list, 0, sizeof(List));
  return list;
}

void List_listObjInit(List * list)
{
  if(!list) return;
  memset(list, 0, sizeof(List));
}

void * List_findByProperty(List * list, void * property, CompareFunction compFunc)
{
  if(!list || list->len == 0) return NULL;

  for(Link * link = list->head; link; link = link->next)
  {
    if(compFunc(property, link->data) == 0)
    {
      return link->data;
    }
  }

  return NULL;
}

void List_sortList(List * list, CompareFunction compFunc)
{

}

void List_destroyList(List * list, FreeDataFunction freeFunc)
{
  /*
  if(!list) return;
  Link * link = list->head;
  Link * nextLink = list->head;
  while(link)
  {
    nextLink = link->next;
    _destroyLink(list, link, freeFunc);
    link = nextLink;
  }
  free(list);
  */
  List_clearList(list, freeFunc);
  free(list);
}

void List_destroyListDynamic(List * list)
{
  if(!list) return;
  List_destroyList(list, NULL);
}

void List_clearList(List * list, FreeDataFunction freeFunc)
{
  if(!list) return;
  Link * link = list->head;
  Link * nextLink = list->head;
  while(link)
  {
    nextLink = link->next;
    _destroyLink(list, link, freeFunc);
    link = nextLink;
  }

  memset(list, 0, sizeof(List));
}

void List_clearListDynamic(List * list)
{
  if(!list) return;
  List_clearList(list, NULL);
}

char List_queue(List * list, void * data)
{
  if(!list) return 0;

  Link * link = createLink();
  if(!link)
  {
    return 0;
  }

  link->data = data;
  if(list->head)
  {
    list->head->prev = link;
    link->next = list->head;
    link->prev = NULL;
    list->head = link;
  }
  else
  {
    list->head = link;
    list->tail = link;

    link->prev = NULL;
    link->next = NULL;
  }
  list->len++;

  return 1;
}

void * List_dequeue(List * list)
{
  if(!list || !list->tail) return NULL;

  void * data = list->tail->data;

  _destroyLinkDynamic(list, list->tail);

  return data;
}

void List_deleteItem(List * list, void * data, FreeDataFunction freeFunc)
{
  if(!list || data == NULL) return;
  for(Link * link = list->head; link; link = link->next)
  {
    if(link->data == data)
    {
      _destroyLink(list, link, freeFunc);
      return;
    }
  }
}

void List_deleteItemDynamic(List * list, void * data)
{
  if(!list) return;
  for(Link * link = list->head; link; link = link->next)
  {
    if(link->data == data)
    {
      _destroyLink(list, link, NULL);
      return;
    }
  }
}

char List_inList(List * list, void * data)
{
  if(!list) return 0;
  for(Link * link = list->head; link; link = link->next)
  {
    if(link->data == data)
    {
      return 1;
    }
  }
  return 0;
}

void _destroyLink(List * list, Link * link, FreeDataFunction freeFunc)
{
  if(link->prev == NULL && link->next == NULL)
  {
    list->head = NULL;
    list->tail = NULL;
  }
  else if(link->prev == NULL)
  {
    list->head->next->prev = NULL;
    list->head = list->head->next;
  }
  else if(link->next == NULL)
  {
    list->tail->prev->next = NULL;
    list->tail = list->tail->prev;
  }
  else
  {
    link->prev->next = link->next;
    link->next->prev = link->prev;
  }

  list->len--;
  if(freeFunc)
  {
    freeFunc(link->data);
  }

  freeLink(link);
}

void _destroyLinkDynamic(List * list, Link * link)
{
  _destroyLink(list, link, NULL);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static Link * createLink()
{
  Link * link = malloc(sizeof(Link));
  memset(link, 0, sizeof(Link));
  return link;
}

static void freeLink(Link * link)
{
  free(link);
}