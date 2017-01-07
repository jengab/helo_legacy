#include "lists.h"
#include "tree.h"
#include <stdlib.h>
#include <string.h>

//some useful functions for general string handling
//we need to delete it after being needless!!!
TokenDescriptor* parse(TokenDescriptor* TokDesc){
  if(TokDesc==NULL)return NULL;
  TokDesc->TypeOfToken=Word;
  //only call HERE strlen, after this it MUSTN'T be called(It's just because of performance issues)
  char* Token=TokDesc->TokenString;
  TokDesc->Length=strlen(Token);
  int Index=0;
  logic accept=true;
  logic IsHex=false;
  if(isdigit(Token[0]) || Token[0]=='-' || Token[0]=='+'){
    TokDesc->TypeOfToken=Number;
  }
  Index++;
  //if str[0] is not alphanumerical return Hybrid
  if(!isalnum(Token[0]) && TokDesc->TypeOfToken!=Number){
    TokDesc->TypeOfToken=Hybrid;
    return TokDesc;
  }
  if(Token[0]=='0' && Token[1]=='x'){
    TokDesc->TypeOfToken=Number;
    Index=2;
    IsHex=true;
  }
  while(Index<TokDesc->Length){
    if(Index==TokDesc->Length-1 && Token[Index]=='\n')return TokDesc;
    if(IsHex==true && (!isalnum(Token[Index]) || (Token[Index]>'F' && Token[Index]<'a') || Token[Index]>'f')){
      TokDesc->TypeOfToken=Hybrid;
      return TokDesc;
    }
    //current state is number, but received alpha
    if(IsHex!=true && TokDesc->TypeOfToken==Number && isalpha(Token[Index])){
      TokDesc->TypeOfToken=Hybrid;
      return TokDesc;
    }
    //current state is word, but received digit
    if(TokDesc->TypeOfToken==Word && isdigit(Token[Index])){
      TokDesc->TypeOfToken=Hybrid;
      return TokDesc;
    }
    //hybrid return for not alphanumerical input
    if(!isalnum(Token[Index])){
      if(TokDesc->TypeOfToken==Number && accept==true && (Token[Index]=='.' || Token[Index]==',')){
        accept=false;
      }
      else{
        TokDesc->TypeOfToken=Hybrid;
        return TokDesc;
      }
    }
    Index++;
  }
  return TokDesc;
}

int tokenize(char* Line,TokenDescriptor** tokens, int StartIndex){
  if(tokens==NULL || Line==NULL)return 0;
  int CurrentPosition=0;
  if(tokens[0]==NULL)tokens[0]=(TokenDescriptor*)malloc(sizeof(TokenDescriptor));
  if(tokens[0]==NULL){
    fprintf(stderr,"Out of Memory!\n");
    ReleaseFiles();
    exit(-2);
  }
  //set the first token's position from line
  tokens[0]->TokenString=&Line[StartIndex];
  //How many tokens have we already made?(return value)
  int TokenCounter=0;
  //upto MAX_LINE_LEN or the end of line(\0)
  while(CurrentPosition+StartIndex<MAX_LINE_LEN && Line[CurrentPosition+StartIndex]!='\0'){
    //StartIndex is senseless here, do not mind about it, it's not used on calling from main
    if(Line[CurrentPosition+StartIndex]==' ' || Line[CurrentPosition+StartIndex]=='\t' || Line[CurrentPosition+StartIndex]=='\n'){
      //We found ' '(space) or tab or linefeed, this means the end of current token and
      //the beginning of a new one
      if(TokenCounter+1>MAX_TOKEN_COUNT)return TokenCounter; //Can we still make TokenCouner bigger, or is it maximal?
      //let's replace WhiteSpace with \0(end of string)
      Line[CurrentPosition+StartIndex]='\0';
      //set initial values(most important step of tokenizing)
      tokens[TokenCounter]=parse(tokens[TokenCounter]);
      //eat up remaining WhiteSpaces
      while(Line[CurrentPosition+StartIndex]==' '
          || Line[CurrentPosition+StartIndex]=='\t'
          || Line[CurrentPosition+StartIndex]=='\n')CurrentPosition++;
      //Do we have to make a new structure?
      if(tokens[++TokenCounter]==NULL){
        tokens[TokenCounter]=(TokenDescriptor*)malloc(sizeof(TokenDescriptor));
        if(tokens[TokenCounter]==NULL){
          fprintf(stderr,"Out Of Memory\n");
          ReleaseFiles();
          exit(-2);
        }
      }
      //set the beginning of the new token(It still doesn't end with \0, we need to set it later)
      tokens[TokenCounter]->TokenString=&Line[CurrentPosition+StartIndex+1];
    }
    //algorithm is not sensitive for upper-lower case differences, we assume that all chars are lower case
    //and here we do the conversion to provide the global truth of our assume
    Line[CurrentPosition+StartIndex]=tolower(Line[CurrentPosition+StartIndex]);
    CurrentPosition++; //step to the following char
  }
  //we need to parse even the last token
  tokens[TokenCounter]=parse(tokens[TokenCounter]);
  return TokenCounter;
}

