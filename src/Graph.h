#ifndef GRAPH_H_
#define GRAPH_H_

#include "Common.h"
#include "TypedHashTable.h"

/////////////
//  TYPES  //
/////////////

typedef struct
{
  void * a;
  void * b;
} GraphEdge;

typedef struct
{
  TypedHashTable * ht;
  U4 valLen;
  bool directed;
} Graph;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

Graph * Graph_init(U4 valLen, bool directed);
void Graph_free(Graph * g);

bool Graph_addEdge(Graph * g, void * a, void * b);
bool Graph_addNode(Graph * g, void * a);
void Graph_iterateNodes(Graph * g, callbackFunction callback, void * args);
void Graph_iterateEdges(Graph * g, callbackFunction callback, void * args);

#endif
