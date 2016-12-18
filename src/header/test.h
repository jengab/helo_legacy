#ifndef TEST_H
#define TEST_H

#include <string.h>
#include "lists.h"
#include "clust.h"
#include "tree.h"



/*If you like more separated coding style(only prototypes in header, and implementation in .c files)
 * you may try to copy all functions here. The only reason I didn't do it, is that actually gcc seemed to fail
 * on this, but maybe the problem had another source. But you should know that if you have some compile problems
 * probably you should rollback all your changes unfortunately. functions in this file all represent a unit test
 * officially they should be called by mu_run_test macro, but in order to do a specific run sometimes unit test
 * maybe called from the code as a function, please note that this is not the official solution. For the official
 * way of using the framework see MinUnit website.
 */

//all this is the unit testing framework itself, you may copy it from MinUnit website anytime
//it knows only what it should know, but does the job better than anything else

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; if (message!=NULL) return message; } while (0)

int tests_run=0;

//here we test BinaryTreeInsert
//before modifying this test please note that it increments tests_run variable manually, if you use mu_run...
//to run the test you need to delete the last line of the function
static char* test_InsertElem(void){
  /*
   * Important note: However we never call functions like SoftInsert, and SoftFree the content of tree.c
   * can be said to be ok if this test doesn't fail, as SoftFree and SoftInsert are almost the copy of the
   * original tree handling functions, so they should work great if the original versions work great
   */
  tree testDict=NULL;
  TokenDescriptor desc;
  desc.Length=9;
  desc.RefCounter=1;
  desc.TokenString="TestToken";
  desc.TypeOfToken=Word;
  intptr_t data;
  data=1;
  key key0={&desc,data};
  InsertElem(&testDict,&key0);
  //pay attention, condition is long, as the function is also responsible for setting various properties
  //they all must be correct, therefore we insert three different type of tokens with different texts
  //it is to ensure that the tree stores datas as we expect. For further information see the developer's guide
  mu_assert("Couldn't create binary tree!\n",testDict!=NULL);
  TokenDescriptor* RetDesc1=testDict->DescriptorOfToken;
  ListOfPositions* RetElem1=testDict->HashTable[Hash(data)];
  mu_assert("Couldn't insert word to binary tree!\n",
      RetDesc1->Length==9 &&
      RetDesc1->RefCounter==0 &&
      strcmp(RetDesc1->TokenString,"TestToken")==0 &&
      RetDesc1->TypeOfToken==Word &&
      RetElem1->Counter==1 &&
      RetElem1->Data==1
  );

  desc.TypeOfToken=Number;
  desc.Length=2;
  desc.TokenString="24";
  key key2={&desc,data};
  InsertElem(&testDict,&key2);
  result res;
  Search(testDict,"24",&res);
  TokenDescriptor* RetDesc2=res.head->DescriptorOfToken;
  ListOfPositions* RetElem2=res.head->HashTable[Hash(data)];

  mu_assert("Number couldn't be inserted to binary tree!\n",
      res.found==true &&
      RetDesc2->Length==2 &&
      RetDesc2->RefCounter==0 &&
      RetDesc2->TypeOfToken==Number &&
      strcmp(RetDesc2->TokenString,"24")==0 &&
      RetElem2->Counter==1 &&
      RetElem2->Data==1
  );

  desc.TypeOfToken=Hybrid;
  desc.Length=13;
  desc.TokenString="#HybridToken1";
  data=(intptr_t)&desc;
  key key1={&desc,data};
  InsertElem(&testDict,&key1);
  Search(testDict,"#HybridToken1",&res);
  TokenDescriptor* RetDesc3=res.head->DescriptorOfToken;
  ListOfPositions* RetElem3=res.head->HashTable[Hash(data)];
  mu_assert("Hybrid token couldn't be inserted to binary tree!\n",
      res.found==true &&
      RetDesc3->Length==13 &&
      RetDesc3->RefCounter==0 &&
      RetDesc3->TypeOfToken==Hybrid &&
      strcmp(RetDesc3->TokenString,"#HybridToken1")==0 &&
      RetElem3->Counter==1 &&
      RetElem3->Data==(intptr_t)&desc
  );
  TokenDescriptor TokDesc;
  TokDesc.TokenString="*\0";
  TokDesc.TypeOfToken=Word;
  TokDesc.Length=1;
  data=1;
  key key4;
  result res4;
  key4.Descriptor=&TokDesc;
  key4.data=-1;
  InsertElem(&testDict,&key4);
  Search(testDict,"*\0",&res4);
  TokenDescriptor* RetDesc4=res4.head->DescriptorOfToken;
  ListOfPositions* RetElem4=res4.head->HashTable[Hash(-1)];

  mu_assert("* couldn't be inserted to binary tree!\n",
      res4.found==true &&
      RetDesc4->Length==1 &&
      RetDesc4->RefCounter==0 &&
      RetDesc4->TypeOfToken==Word &&
      strcmp(RetDesc4->TokenString,"*")==0 &&
      RetElem4->Counter==1 &&
      RetElem4->Data==-1
  );

  key4.Descriptor->TokenString="+d\0";
  key4.Descriptor->TypeOfToken=Number;
  key4.Descriptor->Length=2;
  InsertElem(&testDict,&key4);
  Search(testDict,"+d\0",&res4);
  RetDesc4=res4.head->DescriptorOfToken;
  RetElem4=res4.head->HashTable[Hash(-1)];


  mu_assert("+d couldn't be inserted to binary tree!\n",
        res4.found==true &&
        RetDesc4->Length==2 &&
        RetDesc4->RefCounter==0 &&
        RetDesc4->TypeOfToken==Number &&
        strcmp(RetDesc4->TokenString,"+d")==0 &&
        RetElem4->Counter==1 &&
        RetElem4->Data==-1
  );

  key4.Descriptor->TokenString="+n\0";
  key4.Descriptor->TypeOfToken=Word;
  key4.Descriptor->Length=2;
  InsertElem(&testDict,&key4);
  InsertElem(&testDict,&key4);
  key4.data=2;
  InsertElem(&testDict,&key4);
  Search(testDict,"+n\0",&res4);
  RetDesc4=res4.head->DescriptorOfToken;
  RetElem4=res4.head->HashTable[Hash(-1)];
  ListOfPositions* RetNextElem4=res4.head->HashTable[Hash(key4.data)];
  while(RetNextElem4->Next!=NULL)RetNextElem4=RetNextElem4->Next;

  mu_assert("+n couldn't be inserted to binary tree!\n",
        res4.found==true &&
        RetDesc4->Length==2 &&
        RetDesc4->RefCounter==0 &&
        RetDesc4->TypeOfToken==Word &&
        strcmp(RetDesc4->TokenString,"+n")==0 &&
        RetElem4->Counter==2 &&
        RetElem4->Data==-1 &&
        RetNextElem4!=NULL &&
        RetNextElem4->Counter==1 &&
        RetNextElem4->Data==2
  );

  //effective test of Search and InsertElem are done, now we try to test deallocator functions

  DestroyStatics(testDict,NULL);
  mu_assert("DestroyStatics fails ListOfElems==NULL assertion!\n",testDict->HashTable[0]==NULL &&
      testDict->HashTable[HASH_TABLE_SIZE-1]==NULL
  );

  FreeTree(&testDict);

  mu_assert("FreeTree did not free all nodes!\n",testDict==NULL);

  return 0;
}

