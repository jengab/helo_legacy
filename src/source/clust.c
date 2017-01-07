#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "lists.h"
#include "clust.h"

/*in this file we are going to make HELO for a ListOfLines* arguments
 * this will represent a precluster(finally cluster for us)
 * in order to be able call it later from p_thread we must
 * implement everything thread safe(it is already done this way in final version)
 */

int GetSplit(ClusterNode clust){
  int NumberOfDistinctValuesPerColumn[MAX_TOKEN_COUNT];
  int NumberOfFilledColumns[MAX_TOKEN_COUNT];
  int LineCounter=0;
  memset(NumberOfDistinctValuesPerColumn,0,sizeof(int)*MAX_TOKEN_COUNT);
  memset(NumberOfFilledColumns,0,sizeof(int)*MAX_TOKEN_COUNT);
  tree dict=NULL;
  //for all lines
  ListOfLines* CurrentLine=clust.Content;
  while(CurrentLine!=NULL){
    //all words inside the line
    ListOfWords* CurrentWord=CurrentLine->WordList;
    int LineCounter=0;
    while(CurrentWord!=NULL && LineCounter<MAX_TOKEN_COUNT){
      key key;
      key.Descriptor=CurrentWord->TokDesc;
      key.data=LineCounter;
      //only set pointers to the already existing words
      //modifying words here can be dangerous, don't do it!
      tree temp=SoftInsertElem(&dict,&key);
      ListOfPositions* statics=SearchElem(temp->HashTable,LineCounter);
      if(statics!=NULL && statics->Counter==1){
        NumberOfDistinctValuesPerColumn[LineCounter]++;
      }
      if(temp->DescriptorOfToken->TypeOfToken!=Number){
        NumberOfFilledColumns[LineCounter]++;
      }
      LineCounter++;
      CurrentWord=CurrentWord->NextWord;
    }
    LineCounter++;
    CurrentLine=CurrentLine->NextLine;
  }
  //free the nodes, but not words
  SoftFreeTree(&dict);
  int ind=0;
  double max=-1;
  int pos=-1;
  for(;ind<MAX_TOKEN_COUNT && NumberOfDistinctValuesPerColumn[ind]!=0;ind++){
    if(NumberOfDistinctValuesPerColumn[ind]>1 &&
        max<(double)NumberOfFilledColumns[ind]/NumberOfDistinctValuesPerColumn[ind] &&
        (double)NumberOfFilledColumns[ind]/LineCounter>PERCENT_OF_FILL
      ){
      max=(double)NumberOfFilledColumns[ind]/NumberOfDistinctValuesPerColumn[ind];
      pos=ind;
    }
  }
  return pos;
}

void DoClustersForTreeNode(tree ClustersTree,void* par){
  if(par==NULL || ClustersTree==NULL)return;
  ListOfClusters clust=*(ListOfClusters*)par;
  ListOfClusters* head=(ListOfClusters*)par;
  ListOfLines** pCurrLine=NULL;
  if(clust==NULL){ //if this is the first cluster alloc it
    *head=(ClusterNode*)malloc(sizeof(ClusterNode));
    clust=*head;
    if(clust==NULL){
      fprintf(stderr,"Out of Memory!\n");
      ReleaseFiles();
      exit(-2);
    }
    pCurrLine=&(clust->Content);
    //next cluster
    clust->NextCluster=NULL;
    clust->FormerCluster=NULL;
    clust->ClustersLeft=1;
  }
  else{ //is this a former, already made cluster?
    //go to the last cluster in the list
    clust->ClustersLeft++;
    ClusterNode* CurrentCluster=clust;
    while(CurrentCluster->NextCluster!=NULL)CurrentCluster=CurrentCluster->NextCluster;
    CurrentCluster->NextCluster=(ClusterNode*)malloc(sizeof(ClusterNode));
    if(CurrentCluster->NextCluster==NULL){
      fprintf(stderr,"Out of Memory!\n");
      ReleaseFiles();
      exit(-2);
    }
    pCurrLine=&(CurrentCluster->NextCluster->Content);
    //next cluster
    CurrentCluster->NextCluster->NextCluster=NULL;
    CurrentCluster->NextCluster->FormerCluster=CurrentCluster;
  }
  //this is the next line to insert to our cluster
  int i=0;
  ListOfLines** CurrLine=pCurrLine;
  for(;i<HASH_TABLE_SIZE;i++){
    ListOfPositions* CurrentElem=ClustersTree->HashTable[i];
    while(CurrentElem!=NULL){
      *CurrLine=((ListOfLines*)CurrentElem->Data);
      CurrentElem=CurrentElem->Next;
      CurrLine=&((*CurrLine)->NextLine);
    }
  }
  //close the list of lines
  *CurrLine=NULL;
}

