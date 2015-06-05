//-*-C++-*-

#include <string.h>
#include "WordStore.h"
#include "SemClass.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

typedef struct SemEntry {
  char* word;
  char* sem;
  SemEntry* next;
} SemEntry;

SemEntry* gSemanticsList;
  
void InitSemantics()
{
  gSemanticsList = NULL;
}

char* sem_class(char* word, int&  semNum, int prevSemNum)
{
  SemEntry* j = gSemanticsList;
  if (j == NULL) return NULL;
  int cmpResult;
  cmpResult = strcmp(word,j->word);
  while (j && (0 > cmpResult)) {
    j = j->next;
    if (j)
      cmpResult = strcmp(word,j->word);
  }
  semNum = 0;
  if (cmpResult > 0)
    return NULL;
  for (int i = 0; j && (i < prevSemNum); i++) 
    j = j->next;
  if (!j)
    return NULL;
  if (0 != strcmp(word,j->word))
    return NULL;
  semNum = prevSemNum + 1;
  return j->sem;
}

int sem_class(char* word, char* sem)
{
  SemEntry* j = gSemanticsList;
  int cmpResult;
  if (j == NULL) return 0;

  cmpResult = strcmp(word,j->word);
  while (j && (0 > cmpResult)) {
    j = j->next;
    if (j)
      cmpResult = strcmp(word,j->word);
  }
  if (!j || (cmpResult > 0)) {
    return 0;
  }
  int cmpResult2 = strcmp(sem, j->sem);
  if (cmpResult2 == 0) {
    return 1;
  }
  while (j && (cmpResult == 0) && (cmpResult2 != 0)) {
    j = j->next;
    if (j) {
      cmpResult = strcmp(word,j->word);
      cmpResult2 = strcmp(sem,j->sem);
    }
  }
  if (j && (0 == cmpResult) && (0 == cmpResult2)) {
    return 1;
  } else {
    return 0;
  }
}


char* same_sem_class(char* word1, char* word2)
{
  // get each sem_class of word1, and check to see if it is a sem_class of
  // word2; if you get a match, return the class they're both in

  char* semclass = NULL;
  int sem_num;
  int class_found = 0;
  semclass = sem_class(word1,sem_num);
  while (!class_found && semclass) {
    // see if class is a semantic class of word2
    if (sem_class(word2,semclass)) {
      class_found = 1;
    } else {
      semclass = sem_class(word1,sem_num,sem_num);
    }
  }
  return semclass;
}

void AddSemPair(char* word,char* sem)
{
  // add the pair to gSemanticsList at the end of the pairs
  // containing word

  SemEntry* i = NULL;
  SemEntry* j = NULL;
  SemEntry* newEntry = new SemEntry;
  newEntry->word = word;
  newEntry->sem = sem;


  if (!gSemanticsList || (0 < strcmp(word,gSemanticsList->word))) {
    newEntry->next = gSemanticsList;
    gSemanticsList = newEntry;
  } else { // searching through the list
    i = gSemanticsList;
    j = gSemanticsList->next;
    while (j && (0 >= strcmp(word,j->word))) {
      i = j;
      j = j->next;
    }
    newEntry->next = j;
    i->next = newEntry;
  }
}

void PrintSemList(ostream& stream) {
  SemEntry *i = gSemanticsList;
  while (i) {
    stream << "sem_class(" << i->word << "," << i->sem << ").\n";
    i = i->next;
  }
}
	      
void ReadSemRules(istream& stream, WordStore& ws)
{
  char c;
  char     buffer[80];
  int       i;
  char*     word = NULL;
  char*     sem = NULL;

 while (stream.good()) {
    stream.get(c);
    while (stream.good() && (c != 's'))
      stream.get(c);
    if (stream.good()) {
      // read a rule
      while (c != '(')
	stream.get(c);
      stream.get(c);
      i = 0;
      if (c == 39) { // we're quoted
	int cont = 1;
	char nextc;
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while (c != ',') {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      word = ws.StoreWord(buffer);
      stream.get(c);
      i = 0;
      if (c == 39) { // we're quoted
	int cont = 1;
	char nextc;
	stream.get(c);
	while (cont) {
	  if (c == 39) {
	    stream.get(nextc);
	    if (nextc == 39) {
	      buffer[i++] = nextc;
	      stream.get(c);
	    } else {
	      stream.putback(nextc);
	      cont = 0;
	    }
	  } else {
	    buffer[i++] = c;
	    stream.get(c);
	  }
	} // while (cont)
	stream.get(c);
      } else { // we're not quoted
	while (c != ')') {
	  buffer[i++] = c;
	  stream.get(c);
	}
      }
      buffer[i] = 0;
      sem = ws.StoreWord(buffer);
      stream.get(c); // read .
      AddSemPair(word,sem); // add the rule
    }
  }
  
}