static char* test_parse(void){
  char hybrid1[4]="h#i\0";
  char hybrid2[4]="6hi\0";
  char hybrid3[3]="[]\0";
  char hybrid4[5]="0xag\0";
  char num1[3]="-1\0";
  char num2[5]="-1.2\0";
  char num3[4]="1,2\0";
  char num4[8]="0xaAfF1\0";
  char word[5]="negy\0";
  TokenDescriptor TokDesc;
  TokDesc.TokenString=hybrid1;
  parse(&TokDesc);

  mu_assert("h#i is not a hybrid token according to parse()\n",TokDesc.TypeOfToken==Hybrid &&
      TokDesc.Length==3
  );

  GetWord(TokDesc.TokenString,3);

  mu_assert("A hybrid token is badly parsed!\n",strcmp(TokDesc.TokenString,"h")==0);

  TokDesc.TokenString=hybrid2;
  parse(&TokDesc);

  mu_assert("6hi is not a hybrid token according to parse()\n",TokDesc.TypeOfToken==Hybrid &&
      TokDesc.Length==3
  );

  TokDesc.TokenString=GetWord(TokDesc.TokenString,3);

  mu_assert("A hybrid token is badly parsed!\n",strcmp(TokDesc.TokenString,"hi")==0);

  TokDesc.TokenString=hybrid3;
  parse(&TokDesc);

  mu_assert("[] is not a hybrid token according to parse()\n",TokDesc.TypeOfToken==Hybrid &&
      TokDesc.Length==2
  );

  TokDesc.TokenString=GetWord(TokDesc.TokenString,2);

  mu_assert("A hybrid token is badly parsed!\n",strcmp(TokDesc.TokenString,"")==0);

  TokDesc.TokenString=hybrid4;
  parse(&TokDesc);

  mu_assert("0xag is not a hybrid token according to parse()\n",TokDesc.TypeOfToken==Hybrid &&
    TokDesc.Length==4
  );

  TokDesc.TokenString=num1;
  parse(&TokDesc);

  mu_assert("-1 is not a number token according to parse()\n",TokDesc.TypeOfToken==Number &&
      TokDesc.Length==2
  );

  TokDesc.TokenString=num2;
  parse(&TokDesc);

  mu_assert("-1.2 is not a number token according to parse()\n",TokDesc.TypeOfToken==Number &&
      TokDesc.Length==4
  );

  TokDesc.TokenString=num3;
  parse(&TokDesc);

  mu_assert("1,2 is not a number token according to parse()\n",TokDesc.TypeOfToken==Number &&
      TokDesc.Length==3
  );

  TokDesc.TokenString=num4;
  parse(&TokDesc);

  mu_assert("0xaAfF1 is not a number token according to parse()\n", TokDesc.TypeOfToken==Number //&&
    //TokDesc.Length==7
  );

  TokDesc.TokenString=word;
  parse(&TokDesc);

  mu_assert("negy is not a word token according to parse()\n",TokDesc.TypeOfToken==Word &&
      TokDesc.Length==4
  );
  return 0;
}