ListOfClusters Split(ClusterNode clust,int Position,tree MainDict){
  if(MainDict==NULL)return NULL;
  key key;
  key.Descriptor=NULL;
  ListOfClusters clusters=NULL;
  tree dict=NULL;
  ListOfLines* CurrLine=clust.Content;
  while(CurrLine!=NULL && CurrLine->WordList!=NULL){
    TokenDescriptor* TokDesc=GetElem(CurrLine->WordList,Position);
    key.data=(intptr_t)CurrLine;
    //is this column filled? And is this a Number token?
    if(TokDesc==NULL || TokDesc->TypeOfToken==Number){
      if(TokDesc==NULL){
        result res;
        Search(MainDict,"+n\0",&res);
        key.Descriptor=res.head->DescriptorOfToken;
      }
      else{
        //all number tokens should be in the same cluster
        result res;
        Search(MainDict,"+d\0",&res);
        key.Descriptor=res.head->DescriptorOfToken;
      }
    }
    else{
      key.Descriptor=TokDesc;
    }
    SoftInsertElem(&dict,&key);
    CurrLine=CurrLine->NextLine;
  }
  //now we just need a tree walk to know which lines are in
  //the same cluster
  TreeWalk(dict,DoClustersForTreeNode,(void*)&clusters);
  SoftFreeTree(&dict);
  ListOfClusters CurrClust=NULL;
  CurrClust=clusters;
  while(CurrClust!=NULL){
    CurrClust->goodness=CalcGoodness(CurrClust,MainDict);
    CurrClust=CurrClust->NextCluster;
  }
  if(Position==-1 && clusters!=NULL)clusters->goodness=1;
  return clusters;
}

void PrintClusters(ListOfClusters Clust,FILE* file){
  if(Clust==NULL)return;
  ListOfClusters ptrCurrClust=Clust;
  while(Clust->FormerCluster!=NULL)Clust=Clust->FormerCluster;
  ptrCurrClust=Clust;
  int i=0;
  while(ptrCurrClust!=NULL){
    fprintf(file,"\n%d. Cluster goodness: %f :\n",i,ptrCurrClust->goodness);
    PrintLines(ptrCurrClust->Content,file);
    ptrCurrClust=ptrCurrClust->NextCluster;
    i++;
  }
}

void CountCommon(tree t,void* par){
  if(par==NULL || t==NULL)return;
  CountFuncPar* act=(CountFuncPar*)par;
  int count=SumElem(t);
  if(count>=act->LineNumberInThisCluster){
    act->DistinctValueCount++;
#ifdef DEBUG
    printf("Common word found: %s\n",t->DescriptorOfToken->TokenString);
#endif
  }
}

