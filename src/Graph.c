#include "Graph.h"
#include "HashSet.h"

///////////////
//  DEFINES  //
///////////////

/////////////
//  TYPES  //
/////////////

typedef struct
{
  HashSet * outGoingEdges;
  U4 inDegree;
  U4 outDegree;
} VertexInfo;

typedef struct
{
  void * args;
  callbackFunction callback;
} GraphCallbackPackage1;

typedef struct
{
  void * args;
  void * parentNode;
  callbackFunction callback;
} GraphCallbackPackage2;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static void createVertexInfo(Graph * g, VertexInfo * vi);
static void freeVertexInfo(void * ptr);
static bool addDirectedEdge(Graph * g, void * a, void * b);
static void graphCallBackHelper1(void * keyValPair, void * package);
static void graphCallBackHelper2(void * val, void * package);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

Graph * Graph_init(U4 valLen, bool directed)
{
  Graph * g = calloc(1, sizeof(Graph));
  g->ht = TypedHashTable_init(valLen, sizeof(VertexInfo), true, freeVertexInfo);

  g->valLen = valLen;
  g->directed = directed;

  return g;
}

void Graph_free(Graph * g)
{
  if (!g) return;
  if (g->ht) TypedHashTable_free(g->ht);
  free(g);
}

bool Graph_addEdge(Graph * g, void * a, void * b)
{
  if (!g || !g->ht || !a || !b) return false;

  bool added = true;
  added &= addDirectedEdge(g, a, b);
  if (g->directed && added) added &= addDirectedEdge(g, b, a);

  return added;
}

bool Graph_addNode(Graph * g, void * a)
{
  if (!g || !g->ht || !a) return false;

  if (!TypedHashTable_getRef(g->ht, a))
  {
    VertexInfo newVi;
    createVertexInfo(g, &newVi);
    TypedHashTable_insert(g->ht, a, &newVi);
  }

  return true;
}

void Graph_iterateNodes(Graph * g, callbackFunction callback, void * args)
{

}

void Graph_iterateEdges(Graph * g, callbackFunction callback, void * args)
{
  if (!g || !g->ht || !callback) return;
  GraphCallbackPackage1 pack = {.args = args, .callback = callback};
  TypedHashTable_iterateKV(g->ht, graphCallBackHelper1, &pack);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static void createVertexInfo(Graph * g, VertexInfo * vi)
{
  vi->outGoingEdges = HashSet_init(g->valLen, true, NULL);
  vi->inDegree = 0;
  vi->outDegree = 0;
}

static void freeVertexInfo(void * ptr)
{
  VertexInfo * vertexInfo = ptr;
  if (!vertexInfo) return;
  if (vertexInfo->outGoingEdges) HashSet_free(vertexInfo->outGoingEdges);
  free(vertexInfo);
}

static bool addDirectedEdge(Graph * g, void * a, void * b)
{
  VertexInfo * vi = TypedHashTable_getRef(g->ht, a);

  if (vi)
  {
    vi->outDegree++;
    HashSet_insert(vi->outGoingEdges, b);
  }
  else
  {
    VertexInfo newVi;
    createVertexInfo(g, &newVi);

    newVi.outDegree++;
    HashSet_insert(newVi.outGoingEdges, b);
    TypedHashTable_insert(g->ht, a, &newVi);
  }

  return true;
}

static void graphCallBackHelper1(void * keyValPair, void * package)
{
  GraphCallbackPackage1 * pack = package;
  void * args = pack->args;
  callbackFunction callback = pack->callback;

  HashKVPair * kv = keyValPair;

  void * node_a = kv->key;
  VertexInfo * vi = kv->val;
  HashSet * outEdges = vi->outGoingEdges;

  GraphCallbackPackage2 pack2 = {.args = args, .parentNode = node_a, .callback = callback};

  HashSet_iterate(outEdges, graphCallBackHelper2, &pack2);
}

static void graphCallBackHelper2(void * val, void * package)
{
  GraphCallbackPackage2 * pack2 = package;

  callbackFunction callback = pack2->callback;

  GraphEdge ge = {.a = pack2->parentNode, .b = val};

  callback(&ge, pack2->args);
}
