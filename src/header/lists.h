#ifndef LISTS_H
#define LISTS_H

#include <stdio.h>
#include "tree.h"
#include <ctype.h>

#define MAX_LINE_LEN 100000
#define MAX_TOKEN_COUNT 1000

typedef struct _words{
  TokenDescriptor* TokDesc;
  struct _words* NextWord;
} ListOfWords;

typedef struct _lines{
  struct _lines* NextLine;
  ListOfWords* WordList;
  int WordCounter;
} ListOfLines;

typedef struct _Result{
  ListOfLines** file;
  int* TotalLineCounter;
  int* NumberOfFilledColumns;
  int* NumberOfDistinctValuesPerColumn;
  logic LeaveHeader;
  FILE* ifile;
  tree* dict;
}Result;



extern void ReleaseFiles(void);

//string handling functions

TokenDescriptor* parse(TokenDescriptor* TokDesc);
int tokenize(char* Line,TokenDescriptor** TokDesc,int StartingPosition);
char* GetWord(char* Token,int len);
void FreeTokens(TokenDescriptor** TokDesc);


//allocs place for a new line and sets all pointers to NULL
ListOfLines* NewLine(ListOfLines* FormerLine);
void AddWord(ListOfLines** pLineList,TokenDescriptor* TokDesc);
//get the WordIndex nth element of a WordList(return is NULL if it doesn't exist)
TokenDescriptor* GetElem(ListOfWords* WordList,int WordIndex);
void DeleteLines(ListOfLines** LineList);
void PrintLines(ListOfLines* LineList,FILE* OutputFile);
void PrintLine(ListOfLines* LineList);
void ReadFile(Result parameters);
//void UnsetTokenRef(ListOfLines LineList,int StartPos);

#endif