//we did the trivial part of testing, now we need to test functions that use a complicated inside representation
//therefore we always assume that our well specified test files are in the project directory
//besides we will sign if we couldn't open our test files(Can it be a problem of insufficient privileges also?)

/*static char* test_Read(void){
  FILE* ifile=NULL;
  FILE* ofile=NULL;
  if((ifile=fopen("user.log","rb"))==NULL || (ofile=fopen("user.out","wb"))==NULL){
    fprintf(stderr,"Cannot open input or output file!\n");
    exit(-1);
  }
  Result par;
  par.LeaveHeader=true;
  par.ifile=ifile;
  int NumberOfDistinctValuesPerColumn[MAX_TOKEN_COUNT];
  int NumberOfFilledColumns[MAX_TOKEN_COUNT];
  memset(NumberOfDistinctValuesPerColumn,'\0',MAX_TOKEN_COUNT*sizeof(int));
  memset(NumberOfFilledColumns,'\0',MAX_TOKEN_COUNT*sizeof(int));
  int TotalLineCounter=0;
  tree dict=NULL;
  ListOfLines* FileContent=NULL;
  par.NumberOfDistinctValuesPerColumn=NumberOfDistinctValuesPerColumn;
  par.NumberOfFilledColumns=NumberOfFilledColumns;
  par.TotalLineCounter=&TotalLineCounter;
  par.dict=&dict;
  par.file=&FileContent;
  ReadFile(par);
  //now the linked lists are initialized, we should check them whether they are correct
  //we can test together AddWord, NewLine and GetElem functions.
  fclose(ifile);
  if((ifile=fopen("user.log","rb"))==NULL){
    fprintf(stderr,"Input file could not be opened!\n");
    exit(-1);
  }
  char FirstLine[MAX_LINE_LEN];
  char LastLine[MAX_LINE_LEN];
  memset(FirstLine,'\0',sizeof(char)*MAX_LINE_LEN);
  memset(LastLine,'\0',sizeof(char)*MAX_LINE_LEN);
  fgets(FirstLine,MAX_LINE_LEN,ifile);
  TokenDescriptor* TokDescs1[MAX_TOKEN_COUNT];
  TokenDescriptor* TokDescs2[MAX_TOKEN_COUNT];
  memset(TokDescs1,'\0',sizeof(TokenDescriptor*)*MAX_TOKEN_COUNT);
  memset(TokDescs2,'\0',sizeof(TokenDescriptor*)*MAX_TOKEN_COUNT);
  int NumberOfLines=0;
  int NumberOfListLines=0;
  while(fgets(LastLine,MAX_LINE_LEN,ifile)!=NULL)NumberOfLines++;
  ListOfLines* act=FileContent;
  while(act->NextLine!=NULL){
    NumberOfListLines++;
    act=act->NextLine;
  }

  //pay attention! This assertion is not necessarily correct for all input, as lines that are shorter than
  //four tokens are not stored, but they are present in the file.
  mu_assert("The input file is not correctly stored!\n",NumberOfListLines==NumberOfLines);

  tokenize(FirstLine,TokDescs1,0);
  tokenize(LastLine,TokDescs2,0);
  logic IsSame1=true;
  logic IsSame2=true;
  TokenDescriptor* CurrTok=GetElem(FileContent->WordList,0);
  int i=0;
  int WordCounter=0;
  for(;i<MAX_TOKEN_COUNT && CurrTok!=NULL && TokDescs1[i]!=NULL;i++){
    if(TokDescs1[i]->TypeOfToken==Hybrid){
      TokDescs1[i]->TokenString=
          GetWord(TokDescs1[i]->TokenString,TokDescs1[i]->Length);
      TokDescs1[i]->Length=strlen(TokDescs1[i]->TokenString);
    }
    if(TokDescs1[i]->Length==0)continue;
    if(    strcmp(TokDescs1[i]->TokenString,CurrTok->TokenString)!=0 ||
        TokDescs1[i]->Length!=CurrTok->Length
    ){
      IsSame1=false;
    }
    CurrTok=GetElem(FileContent->WordList,++WordCounter);
  }
  if((i+1<MAX_TOKEN_COUNT && TokDescs1[i+1]!=NULL) || CurrTok!=NULL){
    IsSame1=false;
  }

  for(;i<MAX_TOKEN_COUNT;i++){
      if(strcmp(TokDescs1[i]->TokenString,CurrWord->TokDesc->TokenString)!=0 ||
          TokDescs1[i]->Length!=CurrWord->TokDesc->Length ||
          TokDescs1[i]->TypeOfToken!=CurrWord->TokDesc->TypeOfToken ||
          TokDescs1[i]->RefCounter!=CurrWord->TokDesc->RefCounter
      ){
        IsSame1=false;
      }
  }

  return 0;
}*/

//this is a test "suite", you may make similar functions to create another suite
static char* all_tests(){
  mu_run_test(test_InsertElem);
  mu_run_test(test_parse);
  //mu_run_test(test_Read);
  return 0;
}

#endif