char* GetWord(char* str,int len){
  if(str==NULL)return NULL;
  int j=0;
  while(j<len && !isalpha(str[j])){
    j++;
  }
  str=&str[j];
  while(j<len){
    if(!isalpha(str[j])){
      str[j]='\0';
      break;
    }
    j++;
  }
  return str;
}


void FreeTokens(TokenDescriptor** TokDesc){
  if(TokDesc==NULL)return;
  TokenDescriptor* CurrTok=TokDesc[0];
  int TokenIndex=0;
  while(TokenIndex<MAX_TOKEN_COUNT && CurrTok!=NULL){
    free(CurrTok);
#ifdef DEBUG
    //setting of pointer to NULL is needless, as we don't use this structure from the program anymore
    //but in order to make successful unit tests we set these pointers to NULL(to decided whether they
    //were freed.
    CurrTok=NULL;
#endif
    CurrTok=TokDesc[++TokenIndex];
  }
}


//here begins the implementation of line list functions.

ListOfLines* NewLine(ListOfLines* last){
  ListOfLines* CurrLine=(ListOfLines*)malloc(sizeof(ListOfLines));
  if(CurrLine==NULL){
    ReleaseFiles();
    fprintf(stderr,"Out of Memory\n");
    exit(-2);
  }
  if(last!=NULL){
    last->NextLine=CurrLine;
  }
  CurrLine->NextLine=NULL;
  CurrLine->WordList=NULL;
  return CurrLine;
}

void DeleteLines(ListOfLines** l){
  if(*l==NULL || l==NULL)return;
  ListOfLines* list=*l;
  while(list!=NULL){
    //delete word list first
    ListOfWords* act=list->WordList;
    while(act!=NULL){
      ListOfWords* next=act->NextWord;
      free(act);
      act=next;
    }
    ListOfLines* temp=list->NextLine;
    free(list);
    list=temp;
  }
  *l=NULL;
}

void PrintLines(ListOfLines* Lines,FILE* OutputFile){
  if(Lines==NULL || OutputFile==NULL)return;
  ListOfLines* CurrentLine=Lines;
  while(CurrentLine!=NULL){
    ListOfWords* CurrentWord=CurrentLine->WordList;
    while(CurrentWord!=NULL){
      if(CurrentWord->TokDesc!=NULL && CurrentWord->TokDesc->TokenString!=NULL){
        fprintf(OutputFile,"%s ",CurrentWord->TokDesc->TokenString);
      }
      CurrentWord=CurrentWord->NextWord;
    }
    fprintf(OutputFile,"\n");
    CurrentLine=CurrentLine->NextLine;
  }
}

void AddWord(ListOfLines** ptrLine,TokenDescriptor* TokDesc){
  if(TokDesc==NULL || ptrLine==NULL || *ptrLine==NULL)return;
  ListOfWords* WordList=(*ptrLine)->WordList;
  if(WordList==NULL){//first word in this line
    (*ptrLine)->WordList=(ListOfWords*)malloc(sizeof(ListOfWords));
    WordList=(*ptrLine)->WordList;
    if(WordList==NULL){
      fprintf(stderr,"Out of Memory\n");
      ReleaseFiles();
      exit(-2);
    }
    WordList->NextWord=NULL;
    WordList->TokDesc=TokDesc;
    TokDesc->RefCounter++;
    (*ptrLine)->WordCounter=1;
    return;
  }
  ListOfWords* act=(*ptrLine)->WordList;
  while(act->NextWord!=NULL)act=act->NextWord;
  act->NextWord=(ListOfWords*) malloc(sizeof(ListOfWords));
  if(act->NextWord==NULL){
    fprintf(stderr,"Out of Memory\n");
    ReleaseFiles();
    exit(-2);
  }
  act->NextWord->TokDesc=TokDesc;
  TokDesc->RefCounter++;
  act->NextWord->NextWord=NULL;
  (*ptrLine)->WordCounter++;
}

