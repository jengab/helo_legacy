#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tree.h"


unsigned int Hash(intptr_t key){
  return abs(key)%HASH_TABLE_SIZE;
}

//this file contains an implementation for binary search tree
//We are going to use it to store words and its positions
//and the number of appearances, all nodes are the head of a list basically, but they also store other datas

void Search(tree head,const char* name,result* ptrStructResult){
  if(name==NULL || ptrStructResult==NULL)return;
  if(head==NULL){
    ptrStructResult->found=false;
    ptrStructResult->head=NULL;
    return;
  }
  int cmp=strcmp(head->DescriptorOfToken->TokenString,name);
  if(cmp==0){//found the correct node
    ptrStructResult->found=true;
    ptrStructResult->head=head;
  }
  if(cmp>0){//going left
    if(head->left==NULL){
      ptrStructResult->found=false;
      ptrStructResult->head=head;
      return;
    }
    Search(head->left,name,ptrStructResult);
  }
  if(cmp<0){//going right
    if(head->right==NULL){
      ptrStructResult->found=false;
      ptrStructResult->head=head;
      return;
    }
    Search(head->right,name,ptrStructResult);
  }
}

tree InsertElem(tree* head,const key* key){
  if(key==NULL)return NULL;
  result RetVal={0,NULL};
  tree LocalInstance=*head;
  Search(LocalInstance,key->Descriptor->TokenString,&RetVal);
  if(RetVal.found==false){//no such element in the tree
    if(RetVal.head==NULL){//is it the main node?
      *head=(node*)malloc(sizeof(node));
      if(*head==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      //lets use local variable, to avoid cross reference resolve(see compiled Assembly code)
      LocalInstance=*head;
      memset(LocalInstance->HashTable,'\0',sizeof(ListOfPositions*)*HASH_TABLE_SIZE);
      LocalInstance->right=NULL;
      LocalInstance->left=NULL;
      LocalInstance->DescriptorOfToken=(TokenDescriptor*)malloc(sizeof(TokenDescriptor));
      if(LocalInstance->DescriptorOfToken==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      LocalInstance->DescriptorOfToken->TypeOfToken=key->Descriptor->TypeOfToken;
      LocalInstance->DescriptorOfToken->TokenString=(char*)malloc(sizeof(char)*(key->Descriptor->Length+1));
      if(LocalInstance->DescriptorOfToken->TokenString==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      LocalInstance->DescriptorOfToken->Length=key->Descriptor->Length;
      LocalInstance->DescriptorOfToken->RefCounter=0;
      strcpy(LocalInstance->DescriptorOfToken->TokenString,key->Descriptor->TokenString);
      unsigned int index=Hash(key->data);
      if((LocalInstance->HashTable[index]=(ListOfPositions*)malloc(sizeof(ListOfPositions)))==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      LocalInstance->HashTable[index]->Counter=1;
      LocalInstance->HashTable[index]->Data=key->data;
      LocalInstance->HashTable[index]->Next=NULL;
      return (LocalInstance);
    }
    //where should we put the new node?
    if(strcmp(RetVal.head->DescriptorOfToken->TokenString,key->Descriptor->TokenString)>0){
      RetVal.head->left=(node*)malloc(sizeof(node));
      if(RetVal.head->left==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      memset(RetVal.head->left->HashTable,'\0',sizeof(ListOfPositions*)*HASH_TABLE_SIZE);
      RetVal.head->left->DescriptorOfToken=(TokenDescriptor*)malloc(sizeof(TokenDescriptor));
      if(RetVal.head->left->DescriptorOfToken==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      RetVal.head->left->DescriptorOfToken->TypeOfToken=key->Descriptor->TypeOfToken;
      RetVal.head->left->DescriptorOfToken->TokenString=(char*)malloc(sizeof(char)*(strlen(key->Descriptor->TokenString)+1));
      if(RetVal.head->left->DescriptorOfToken->TokenString==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      strcpy(RetVal.head->left->DescriptorOfToken->TokenString,key->Descriptor->TokenString);
      RetVal.head->left->DescriptorOfToken->Length=key->Descriptor->Length;
      RetVal.head->left->DescriptorOfToken->RefCounter=0;
      RetVal.head->left->left=NULL;
      RetVal.head->left->right=NULL;
      unsigned int index=Hash(key->data);
      if((RetVal.head->left->HashTable[index]=(ListOfPositions*)malloc(sizeof(ListOfPositions)))==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      RetVal.head->left->HashTable[index]->Counter=1;
      RetVal.head->left->HashTable[index]->Data=key->data;
      RetVal.head->left->HashTable[index]->Next=NULL;
      return(RetVal.head->left);
    }
    else{
      RetVal.head->right=(node*)malloc(sizeof(node));
      if(RetVal.head->right==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      memset(RetVal.head->right->HashTable,'\0',HASH_TABLE_SIZE*sizeof(ListOfPositions*));
      RetVal.head->right->DescriptorOfToken=(TokenDescriptor*)malloc(sizeof(TokenDescriptor));
      if(RetVal.head->right->DescriptorOfToken==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      RetVal.head->right->DescriptorOfToken->TypeOfToken=key->Descriptor->TypeOfToken;
      RetVal.head->right->DescriptorOfToken->TokenString=(char*)malloc(sizeof(char)*(strlen(key->Descriptor->TokenString)+1));
      if(RetVal.head->right->DescriptorOfToken->TokenString==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      strcpy(RetVal.head->right->DescriptorOfToken->TokenString,key->Descriptor->TokenString);
      RetVal.head->right->DescriptorOfToken->Length=key->Descriptor->Length;
      RetVal.head->right->DescriptorOfToken->RefCounter=0;
      RetVal.head->right->right=NULL;
      RetVal.head->right->left=NULL;
      unsigned int index=Hash(key->data);
      if((RetVal.head->right->HashTable[index]=(ListOfPositions*)malloc(sizeof(ListOfPositions)))==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      RetVal.head->right->HashTable[index]->Counter=1;
      RetVal.head->right->HashTable[index]->Data=key->data;
      RetVal.head->right->HashTable[index]->Next=NULL;
      return(RetVal.head->right);
    }
  }
  else{//the list of string exists
    unsigned int index=Hash(key->data);
    ListOfPositions* act=RetVal.head->HashTable[index];
    if(act==NULL){
      act=(ListOfPositions*)malloc(sizeof(ListOfPositions));
      if(act==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      act->Counter=1;
      act->Data=key->data;
      act->Next=NULL;
      RetVal.head->HashTable[index]=act;
      return RetVal.head;
    }
    while(act->Next!=NULL && act->Data!=key->data)act=act->Next;
    if(act->Data==key->data){
      act->Counter++;
    }
    else{
      act->Next=(ListOfPositions*)malloc(sizeof(ListOfPositions));
      if(act->Next==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      act->Next->Next=NULL;
      act->Next->Counter=1;
      act->Next->Data=key->data;
    }
  }
  return RetVal.head;
}


//this is a post-order treewalk
void FreeTree(tree* head){
  if(*head==NULL)return;
  FreeTree(&(*head)->right);
  FreeTree(&(*head)->left);
  int i=0;
  for(;i<HASH_TABLE_SIZE;i++){
    ListOfPositions* act=(*head)->HashTable[i];
    while(act!=NULL){
      ListOfPositions* tmp=act->Next;
      free(act);
      act=NULL;
      act=tmp;
    }
  }
  memset((*head)->HashTable,'\0',sizeof(ListOfPositions*)*HASH_TABLE_SIZE);
  free((*head)->DescriptorOfToken->TokenString);
  (*head)->DescriptorOfToken->TokenString=NULL;
  free((*head)->DescriptorOfToken);
  (*head)->DescriptorOfToken=NULL;
  free(*head);
  *head=NULL;
}

ListOfPositions* SearchElem(ListOfPositions** HashTable,intptr_t pos){
  ListOfPositions* act=HashTable[Hash(pos)];
  while(act!=NULL && act->Data!=pos)act=act->Next;
  return act;
}

//this function is only for test purposes, should not be compiled to a release version
void PrintNode(tree node,void* par){
  if(node==NULL)return;
  char type[15];
  if(node->DescriptorOfToken->TypeOfToken==Hybrid)strcpy(type,"Hybrid\0");
  if(node->DescriptorOfToken->TypeOfToken==Number)strcpy(type,"Number\0");
  if(node->DescriptorOfToken->TypeOfToken==Word)strcpy(type,"Word\0");
  printf("%s Type: %s\n",node->DescriptorOfToken->TokenString,type);
  int i=0;
  for(;i<HASH_TABLE_SIZE;i++){
    ListOfPositions* act=node->HashTable[i];
    while(act!=NULL){
      printf("\tPozicio: %d Multiplicitas: %d\n",act->Data,act->Counter);
      act=act->Next;
    }
  }
}

//this is general in-order tree walk method, therefore the second parameter
//is a void f(tree,void*) type function pointer, and third parameter may
//be given for the pointed function as initial parameter
void TreeWalk(tree head,void (*activity)(tree,void*),void* par){
  if(head==NULL || activity==NULL)return;
  TreeWalk(head->left,activity,par);
  activity(head,par);
  TreeWalk(head->right,activity,par);
}

//insert without storing the data, just setting pointers
tree SoftInsertElem(tree* head,const key* key){
  if(key==NULL)return NULL;
  result ret={0,NULL};
  tree temp=*head;
  Search(temp,key->Descriptor->TokenString,&ret);
  if(ret.found==false){//no such element in the tree
    if(ret.head==NULL){//is it the main node?
      *head=(node*)malloc(sizeof(node));
      if(*head==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      temp=*head;
      memset(temp->HashTable,'\0',sizeof(ListOfPositions*)*HASH_TABLE_SIZE);
      temp->right=NULL;
      temp->left=NULL;
      temp->DescriptorOfToken=key->Descriptor;
      temp->DescriptorOfToken->TypeOfToken=key->Descriptor->TypeOfToken;
      unsigned int index=Hash(key->data);
      if((temp->HashTable[index]=(ListOfPositions*)malloc(sizeof(ListOfPositions)))==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      temp->HashTable[index]->Counter=1;
      temp->HashTable[index]->Data=key->data;
      temp->HashTable[index]->Next=NULL;
      return (temp);
    }
    //where should we put the new node?
    if(strcmp(ret.head->DescriptorOfToken->TokenString,key->Descriptor->TokenString)>0){
      ret.head->left=(node*)malloc(sizeof(node));
      if(ret.head->left==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      memset(ret.head->left->HashTable,'\0',sizeof(ListOfPositions*)*HASH_TABLE_SIZE);
      ret.head->left->DescriptorOfToken=key->Descriptor;
      ret.head->left->left=NULL;
      ret.head->left->right=NULL;
      unsigned int index=Hash(key->data);
      if((ret.head->left->HashTable[index]=(ListOfPositions*)malloc(sizeof(ListOfPositions)))==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      ret.head->left->HashTable[index]->Counter=1;
      ret.head->left->HashTable[index]->Data=key->data;
      ret.head->left->HashTable[index]->Next=NULL;
      return(ret.head->left);
    }
    else{
      ret.head->right=(node*)malloc(sizeof(node));
      if(ret.head->right==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      memset(ret.head->right->HashTable,'\0',sizeof(ListOfPositions*)*HASH_TABLE_SIZE);
      ret.head->right->right=NULL;
      ret.head->right->left=NULL;
      ret.head->right->DescriptorOfToken=key->Descriptor;
      unsigned int index=Hash(key->data);
      if((ret.head->right->HashTable[index]=(ListOfPositions*)malloc(sizeof(ListOfPositions)))==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      ret.head->right->HashTable[index]->Counter=1;
      ret.head->right->HashTable[index]->Data=key->data;
      ret.head->right->HashTable[index]->Next=NULL;
      return(ret.head->right);
    }
  }
  else{//the list of string exists
    unsigned int index=Hash(key->data);
    ListOfPositions* act=ret.head->HashTable[index];
    if(act==NULL){
      act=(ListOfPositions*)malloc(sizeof(ListOfPositions));
      if(act==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      act->Counter=1;
      act->Data=key->data;
      act->Next=NULL;
      ret.head->HashTable[index]=act;
      return ret.head;
    }
    while(act->Next!=NULL && act->Data!=key->data)act=act->Next;
    if(act->Data==key->data){
      act->Counter++;
    }
    else{
      act->Next=(ListOfPositions*)malloc(sizeof(ListOfPositions));
      if(act->Next==NULL){
        fprintf(stderr,"Out of Memory\n");
        ReleaseFiles();
        exit(-2);
      }
      act->Next->Next=NULL;
      act->Next->Counter=1;
      act->Next->Data=key->data;
    }
  }
  return ret.head;
}

//we just delete nodes, not word descriptors
void SoftFreeTree(tree* head){
  if(*head==NULL)return;
  SoftFreeTree(&(*head)->right);
  SoftFreeTree(&(*head)->left);
  int i=0;
  for(;i<HASH_TABLE_SIZE;i++){
    ListOfPositions* act=(*head)->HashTable[i];
    while(act!=NULL){
      ListOfPositions* tmp=act->Next;
      free(act);
      act=tmp;
    }
  }
  free(*head);
  *head=NULL;
}

void DestroyStatics(tree node,void* par){
  if(node==NULL)return;
    int i=0;
  for(;i<HASH_TABLE_SIZE;i++){
    ListOfPositions* act=node->HashTable[i];
    while(act!=NULL){
      ListOfPositions* next=act->Next;
      free(act);
      act=next;
    }
    node->HashTable[i]=NULL;
  }
}


int SumElem(tree node){
  if(node==NULL)return 0;
  int res=0;
  int i=0;
  for(;i<HASH_TABLE_SIZE;i++){
    ListOfPositions* actelem=node->HashTable[i];
    while(actelem!=NULL){
      res+=actelem->Counter;
      actelem=actelem->Next;
    }
  }
  return res;
}
