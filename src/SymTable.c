#include "SymTable.h"

///////////////
//  DEFINES  //
///////////////

/////////////
//  TYPES  //
/////////////

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

// creates a sym obj and returns the addr
SymObj * createSymObj(char * name, unsigned int nameLen, unsigned int typeFlags);

// fres a sym obj
void freeSymObj(void * ptr);

// frees a scope
void freeScopeNode(ScopeNode * self);

// check if a sym name is in a sym list, returns teh addr if it exists
SymObj * checkIfSymInList(SymObj * thisSymObj, char * name, unsigned int nameLen);

// check if a sym name is in a sym list or parent scopes, returns teh addr if it exists
SymObj * checkIfSymInHierarchy(ScopeNode * self, char * name, unsigned int nameLen);

// init a new scope
ScopeNode * initScopeNode(ScopeNode * parent);

// delete a scope
char deleteScopeNode(ScopeNode * sn);

// adds new scope child to the scope provided, returns the addr of the child scope
ScopeNode * addScope(ScopeNode * self);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

SymTable * SymTable_init(void)
{
  SymTable * st = calloc(1, sizeof(SymTable));
  st->rootScope = initScopeNode(NULL);
  st->currentScope = st->rootScope;
  return st;
}

void SymTable_free(SymTable * st)
{

}

void SymTable_print(SymTable * st)
{

}

char SymTable_enterNewScope(SymTable * st)
{
  st->currentScope = addScope(st->currentScope);
  return st->currentScope != 0;
}

char SymTable_backOneScope(SymTable * st)
{
  if (st->currentScope == st->rootScope || st->currentScope->parent == NULL) return 0;
  st->currentScope = st->currentScope->parent;
  return 1;
}

// inserts a new ID to current scope
SymObj * SymTable_insertID(SymTable * st, char * name, unsigned int nameLen, unsigned int typeFlags)
{
  SymObj * symObjFound = checkIfSymInList(st->currentScope->symObjList, name, nameLen);
  if (symObjFound) return symObjFound;

  SymObj * so = createSymObj(name, nameLen, typeFlags);

  so->nextSymObj = st->currentScope->symObjList;
  st->currentScope->symObjList = so;
  st->currentScope->numOfSymObj++;
  return so;
}

SymObj * SymTable_symDeclared(SymTable * st, char * name, unsigned int nameLen)
{
  return checkIfSymInHierarchy(st->currentScope, name, nameLen);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

SymObj * createSymObj(char * name, unsigned int nameLen, unsigned int typeFlags)
{
  SymObj * so = calloc(1, sizeof(SymObj));
  so->typeFlags = typeFlags;
  so->nameLen = nameLen;
  so->name = malloc(nameLen);
  so->nextSymObj = NULL;
  memcpy(so->name, name, nameLen);
  return so;
}

void freeSymObj(void * ptr)
{
  SymObj * so = (SymObj*)ptr;
  if (so->name) free(so->name);
  free(ptr);
}

void freeScopeNode(ScopeNode * self)
{

}

// return 1 if the sym exists in this list
SymObj * checkIfSymInList(SymObj * thisSymObj, char * name, unsigned int nameLen)
{

  if (thisSymObj == NULL) return NULL;

  if (nameLen == thisSymObj->nameLen)
  {
    for (int i = 0; i < nameLen; i++)
    {
      if (name[i] != thisSymObj->name[i])
      {
        return checkIfSymInList(thisSymObj->nextSymObj, name, nameLen);
      }
    }
    return thisSymObj;
  }
  return checkIfSymInList(thisSymObj->nextSymObj, name, nameLen);
}

// returns 1 if the sym is in this scope or any of the parents above
SymObj * checkIfSymInHierarchy(ScopeNode * self, char * name, unsigned int nameLen)
{
  if (self == NULL) return NULL;

  SymObj * sumObjFound = checkIfSymInList(self->symObjList, name, nameLen);
  if (sumObjFound) return sumObjFound;
  return checkIfSymInHierarchy(self->parent, name, nameLen);
}

ScopeNode * initScopeNode(ScopeNode * parent)
{
  ScopeNode * sn = malloc(sizeof(ScopeNode));
  sn->numChildren = 0;
  sn->numOfSymObj = 0;
  sn->symObjList = NULL;
  sn->parent = parent;
  sn->children = NULL;

  return sn;
}

char deleteScopeNode(ScopeNode * sn)
{
  if (sn == NULL) return 1;

  for (int i = 0; i < sn->numChildren; i++)
  {
    deleteScopeNode(sn->children[i]);
  }

  free(sn);
  return 1;
}

// adds new scope child to the scope provided, returns the addr of the child scope
ScopeNode * addScope(ScopeNode * self)
{
  if (self == NULL) return 0;
  ScopeNode * newScope = initScopeNode(self);

  if (self->children == NULL || self->numChildren == 0)
  {
    self->children = malloc(sizeof(ScopeNode*));
    self->children[0] = newScope;
    self->numChildren++;
    return newScope;
  }

  //add another scope to end
  ScopeNode ** newChildList = (ScopeNode **)calloc(self->numChildren + 1, sizeof(ScopeNode *));
  memcpy(newChildList, self->children, sizeof(ScopeNode*) * self->numChildren);
  newChildList[self->numChildren] = newScope;

  ScopeNode ** tmp = self->children;
  self->children = newChildList;
  self->numChildren++;
  free(tmp);

  return newScope;
}
