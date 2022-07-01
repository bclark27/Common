#include "BinaryTree.h"

///////////////
//  DEFINES  //
///////////////

#define BF(node) (computeHeight((TreeNode*)node->right) - computeHeight((TreeNode*)node->left))


/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

TreeNode * initTreeNode(void);
int computeHeight(TreeNode * node);
char insertHelper(BinaryTree * bt, TreeNode * thisNode, TreeNode * nodeToPut);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

BinaryTree * BinaryTree_init(CompareFunction compFunc, FreeDataFunction freeFunc)
{
  BinaryTree * bt = (BinaryTree *)callocOrDie(1, sizeof(BinaryTree));
  bt->compFunc = compFunc;
  bt->freeFunc = freeFunc;
  return bt;
}

void BinaryTree_free(BinaryTree * bt);

void BinaryTree_nodePreOrderTraversal(TreeNode * tn, binaryTreeCallback callback)
{
  if (tn == NULL) return;
  if (tn->data != NULL) callback(tn->data);
  BinaryTree_nodePreOrderTraversal(tn->left, callback);
  BinaryTree_nodePreOrderTraversal(tn->right, callback);
}

void BinaryTree_nodePostOrderTraversal(TreeNode * tn, binaryTreeCallback callback)
{
  if (tn == NULL) return;
  BinaryTree_nodePostOrderTraversal(tn->left, callback);
  BinaryTree_nodePostOrderTraversal(tn->right, callback);
  if (tn->data != NULL) callback(tn->data);
}

void BinaryTree_nodeInOrderTraversal(TreeNode * tn, binaryTreeCallback callback)
{
  if (tn == NULL) return;
  BinaryTree_nodeInOrderTraversal(tn->left, callback);
  if (tn->data != NULL) callback(tn->data);
  BinaryTree_nodeInOrderTraversal(tn->right, callback);
}

char BinaryTree_insert(BinaryTree * bt, void * data)
{
  if (data == NULL)
  {
    printf("Data to insert is NULL\n");
    exit(1);
  }

  if (bt == NULL)
  {
    printf("Binary Tree is NULL\n");
    exit(1);
  }

  TreeNode * tn = initTreeNode();
  tn->data = data;

  if (bt->headNode == NULL)
  {
    bt->headNode = tn;
    return 1;
  }

  return insertHelper(bt, bt->headNode, tn);
}

char BinaryTree_remove(BinaryTree * bt, void * data);

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

TreeNode * initTreeNode(void)
{
  TreeNode * tn = (TreeNode *)callocOrDie(1, sizeof(TreeNode));
  tn->nodeCount = 1;
  return tn;
}

int computeHeight(TreeNode * node)
{
  if (node == NULL)
  {
    return 0;
  }
  int left = computeHeight(node->left);
  int right = computeHeight(node->right);
  return MAX(left, right) + 1;
}



char insertHelper(BinaryTree * bt, TreeNode * thisNode, TreeNode * nodeToPut)
{
  int comp = bt->compFunc(thisNode->data, nodeToPut->data);
  if (comp == 0) return 0;

  if (comp > 0)
  {
    if (thisNode->left == NULL)
    {
      thisNode->left = nodeToPut;
      return 1;
    }
    else
    {
      if (insertHelper(bt, thisNode->left, nodeToPut))
      {
        thisNode->nodeCount++;

        if (1)
        {

        }
      }
    }
  }
  else
  {
    if (thisNode->right == NULL)
    {
      thisNode->right = nodeToPut;
      return 1;
    }
    else
    {
      if (insertHelper(bt, thisNode->right, nodeToPut))
      {
        thisNode->nodeCount++;
      }
    }
  }

  return 1;
}
