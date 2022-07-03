#include "BinaryTree.h"

///////////////
//  DEFINES  //
///////////////

#define BF(node) (computeHeight((TreeNode*)node->right) - computeHeight((TreeNode*)node->left))


/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

TreeNode * initTreeNode(void);
U4 computeHeight(TreeNode * node);
bool insertHelper(BinaryTree * bt, TreeNode * thisNode, TreeNode * nodeToPut);
void deleteSubTree(TreeNode * node, bool freeData, FreeDataFunction freeFunc);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

BinaryTree * BinaryTree_init(U4 dataLen, bool passByVal, CompareFunction compFunc, FreeDataFunction freeFunc)
{
  BinaryTree * bt = (BinaryTree *)callocOrDie(1, sizeof(BinaryTree));
  bt->compFunc = compFunc;
  bt->freeFunc = freeFunc;
  bt->passByVal = passByVal;
  bt->dataLen = dataLen;
  return bt;
}

void BinaryTree_free(BinaryTree * bt)
{
  if (!bt) return;
  deleteSubTree(bt->headNode, bt->passByVal, bt->freeFunc);
}

void BinaryTree_nodePreOrderTraversal(TreeNode * tn, callbackFunction callback)
{
  if (tn == NULL) return;
  if (tn->data != NULL && callback) callback(tn->data);
  BinaryTree_nodePreOrderTraversal(tn->left, callback);
  BinaryTree_nodePreOrderTraversal(tn->right, callback);
}

void BinaryTree_nodePostOrderTraversal(TreeNode * tn, callbackFunction callback)
{
  if (tn == NULL) return;
  BinaryTree_nodePostOrderTraversal(tn->left, callback);
  BinaryTree_nodePostOrderTraversal(tn->right, callback);
  if (tn->data != NULL && callback) callback(tn->data);
}

void BinaryTree_nodeInOrderTraversal(TreeNode * tn, callbackFunction callback)
{
  if (tn == NULL) return;
  BinaryTree_nodeInOrderTraversal(tn->left, callback);
  if (tn->data != NULL && callback) callback(tn->data);
  BinaryTree_nodeInOrderTraversal(tn->right, callback);
}

bool BinaryTree_insert(BinaryTree * bt, void * data)
{
  if (!data || !data) return false;

  TreeNode * tn = initTreeNode();

  if (bt->passByVal)
  {
    tn->data = mallocOrDie(bt->dataLen);
    memcpy(tn->data, data, bt->dataLen);
  }
  else
  {
    tn->data = data;
  }

  if (bt->headNode == NULL)
  {
    bt->headNode = tn;
    return true;
  }

  return insertHelper(bt, bt->headNode, tn);
}

bool BinaryTree_remove(BinaryTree * bt, void * data);

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

TreeNode * initTreeNode(void)
{
  TreeNode * tn = (TreeNode *)callocOrDie(1, sizeof(TreeNode));
  tn->nodeCount = 1;
  return tn;
}

U4 computeHeight(TreeNode * node)
{
  if (node == NULL)
  {
    return 0;
  }

  U4 left = computeHeight(node->left);
  U4 right = computeHeight(node->right);
  return MAX(left, right) + 1;
}

bool insertHelper(BinaryTree * bt, TreeNode * thisNode, TreeNode * nodeToPut)
{
  U1 comp = bt->compFunc(thisNode->data, nodeToPut->data);
  if (comp == 0) return false;

  if (comp > 0)
  {
    if (thisNode->left == NULL)
    {
      thisNode->left = nodeToPut;
    }
    else
    {
      if (insertHelper(bt, thisNode->left, nodeToPut))
      {
        thisNode->nodeCount++;
      }
    }
  }
  else
  {
    if (thisNode->right == NULL)
    {
      thisNode->right = nodeToPut;
    }
    else
    {
      if (insertHelper(bt, thisNode->right, nodeToPut))
      {
        thisNode->nodeCount++;
      }
    }
  }

  return true;
}

void deleteSubTree(TreeNode * node, bool freeData, FreeDataFunction freeFunc)
{
  if (!node) return;

  deleteSubTree(node->right, freeData, freeFunc);
  deleteSubTree(node->left, freeData, freeFunc);

  if (freeData && node->data && freeFunc) freeFunc(node->data);

  free(node);
}