double CalcGoodness(ListOfClusters clust,tree MainDict){
  if(clust==NULL || MainDict==NULL)return (double)0;
  double AvgLen=0;
  int lineno=0;
  tree dict=NULL;
  ListOfLines* CurrLine=clust->Content;
  while(CurrLine!=NULL){
    ListOfWords* CurrWord=CurrLine->WordList;
    while(CurrWord!=NULL){
      key key;
      key.Descriptor=CurrWord->TokDesc;
      key.data=1;
      SoftInsertElem(&dict,&key);
      CurrWord=CurrWord->NextWord;
    }
    AvgLen+=CurrLine->WordCounter;
    CurrLine=CurrLine->NextLine;
    lineno++;
  }
  AvgLen/=lineno;
  CountFuncPar par;
  par.DistinctValueCount=0;
  par.LineNumberInThisCluster=lineno;
  TreeWalk(dict,CountCommon,(void*)&par);
  SoftFreeTree(&dict);
  return (double)par.DistinctValueCount/AvgLen;
}

ClusterNode* PutClusters(ClusterNode** old,ListOfClusters new){
  if(old==NULL)return NULL;
  ClusterNode* tmp=*old;
  if(new==NULL)return *old;
  ClusterNode* CurrCluser=new;
  while(CurrCluser->NextCluster!=NULL)CurrCluser=CurrCluser->NextCluster;
  CurrCluser->NextCluster=tmp->NextCluster;
  if(tmp->NextCluster!=NULL)(*old)->NextCluster->FormerCluster=CurrCluser;
  if(tmp->FormerCluster!=NULL) (*old)->FormerCluster->NextCluster=new;
  new->FormerCluster=tmp->FormerCluster;
  free(*old);
  return new;
}

void FreeClusters(ListOfClusters Clusters,logic FreeLines){
  if(Clusters==NULL)return;
  while(Clusters->FormerCluster!=NULL)Clusters=Clusters->FormerCluster;
  while(Clusters!=NULL){
    ClusterNode* temp=Clusters->NextCluster;
    if(FreeLines==true)DeleteLines(&(Clusters->Content));
    free(Clusters);
    Clusters=temp;
  }
}

ListOfClusters* Distribute(ListOfClusters clusts,int NumOfCPUS){
  if(clusts==NULL)return NULL;
  int len=ceil(clusts->ClustersLeft/NumOfCPUS);
  //an array for all the cores indexed by core numbers
  ListOfClusters* ArrayOfClusters=NULL;
  ArrayOfClusters=(ListOfClusters*)malloc(sizeof(ListOfClusters)*NumOfCPUS);
  if(ArrayOfClusters==NULL){
    fprintf(stderr,"Out Of Memory!\n");
    ReleaseFiles();
    exit(-2);
  }
  memset(ArrayOfClusters,'\0',sizeof(ListOfClusters)*NumOfCPUS);
  int ClusterCounter=1;
  int CPUCounter=1;
  ClusterNode* CurrCluster=clusts;
  ArrayOfClusters[0]=clusts;
  while(CPUCounter<NumOfCPUS && CurrCluster!=NULL){
    if(ClusterCounter==len){
      ClusterCounter=0;
      ArrayOfClusters[CPUCounter]=CurrCluster->NextCluster;
      CPUCounter++;
      CurrCluster->NextCluster->FormerCluster=NULL;
      CurrCluster->NextCluster=NULL;
    }
    CurrCluster=CurrCluster->NextCluster;
    ClusterCounter++;
  }
  return ArrayOfClusters;
}

