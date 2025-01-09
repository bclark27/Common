#include "Graph.h"
#include "List.h"
#include "HashSet.h"

///////////////
//  DEFINES  //
///////////////

/////////////
//  TYPES  //
/////////////

typedef struct
{
  List * outGoingEdges;
  U4 inDegree;
  U4 outDegree;
} VertexInfo;

typedef struct
{
  HashSet * visited;
  CallbackFunction callback;
  void * args;
} EdgeItrPkg;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static void freeVertexInfo(void * ptr);
static void edgeItrHelper(void * kvpair, void * pkg);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

Graph * Graph_init(U4 valLen, bool directed, FreeDataFunction freeNodeFunction,  HashFunction nodeHashFunc)
{
  Graph * g = calloc(1, sizeof(Graph));
  g->ht = TypedHashTable_init(valLen, false, NULL, sizeof(VertexInfo), false, freeVertexInfo, nodeHashFunc);

  g->valLen = valLen;
  g->nodeHashFunc = nodeHashFunc;
  g->freeNodeFunction = freeNodeFunction;
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

  Graph_addNode(g, a);
  Graph_addNode(g, b);


  VertexInfo * a_vi = TypedHashTable_getRef(g->ht, a);

  List * aOutGoing = a_vi->outGoingEdges;

  ListItr itr = List_getItr(aOutGoing);
  void * val = List_getNextRef(&itr);
  bool found = false;

  while (val)
  {
    if (memcmp(val, b, g->valLen) == 0)
    {
      found = true;
      break;
    }
    val = List_getNextRef(&itr);
  }

  if (!found)
  {
    List_queue(aOutGoing, b);
    a_vi->outDegree++;
    VertexInfo * b_vi = TypedHashTable_getRef(g->ht, b);
    b_vi->inDegree++;
  }

  return true;
}

bool Graph_addNode(Graph * g, void * a)
{
  if (!g || !g->ht || !a) return false;

  // if the node doesnt exist already...
  if (!TypedHashTable_keyIn(g->ht, a))
  {
    // add a new kv pair <a, vertexinfo> to the table with empty list in vertex info
    VertexInfo vi = {.outGoingEdges = List_listInit(g->valLen, true, NULL, g->freeNodeFunction), .outDegree = 0, .inDegree = 0};
    TypedHashTable_insert(g->ht, a, &vi);
  }

  return true;
}

void Graph_depthFirst(Graph * g, void * startNode, CallbackFunction callback, void * args)
{
  if (!g || !callback) return;

  // check if the start node is even part of the graph first
  if (!TypedHashTable_getRef(g->ht, startNode)) return;

  HashSet * visited = HashSet_init(g->valLen, false, NULL, g->nodeHashFunc);
  Stack * toVisit = Stack_init(g->valLen, false, NULL);

  Stack_push(toVisit, startNode);

  while (toVisit->len)
  {
    void * thisNode = Stack_pop(toVisit);

    if (!HashSet_keyIn(visited, thisNode))
    {
      HashSet_insert(visited, thisNode);
      callback(thisNode, args);

      VertexInfo * vi = TypedHashTable_getRef(g->ht, thisNode);

      if (vi && vi->outGoingEdges)
      {
        List * outGoing = vi->outGoingEdges;
        ListItr itr = List_getItr(outGoing);
        void * nextNode = List_getNextRef(&itr);

        while (nextNode)
        {
          Stack_push(toVisit, nextNode);
          nextNode = List_getNextRef(&itr);
        }
      }
    }
  }

  Stack_free(toVisit);
  HashSet_free(visited);
}

void Graph_breadthFirst(Graph * g, void * startNode, CallbackFunction callback, void * args)
{
  if (!g || !callback) return;

  // check if the start node is even part of the graph first
  if (!TypedHashTable_getRef(g->ht, startNode)) return;

  HashSet * visited = HashSet_init(g->valLen, false, NULL, g->nodeHashFunc);
  Queue * toVisit = Queue_init(g->valLen, false, NULL);

  Queue_queue(toVisit, startNode);

  while (toVisit->len)
  {
    void * thisNode = Queue_dequeue(toVisit);

    if (!HashSet_keyIn(visited, thisNode))
    {
      HashSet_insert(visited, thisNode);
      callback(thisNode, args);

      VertexInfo * vi = TypedHashTable_getRef(g->ht, thisNode);

      if (vi && vi->outGoingEdges)
      {
        List * outGoing = vi->outGoingEdges;
        ListItr itr = List_getItr(outGoing);
        void * nextNode = List_getNextRef(&itr);

        while (nextNode)
        {
          Queue_queue(toVisit, nextNode);
          nextNode = List_getNextRef(&itr);
        }
      }
    }
  }

  Queue_free(toVisit);
  HashSet_free(visited);
}

void Graph_iterateEdges(Graph * g, CallbackFunction callback, void * args)
{
  if (!g || !g->ht || !callback) return;
  HashSet * visited = HashSet_init(sizeof(GraphEdge), false, NULL, g->nodeHashFunc);
  EdgeItrPkg pkg = {.callback = callback, .args = args, .visited = visited};
  TypedHashTable_iterateKV(g->ht, edgeItrHelper, &pkg);
  HashSet_free(visited);
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static void freeVertexInfo(void * ptr)
{
  if (!ptr) return;
  VertexInfo * vi = ptr;

  if (vi->outGoingEdges) List_destroyList(vi->outGoingEdges);
  free(vi);
}

static void edgeItrHelper(void * kvpair, void * pkg)
{
  EdgeItrPkg * p = pkg;
  HashSet * visited = p->visited;
  CallbackFunction callback = p->callback;
  void * args = p->args;

  HashKVPair * kv = kvpair;
  void * k = kv->key;
  VertexInfo * vi = kv->val;

  GraphEdge ge1 = {.a = k};
  GraphEdge ge2 = {.b = k};

  List * aOutGoing = vi->outGoingEdges;
  ListItr itr = List_getItr(aOutGoing);
  void * val = List_getNextRef(&itr);

  while (val)
  {
    ge1.b = val;
    ge2.a = val;
    if (HashSet_keyIn(visited, &ge1) || HashSet_keyIn(visited, &ge2))
    {
      continue;
    }

    HashSet_insert(visited, &ge1);
    HashSet_insert(visited, &ge2);

    callback(&ge1, args);

    val = List_getNextRef(&itr);
  }
}