TokenDescriptor* GetElem(ListOfWords* WordList,int WordIndex){
  int WordCounter=0;
  ListOfWords* CurrentWord=WordList;
  while(CurrentWord!=NULL && WordCounter!=WordIndex){
    CurrentWord=CurrentWord->NextWord;
    WordCounter++;
  }
  if(CurrentWord==NULL)return NULL;
  return CurrentWord->TokDesc;
}

void PrintLine(ListOfLines* line){
  if(line==NULL)return;
  ListOfWords* actw=line->WordList;
  while(actw!=NULL){
    printf("%s ",actw->TokDesc->TokenString);
    actw=actw->NextWord;
  }
  printf("\n");
}

void ReadFile(Result parameters){
  char buff[MAX_LINE_LEN];
  ListOfLines* CurrentLine=NULL;
  TokenDescriptor* tok[MAX_TOKEN_COUNT];
  memset(tok,'\0',MAX_TOKEN_COUNT*sizeof(TokenDescriptor*));
  while(fgets(&buff[0],MAX_LINE_LEN,parameters.ifile)!=NULL){
    //got the item(one line), tokenize it first
    int NumberOfTokens=tokenize(buff,tok,0);
    if(NumberOfTokens<4 && parameters.LeaveHeader==false) continue;
    ListOfLines* TmpLine=CurrentLine;
    CurrentLine=NewLine(CurrentLine);
    if(TmpLine==NULL) *(parameters.file)=CurrentLine;
    int TokenCounter;
    int AddedTokenCounter=0;
    int EmptyTokens=0;
    //for all word inside the line
    for(TokenCounter=0;TokenCounter<=NumberOfTokens;TokenCounter++){
        //we assume that the first four tokens of the messages are the header, so we throw them away
        //but with a 4th header command line option you may tell the program to turn of filter
        //empty strings are not counted in the first four elements
        if(TokenCounter<4+EmptyTokens && parameters.LeaveHeader==false){
          if(tok[TokenCounter]->Length==0)EmptyTokens++;
          continue;
        }
        if(tok[TokenCounter]->TypeOfToken==Hybrid){
          tok[TokenCounter]->TokenString=GetWord(tok[TokenCounter]->TokenString,tok[TokenCounter]->Length);
          tok[TokenCounter]->Length=strlen(tok[TokenCounter]->TokenString);
        }
        //set the appearance number of the token
        if(tok[TokenCounter]->Length>0){
          key key;
          key.Descriptor=tok[TokenCounter];
          key.data=AddedTokenCounter;
          node* temp=InsertElem(parameters.dict,&key);
          ListOfPositions* descript=SearchElem(temp->HashTable,AddedTokenCounter);
          //Numbers should decrease the mean of the column
          if(descript!=NULL && descript->Counter==1){
            //we have just put the word in this position
            parameters.NumberOfDistinctValuesPerColumn[AddedTokenCounter]++;
          }
          if(temp->DescriptorOfToken->TypeOfToken!=Number)parameters.NumberOfFilledColumns[AddedTokenCounter]++;
          AddWord(&CurrentLine,temp->DescriptorOfToken);
          AddedTokenCounter++;
        }
      }
      if(CurrentLine->WordCounter==0){
        if((*parameters.file)==CurrentLine)(*parameters.file)=NULL;
        if(TmpLine!=NULL)TmpLine->NextLine=NULL;
        DeleteLines(&CurrentLine);
        CurrentLine=TmpLine;
      }
      *(parameters.TotalLineCounter)++;
    }
    FreeTokens(tok);
}


//this function is for freeing needless tokens(that are not referred yet)
//it's in comment, as RefCounter is not used yet, the implementation maybe not totally correct
/*void UnsetTokenRef(ListOfLines LineList,int StartPos){
  ListOfLines CurrLine=LineList;
  while(CurrLine!=NULL){
    ListOfWords CurrWord=*(CurrLine->WordList);
    int WordCounter=0;
    while(CurrWord!=NULL){
      if(WordCounter>=StartPos){
        CurrWord->TokDesc->RefCounter--;
        if(CurrWord->TokDesc->RefCounter==0){
          free(CurrWord->TokDesc->TokenString);
          free(CurrWord->TokDesc);
        }
      }
      CurrWord=CurrWord->NextWord;
      WordCounter++;
    }
    CurrLine=CurrLine->NextLine;
  }
}
*/
