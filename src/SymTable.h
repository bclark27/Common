#ifndef SYM_TABLE_H_
#define SYM_TABLE_H_

#include "Standard.h"

///////////////
//  DEFINES  //
///////////////

#define FLAG_FUNCTION           0x01
#define FLAG_VAR                0x02
#define FLAG_CONST              0x04

#define FLAG_BOOL               0x08
#define FLAG_INT                0x10
#define FLAG_STRING             0x20

#define TYPE_MASK               0x38

/////////////
//  TYPES  //
/////////////

typedef struct SymObj
{
  unsigned int typeFlags;
  unsigned int nameLen;
  char * name;
  struct SymObj * nextSymObj;
} SymObj;

typedef struct ScopeNode
{
  unsigned int numChildren;
  unsigned int numOfSymObj;
  struct SymObj * symObjList;
  struct ScopeNode * parent;
  struct ScopeNode ** children;
} ScopeNode;

typedef struct SymTable
{
  ScopeNode * rootScope;
  ScopeNode * currentScope;

} SymTable;

#endif

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

// create table
SymTable * SymTable_init(void);

// free table
void SymTable_free(SymTable * st);

// print out the sym table
void SymTable_print(SymTable * st);

// creates a child scope and enters it, returns 0 upon failure to make new child
char SymTable_enterNewScope(SymTable * st);

// recurse back to parent scope, returns 0 if scope is root anc cant go back
char SymTable_backOneScope(SymTable * st);

// inserts a new ID and returns 1 if enterd success, 0 if not added because it exists already
SymObj * SymTable_insertID(SymTable * st, char * name, unsigned int nameLen, unsigned int typeFlags);

// returns 1 if sym exists already
SymObj * SymTable_symDeclared(SymTable * st, char * name, unsigned int nameLen);
