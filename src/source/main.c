#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

//unit test
#include "test.h"

//unix specific includes
#include <unistd.h>
#include <pthread.h>

//#define DEBUG
//#define PROFILE

#ifdef PROFILE
#include <gperftools/heap-profiler.h>
#endif


/*Useful note: in order to understand functions, and used data structures
 * see the own header files, all the functions are briefly defined there by
 * comments. There are some comments in the code that would like to warn you
 * about possibility of committing error. Please always consider these good
 * advices, as you should understand all the details of the code in order to
 * make your hack work there, but you may cause a lot of problem for yourself.
 * I defined some #ifdef DEBUG parts, these maybe useful to see inside storage,
 * feel free to put #define DEBUG at the beginning of files in this case.
 * Additionally the unit tests only run if DEBUG macro is defined, so you may
 * even define it because of this reason.
 */

//to provide parameter for void* thread function
typedef struct{
  ListOfClusters c;
  tree dict;
} threadpar;

//global limit variable, already set when looked up
double lim=0.4;
//files need to be global to be able to close them from all modules
FILE *ifile=NULL;
FILE *ofile=NULL;

void ReleaseFiles(void){
  if(ifile!=NULL)fclose(ifile);
  if(ofile!=NULL)fclose(ofile);
}

void* SplitSubCluster(void* par){
  threadpar *tpar=(threadpar*)par;
  ListOfClusters clust=tpar->c;
  ClusterNode* CurrCluster=clust;
  ClusterNode* RetVal=CurrCluster;
  while(CurrCluster!=NULL){
    if(CurrCluster->goodness>=lim){
      if(CurrCluster->NextCluster==NULL)RetVal=CurrCluster;
      CurrCluster=CurrCluster->NextCluster;
      continue;
    }
    int pos=GetSplit(*CurrCluster);
    ClusterNode* new=Split(*CurrCluster,pos,tpar->dict);
    CurrCluster=PutClusters(&CurrCluster,new);
    if(CurrCluster->goodness>=lim){
      if(CurrCluster->NextCluster==NULL)RetVal=CurrCluster;
      CurrCluster=CurrCluster->NextCluster;
    }
  }
  return (void*)RetVal;
}