void GenTemplate(ListOfClusters clust,tree dict){
  if(clust==NULL || dict==NULL)return;
  ClusterNode* CurrClust=clust;
  while(CurrClust->FormerCluster!=NULL)CurrClust=CurrClust->FormerCluster;
  //iterate all the clusters
  while(CurrClust!=NULL){
    ListOfLines* CurrLine=CurrClust->Content;
    int NumberOfDistinctValuesPerColumn[MAX_TOKEN_COUNT];
    memset(NumberOfDistinctValuesPerColumn,'\0',sizeof(int)*MAX_TOKEN_COUNT);
    int NumberOfFilledColumns[MAX_TOKEN_COUNT];
    memset(NumberOfFilledColumns,'\0',sizeof(int)*MAX_TOKEN_COUNT);
    int GlobLineno=0;
    tree LocalDict=NULL;
    ListOfLines* FirstLineInCluster=CurrClust->Content;
    //iterate the lines inside the cluster
    while(CurrLine!=NULL){
      int TokenCounter=0;
      TokenDescriptor* Descriptor;
      //find those columns, where there is only one distinct word
      for(;TokenCounter<MAX_TOKEN_COUNT;TokenCounter++){
        Descriptor=GetElem(CurrLine->WordList,TokenCounter);
        if(Descriptor==NULL){//skip, because now we can assume the remainder is +n
          break;
        }
        key key;
        key.data=TokenCounter;
        key.Descriptor=Descriptor;
        tree tmp=SoftInsertElem(&LocalDict,&key);
        ListOfPositions* Elem=SearchElem(tmp->HashTable,TokenCounter);
        //is this the first appearance of this token in this position?
        if(Elem!=NULL && Elem->Counter==1){
          NumberOfDistinctValuesPerColumn[TokenCounter]++;
        }
        NumberOfFilledColumns[TokenCounter]++;
      }
      GlobLineno++;
      CurrLine=CurrLine->NextLine;
    }
    //this is the template that will represent the cluster after the function return
    ListOfLines* templ=NewLine(NULL);
    int WordCounter=0;
    for(;WordCounter<MAX_TOKEN_COUNT && NumberOfDistinctValuesPerColumn[WordCounter]!=0;WordCounter++){
      if(NumberOfFilledColumns[WordCounter]==GlobLineno){ //isn't it n+
        if(NumberOfDistinctValuesPerColumn[WordCounter]>1){//is it constant?
          TokenDescriptor* TokDesc=GetElem(FirstLineInCluster->WordList,WordCounter);
          if(TokDesc->TypeOfToken!=Number){
            result res;
            Search(dict,"*\0",&res);
            AddWord(&templ,res.head->DescriptorOfToken);
          }
          else{
            result res;
            Search(dict,"+d\0",&res);
            AddWord(&templ,res.head->DescriptorOfToken);
          }
        }
        else{
          AddWord(&templ,GetElem(FirstLineInCluster->WordList,WordCounter));
        }
      }
      else{
        result res;
        Search(dict,"+n\0",&res);
        AddWord(&templ,res.head->DescriptorOfToken);
        //UnsetTokenRef(FirstLineInCluster,WordCounter);
        break;
      }
    }
    CurrClust->Content=templ;
    DeleteLines(&FirstLineInCluster);
    CurrClust=CurrClust->NextCluster;
    SoftFreeTree(&LocalDict);
  }
}

double GetGoodness(ClusterNode clust1,ClusterNode clust2){
  if(clust1.Content==NULL || clust2.Content==NULL)return (double)0;
  ListOfWords* CurrWord1=clust1.Content->WordList;
  ListOfWords* CurrWord2=clust2.Content->WordList;
  int LineLen=0;
  int CommonWordCounter=0;
  while(CurrWord1!=NULL || CurrWord2!=NULL){ //let's compare the two lines
    if(CurrWord1!=NULL && CurrWord2!=NULL && strcmp(CurrWord1->TokDesc->TokenString,CurrWord2->TokDesc->TokenString)==0){
      CommonWordCounter++;
    }
    if(CurrWord1!=NULL){
      LineLen++;
      CurrWord1=CurrWord1->NextWord;
    }
    if(CurrWord2!=NULL){
      LineLen++;
      CurrWord2=CurrWord2->NextWord;
    }
  }
  return (double)CommonWordCounter/(double)(LineLen/2);
}

void JoinClusters(ClusterNode* clust1,ClusterNode* clust2){
  if(clust1->Content==NULL || clust2->Content==NULL)return;
  ListOfLines* LastLine=clust1->Content;
  while(LastLine->NextLine!=NULL){
    if(LastLine==clust2->Content)return;
    LastLine=LastLine->NextLine;
  }
  //this time we check if still the last element is different than the cluster
  if(LastLine==clust2->Content)return;
  LastLine->NextLine=clust2->Content;
  clust2->Content=NULL;
}
