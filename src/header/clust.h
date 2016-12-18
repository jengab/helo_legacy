#ifndef CLUST_H
#define CLUST_H

#include "tree.h"
#include "lists.h"

//what percent of a column should be filled to be able to split there?
#define PERCENT_OF_FILL 0.5
#define MERGE_LIMIT 0.8

//clustering functions

typedef struct _clustnode{
  //here we find the lines included in the cluster
  ListOfLines* Content;
  double goodness;
  struct _clustnode* NextCluster;
  struct _clustnode* FormerCluster;
  int ClustersLeft;
}
ClusterNode;

typedef ClusterNode* ListOfClusters;

typedef struct{
  int LineNumberInThisCluster;
  int DistinctValueCount;
} CountFuncPar;

extern void ReleaseFiles(void);

//returns the split position for one cluster
int GetSplit(ClusterNode clust);

//the most important part of algorithm, it splits one cluster to several others
ListOfClusters Split(ClusterNode clust,int Position,tree MainDict);
void PrintClusters(ListOfClusters c,FILE* OutputFile);
double CalcGoodness(ListOfClusters Clusts,tree MainDict);

//make the new ListOfClusters according to the data stored in the binary tree node
//par should be ListOfClusters*, but should be cast to void* because of compatibility reasons
//note that par should point to the first cluster in the list!
void DoClustersForTreeNode(tree ClustersTree,void* par);
//This function counts the number of common words in the cluster, it needs a prepared
void CountCommon(tree t,void* par);

//replace one, given cluster with a list of clusters
ClusterNode* PutClusters(ClusterNode** old,ListOfClusters new);
void FreeClusters(ListOfClusters cluster,logic FreeLines);

//distribute task among CPU cores
ListOfClusters* Distribute(ListOfClusters clust,int num);

//this function generates template for a given cluster
void GenTemplate(ListOfClusters clust,tree dict);

//calculates goodness for two templates(it's needed for the last step of the algorithm)
double GetGoodness(ClusterNode clust1,ClusterNode clust2);

//Joins the two clusters(the Content of second attribute is set NULL)
void JoinClusters(ClusterNode* clust1,ClusterNode* clust2);

#endif
