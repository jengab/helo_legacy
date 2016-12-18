#ifndef TREE_H
#define TREE_H

#include <inttypes.h>

#define HASH_TABLE_SIZE 23

//bool-like type(bool is not KRC standard)
typedef enum{
  false,true
}logic;

//enum to set WordTypes
typedef enum _WordType{
  Word,Hybrid,Number
}
wordtype;

//this structure describes a token, RefCounter was introduced to know how many pointers refer this token
//it may be used to know wether we can free yet, Length is only for performance purposes
typedef struct _TokenDescriptor{
  char* TokenString;
  wordtype TypeOfToken;
  int RefCounter;
  int Length;
}
TokenDescriptor;

typedef struct _ListOfPositions{
  intptr_t Data;
  int Counter;
  struct _ListOfPositions* Next;
} ListOfPositions;

//this structure is just for search
typedef struct _key{
  TokenDescriptor* Descriptor;
  intptr_t data;
}
key;

//define tree node...
typedef struct _node{
  struct _node* left;
  struct _node* right;
  ListOfPositions* HashTable[HASH_TABLE_SIZE];
  TokenDescriptor* DescriptorOfToken;
}
node;

//this structure is for search result also(note that search is a void return function!)
typedef struct _result{
  logic found;
  node* head;
} result;

typedef node* tree;


extern void ReleaseFiles(void);

unsigned int Hash(intptr_t key);

//constructing binary tree also
tree InsertElem(tree* head,const key* key);
//binary search inside tree
void Search(tree head,const char* name,result* res);
//deallocate whole tree
void FreeTree(tree* head);
//in-order treewalk algorithm
void TreeWalk(tree head,void (*activity)(tree,void*),void* par);
void PrintNode(tree node,void* par);
//Search inside Statics of a given tree node, returns NULL is no statics found
ListOfPositions* SearchElem(ListOfPositions** first,intptr_t pos);
//Build tree, just setting pointers to correct values
tree SoftInsertElem(tree* head,const key* key);
//deallocate SoftInsert-made tree
void SoftFreeTree(tree* head);
//delete linked list of statics, and setting data NULL in a given node(subtree)
void DestroyStatics(tree node,void* par);
//returns the sum of counters for distinct positions for one node only
int SumElem(tree node);

#endif
