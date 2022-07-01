#ifndef BINARY_TREE_H_
#define BINARY_TREE_H_

#include "Standard.h"
#include "List.h"

///////////////
//  DEFINES  //
///////////////

#define BinaryTree_preOrderTraversal(bt, call) BinaryTree_nodePreOrderTraversal(((BinaryTree*)bt)->headNode, call)
#define BinaryTree_postOrderTraversal(bt, call) BinaryTree_nodePostOrderTraversal(((BinaryTree*)bt)->headNode, call)
#define BinaryTree_inOrderTraversal(bt, call) BinaryTree_nodeInOrderTraversal(((BinaryTree*)bt)->headNode, call)

/////////////
//  TYPES  //
/////////////

typedef void ( *binaryTreeCallback)(void *);

typedef struct TreeNode
{
  unsigned int nodeCount;
  void * data;
  struct TreeNode * left;
  struct TreeNode * right;
} TreeNode;

typedef struct BinaryTree
{
  unsigned int nodeCount;
  CompareFunction compFunc;
  FreeDataFunction freeFunc;
  TreeNode * headNode;
} BinaryTree;

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

BinaryTree * BinaryTree_init(CompareFunction compFunc, FreeDataFunction freeFunc);
void BinaryTree_free(BinaryTree * bt);

void BinaryTree_nodePreOrderTraversal(TreeNode * tn, binaryTreeCallback callback);
void BinaryTree_nodePostOrderTraversal(TreeNode * tn, binaryTreeCallback callback);
void BinaryTree_nodeInOrderTraversal(TreeNode * tn, binaryTreeCallback callback);

char BinaryTree_insert(BinaryTree * bt, void * data);
char BinaryTree_remove(BinaryTree * bt, void * data);

#endif