int main(int argc,char* argv[]){
#ifdef PROFILE
  HeapProfilerStart("profile");
#endif
#ifdef DEBUG
  char* msg=all_tests();
  if(msg!=0){
    printf("Unit test error:\n%s\nTotal Number of unit tests passed: %d",msg,tests_run-1);
    return -4;
  }
#endif
  int numCPU=sysconf(_SC_NPROCESSORS_ONLN);
  logic LeaveHeader=false;
  printf("Starting Helo! Number of CPU cores: %d\n",numCPU);
  if(argc<3){
    fprintf(stderr,"Usage: %s <input_file> <output_file> [<threshold>] [\"header\"]\n",argv[0]);
    return -1;
  }
  if(argc>3){
    if(argc>4 && strcmp(argv[4],"header")==0){
      LeaveHeader=true;
      lim=atof(argv[3]);
    }
    else{
      if(argc==4){
        if(atof(argv[3])!=0){
          lim=atof(argv[3]);
        }
        else{
          if(strcmp(argv[3],"header")==0){
            LeaveHeader=true;
          }
          else{
            fprintf(stderr,"Unknown 3rd input parameter, did you mean header?\n");
            return -1;
          }
        }
      }
      else{
        fprintf(stderr,"Unknown 4th input parameter, did you mean header?\n");
        return -1;
      }
    }
  }
  if(lim<=0 || lim>1){
    fprintf(stderr,"Wrong threshold value! It must be more than 0, and less, or equal to 1!\n");
    return -3;
  }
  if((ifile=fopen(argv[1],"rb"))==NULL || (ofile=fopen(argv[2],"wb"))==NULL){
    fprintf(stderr,"Opening input or output file failed, cannot continue the process!\n");
    return -2;
  }
  ListOfLines* fileLines=NULL;
  tree dict=NULL;
  int TotalLineCounter=0;
  int NumberOfFilledColumns[MAX_TOKEN_COUNT];
  int NumberOfDistinctValuesPerColumn[MAX_TOKEN_COUNT];
  memset(NumberOfDistinctValuesPerColumn,'\0',sizeof(int)*MAX_TOKEN_COUNT);
  memset(NumberOfFilledColumns,'\0',sizeof(int)*MAX_TOKEN_COUNT);

  Result parameter;
  parameter.LeaveHeader=LeaveHeader;
  parameter.NumberOfDistinctValuesPerColumn=NumberOfDistinctValuesPerColumn;
  parameter.NumberOfFilledColumns=NumberOfFilledColumns;
  parameter.TotalLineCounter=&TotalLineCounter;
  parameter.dict=&dict;
  parameter.file=&fileLines;
  parameter.ifile=ifile;

  ReadFile(parameter);

  TokenDescriptor d={"*\0",Word};
  key k;
  k.Descriptor=&d;
  k.data=-1;
  InsertElem(&dict,&k);
  k.Descriptor->TokenString="+d\0";
  k.Descriptor->TypeOfToken=Number;
  InsertElem(&dict,&k);
  result res;
  Search(dict,"+d\0",&res);
  k.Descriptor->TokenString="+n\0";
  k.Descriptor->TypeOfToken=Word;
  InsertElem(&dict,&k);
  //main dictionary is full, it mustn't be modified yet!!!
  //See comments for DestroyStatics in tree.h
  TreeWalk(dict,DestroyStatics,NULL);
#ifdef DEBUG
  PrintLines(fileLines,stdout);
#endif
  int i;
  double mean=-1;
  int pos=-1;
  for(i=0;i<MAX_TOKEN_COUNT && NumberOfDistinctValuesPerColumn[i]!=0;i++){
    if(NumberOfDistinctValuesPerColumn[i]>1 && (double)NumberOfFilledColumns[i]/TotalLineCounter>=PERCENT_OF_FILL && (double)NumberOfFilledColumns[i]/NumberOfDistinctValuesPerColumn[i]>mean){
      mean=(double)NumberOfFilledColumns[i]/NumberOfDistinctValuesPerColumn[i];
      pos=i;
    }
  }
  ClusterNode first;
  first.Content=fileLines;
  first.goodness=0;
  first.NextCluster=NULL;
  first.FormerCluster=NULL;
  if(pos==-1){  //file is already one cluster
    first.goodness=0;
    GenTemplate(&first,dict);
    PrintClusters(&first,ofile);
    //GenTemplate deleted already original lines, to call DeleteLines for fileLines is a definitely bad
    //idea
    DeleteLines(&first.Content);
    FreeTree(&dict);
    fclose(ifile);
    fclose(ofile);
    printf("Successful run, only found one cluster, for details see output file: %s\n",argv[2]);
    return 0;
  }
  ListOfClusters clust=Split(first,pos,dict);
#ifdef DEBUG
  TreeWalk(dict,PrintNode,NULL);
  PrintClusters(clust,stdout);
#endif
  ListOfClusters* ArrayOfClusters=Distribute(clust,numCPU);
  if(ArrayOfClusters==NULL && clust!=NULL){
    fprintf(stderr,"Out of Memory!\n");
    ReleaseFiles();
    exit(-2);
  }
  pthread_t* thread=(pthread_t*)malloc(sizeof(pthread_t)*numCPU);
  if(thread==NULL){
    fprintf(stderr,"Out of Memory!\n");
    ReleaseFiles();
    exit(-2);
  }
  threadpar* ThreadParameter=(threadpar*)malloc(sizeof(threadpar)*numCPU);
  if(ThreadParameter==NULL){
    fprintf(stderr,"Out of Memory!\n");
    ReleaseFiles();
    exit(-2);
  }
  printf("Beginning of multithreaded run!\n");
  int j=0;
  for(j=0;j<numCPU;j++){
    ThreadParameter[j].c=ArrayOfClusters[j];
    ThreadParameter[j].dict=dict;
    pthread_create(&thread[j],NULL,SplitSubCluster,(void*)&ThreadParameter[j]);
  }
  for(j=0;j<numCPU;j++){
    pthread_join(thread[j],(void**)&ArrayOfClusters[j]);
    //we actually returned one of the last clusters from the thread, so it's better to go the beginning
    //of the list
    if(ArrayOfClusters[j]!=NULL){
      while(ArrayOfClusters[j]->FormerCluster!=NULL)
        ArrayOfClusters[j]=ArrayOfClusters[j]->FormerCluster;
    }
  }
  printf("Multithreaded run is done, making templates...\n");
  for(j=0;j<numCPU;j++){
    GenTemplate(ArrayOfClusters[j],dict);
  }
  printf("Templates are done, joining similar templates...\n");
  i=0;
  int Counter=0;
  //We've arrived to the last step of online clustering
  //yes, this is a quadratic algorithm, but just for templates that are probably few
  for(;i<numCPU;i++){
    ListOfClusters OuterCluster=ArrayOfClusters[i];
    while(OuterCluster!=NULL){
      ListOfClusters actclust=OuterCluster->NextCluster;
      int Index=i+1;
      while(actclust==NULL && Index<numCPU)actclust=ArrayOfClusters[Index++];
      while(actclust!=NULL){
        double goodness=GetGoodness(*OuterCluster,*actclust);
        if(goodness>MERGE_LIMIT){
          JoinClusters(OuterCluster,actclust);
          GenTemplate(OuterCluster,dict);
        }
#ifdef DEBUG
        printf("%d. Distance: %f\n",Counter,goodness);
        PrintLine(OuterCluster->Content);
        PrintLine(actclust->Content);
        printf("\n\n");
        Counter++;
#endif
        actclust=actclust->NextCluster;
        if(actclust==NULL && Index<numCPU)actclust=ArrayOfClusters[Index++];
      }
      OuterCluster=OuterCluster->NextCluster;
    }
  }
  printf("Templates are merged! Writing the result to file...\n");
  for(j=0;j<numCPU;j++){
    PrintClusters(ArrayOfClusters[j],ofile);
    //no need to free clusters anymore(MUSTN'T), this is also true for lines
    FreeClusters(ArrayOfClusters[j],true);
  }
  //it's important which order we call frees, this order is suspected to be good,
  //don't modify it unless you know what you do!!!
  free(ThreadParameter);
  free(thread);
  free(ArrayOfClusters);
  FreeTree(&dict);
  fclose(ifile);
  fclose(ofile);
  printf("Successful run, see the output file (filename: %s)\n",argv[2]);
#ifdef PROFILE
  FILE* file=fopen("profile","wb");
  if(file==NULL)return 1;
  fprintf(file,"%s",GetHeapProfile());
  fclose(file);
  HeapProfilerStop();
#endif
  return 0;
}
