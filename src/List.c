#include "Common.h"
#include "List.h"

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static Link * createLink(void);
static void deleteLinkData(List * list, Link * l);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

List * List_listInit(U4 dataLen, bool passByVal, CompareFunction compFunc, FreeDataFunction freeFunc)
{
  List * l = callocOrDie(1, sizeof(List));
  l->dataLen = dataLen;
  l->passByVal = passByVal;
  l->compFunc = compFunc;
  l->freeFunc = freeFunc;

  return l;
}

void List_listObjInit(List * list, U4 dataLen, bool passByVal, CompareFunction compFunc, FreeDataFunction freeFunc)
{
  memset(list, 0, sizeof(List));

  list->dataLen = dataLen;
  list->passByVal = passByVal;
  list->compFunc = compFunc;
  list->freeFunc = freeFunc;
}

ListItr List_getItr(List * list)
{
  if (!list) return (ListItr){.list = NULL, .curr = NULL};
  return (ListItr){.list = list, .curr = list->head};
}

void * List_getNextRef(ListItr * itr)
{
  if (!itr) return NULL;
  Link * l = itr->curr;

  if (!l) return NULL;

  itr->curr = (Link *)l->next;

  return l->data;
}

void * List_getNextVal(ListItr * itr)
{
  if (!itr) return NULL;
  Link * l = itr->curr;

  if (!l) return NULL;

  itr->curr = (Link *)l->next;

  void * data = mallocOrDie(itr->list->dataLen);
  memcpy(data, l->data, itr->list->dataLen);
  return data;
}

void List_sortList(List * list)
{

}

void List_destroyList(List * list)
{
  Link * l = list->head;

  while (l)
  {
    deleteLinkData(list, l);

    Link * next = (Link *)l->next;
    free(l);
    l = next;
  }

  free(list);
}

void List_clearList(List * list)
{
  Link * l = list->head;

  while (l)
  {

    if (list->passByVal && l->data)
    {
      if (list->freeFunc)
      {
        list->freeFunc(l->data);
      }
      else
      {
        free(l->data);
      }
    }

    Link * next = (Link *)l->next;
    free(l);
    l = next;
  }

  list->head = NULL;
  list->tail = NULL;
  list->len = 0;
}

void List_queue(List * list, void * data)
{
  Link * link = createLink();

  if (list->passByVal)
  {
    link->data = mallocOrDie(list->dataLen);
    memcpy(link->data, data, list->dataLen);
  }
  else
  {
    link->data = data;
  }

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
}

void * List_dequeue(List * list)
{
  if (!list || !list->tail) return NULL;

  void * data = list->tail->data;

  List_destroyLink(list, list->tail);

  return data;
}

void * List_getVal(List * list, U4 index)
{
  ListItr itr = List_getItr(list);
  void * data = List_getNextRef(&itr);
  U4 i = 0;

  while (i != index && data)
  {
    data = List_getNextRef(&itr);
    i++;
  }

  if (i != index) return NULL;

  void * ret = mallocOrDie(list->dataLen);
  memcpy(ret, data, list->dataLen);
  return ret;
}

void * List_getRef(List * list, U4 index)
{
  ListItr itr = List_getItr(list);
  void * data = List_getNextRef(&itr);
  U4 i = 0;

  while (i != index && data)
  {
    data = List_getNextRef(&itr);
    i++;
  }

  return data;
}

void * List_removeItem(List * list, U4 index)
{
  Link * l = list->head;
  U4 i = 0;

  while (i != index && l)
  {
    l = l->next;
    i++;
  }

  if (i != index || !l) return NULL;

  void * data = l->data;
  List_destroyLink(list, l);
  return data;
}

void List_deleteItem(List * list, U4 index)
{
  Link * l = list->head;
  U4 i = 0;

  while (i != index && l)
  {
    l = l->next;
    i++;
  }

  if (i != index || !l) return;

  deleteLinkData(list, l);
  List_destroyLink(list, l);
}

void List_destroyLink(List * list, Link * link)
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

  free(link);
}

void List_destroyLinkAndData(List * list, Link * link)
{
  if (!list || !link) return;

  void * data = link->data;

  List_destroyLink(list, link);
  if (list->freeFunc) list->freeFunc(data);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static Link * createLink(void)
{
  Link * link = malloc(sizeof(Link));
  memset(link, 0, sizeof(Link));
  return link;
}

static void deleteLinkData(List * list, Link * l)
{
  if (list->passByVal && l->data)
  {
    if (list->freeFunc)
    {
      list->freeFunc(l->data);
    }
    else
    {
      free(l->data);
    }
  }
}
