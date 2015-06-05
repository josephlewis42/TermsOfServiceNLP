//-*-C++-*-


#include "utils.h"
#include <stdlib.h>


#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

const char CASEDIFF = 'a' - 'A';

char lowercase(char c)
{
  if ((c <= 'Z') && (c >= 'A'))
    return c + CASEDIFF;
  else return c;
}


// DynamicPtrArray routines

DynamicPtrArray::DynamicPtrArray()
{
  m_PointerArray = (void **) malloc(200 * sizeof(void *));

  m_nAllocatedSize = 200;

  m_nFilledSize = 0;
}


DynamicPtrArray::DynamicPtrArray(int initialSize)
{
  m_PointerArray = (void **) malloc(initialSize * sizeof(void *));
  m_nAllocatedSize = initialSize;
  m_nFilledSize = 0;
}

DynamicPtrArray::~DynamicPtrArray()
{
  free(m_PointerArray);
}

DynamicPtrArray* DynamicPtrArray::Copy()
{
  DynamicPtrArray* result = new DynamicPtrArray(m_nFilledSize);
  for (int i = 0; i < m_nFilledSize; i++) {
    result->Add(m_PointerArray[i]);
  }
  return result;
}

void DynamicPtrArray::Add(void* itemPtr)
{
  if (m_nFilledSize >= m_nAllocatedSize) {
    // we need to grow the array
    void** temp = (void**)realloc(m_PointerArray, (m_nAllocatedSize + 100) * sizeof(void *));
    if (temp) {
      m_PointerArray = temp;
      m_nAllocatedSize = m_nAllocatedSize + 100;
    }
  }
  m_PointerArray[m_nFilledSize++] = itemPtr;
}

void DynamicPtrArray::AddFirst(void* itemPtr)
{
  if (m_nFilledSize >= m_nAllocatedSize) {
    // we need to grow the array
    void** temp = (void**)realloc(m_PointerArray, (m_nAllocatedSize + 100) * sizeof(void *));
    if (temp) {
      m_PointerArray = temp;
      m_nAllocatedSize = m_nAllocatedSize + 100;
    }
  }
  int i;
  for (i = m_nFilledSize; i > 0; i--) {
    m_PointerArray[i] = m_PointerArray[i-1];
  }
  m_PointerArray[0] = itemPtr;
  m_nFilledSize++;
}

void DynamicPtrArray::RemoveItem(int index)
{
  for (int i = index; i <m_nFilledSize-1; i++) {
    m_PointerArray[i] = m_PointerArray[i+1];
  }
  m_PointerArray[m_nFilledSize-1] = NULL;
  m_nFilledSize--;
}

// PriorityDynPtrArray routines

PriorityDynPtrArray::PriorityDynPtrArray(int queueLength)
{
  m_PointerArray = (PriorityItem*) malloc(queueLength * sizeof(PriorityItem));
  m_nAllocatedSize = queueLength;
  m_nFilledSize = 0;
}

PriorityDynPtrArray::~PriorityDynPtrArray()
{
  free(m_PointerArray);
}

void* PriorityDynPtrArray::Add(double priority, void* item)
{
  void* result = NULL;
  int i = 0;
  while ((i < m_nFilledSize) && (priority > m_PointerArray[i].priority)) {
    i++;
  }
  if (m_nFilledSize == m_nAllocatedSize) {
    // we're going to have to delete something
    if (i == m_nAllocatedSize)
      result = item;
    else
      result = m_PointerArray[m_nAllocatedSize-1].item;
  } else {
    m_nFilledSize++;
  }
  if (i < m_nAllocatedSize) {
    for (int j = m_nFilledSize-2; j >= i; j--) {
      m_PointerArray[j+1] = m_PointerArray[j];
    }
    m_PointerArray[i].priority = priority;
    m_PointerArray[i].item = item;
  } 
  return result;
}

void* PriorityDynPtrArray::GetNextItem()
{
  void* retVal = NULL;
  if (m_CurPointer < m_nFilledSize) {
    retVal = m_PointerArray[m_CurPointer].item;
    m_CurPointer++;
  }
  return retVal;
}

void* PriorityDynPtrArray::GetFirst()
{
  if (m_nFilledSize) 
    return m_PointerArray[0].item;
  else
    return NULL;
}

double PriorityDynPtrArray::GetFirstVal()
{
  return m_PointerArray[0].priority;
}

double PriorityDynPtrArray::GetLastVal()
{
  if (m_nFilledSize) {
    return m_PointerArray[m_nFilledSize-1].priority;
  } else {
    return 99999.9;
  }
}


// global utility routines

int EqualFillers(WordList* a, WordList* b)
{
  int eqFlag = 1;

  WordList* i = NULL;
  WordList* j = NULL;

  i = a;
  j = b;

  while (eqFlag && i && j) {
    eqFlag = i->word == j->word;
    i = i->next;
    j = j->next;
  }
  if (eqFlag && (i == j)) 
    return 1;
  else return 0;
}

int EqualFillerList(Filler* a, Filler* b)
{
  // Now the main issue here is that order doesn't matter, just that
  // all of the same elements be in the two lists
  Filler* c = CopyFillerList(a);
  
  Filler* i = b;
  int eqSoFar = 1;
  Filler* j = NULL;
  WordList* k;
  WordList* l;
  while (eqSoFar && i) {
    // see if i->filler is in c
    j = c;
    Filler* prev = NULL;
    int found = 0;
    while (j && !found) {
      if (EqualFillers(j->filler,i->filler)) {
	found = 1;
      } else {
	prev = j;
	j = j->next;
      }
    }
    if (found) {  // we found a match in j
      // remove j from c
      if (prev) {
	prev->next = j->next;
      } else {
	c = c->next;
      }
      // delete j and contents
      k = j->filler;
      while (k) {
	l = k->next;
	delete k;
	k = l;
      } 
      delete j;
      // increment i
      i = i->next;
    } else {
      eqSoFar = 0;
    }
  }
  // delete anything left in c
  j = c;
  while (j) {
    i = j->next;
    k = j->filler;
    while (k) {
      l = k->next;
      delete k;
      k = l;
    } 
    delete j;
    j = i;
  }
  return eqSoFar;
}

int WordListSubset(WordList* subset, WordList* set)
{
  // test to see whether every item in subset appears in set
  WordList* i = NULL;
  WordList* j = NULL;
  int result = 1;
  i = subset;
  int found = 0;

  while (result && i) {
    j = set;
    found = 0;
    while (!found && j) {
      if (i->word == j->word) {
	found = 1;
      } else j = j->next;
    }
    if (!found) result = 0;
    i = i->next;
  }
  return result;
}

WordList* CopyWordList(WordList* orig)
{
  if (NULL == orig) {
    return NULL;
  }
  WordList* i = NULL;
  WordList* j = NULL;
  WordList* k = NULL;
  WordList* l = NULL;


  i = new WordList;
  i->word = orig->word;
  i->prevText = orig->prevText;
  i->origText = orig->origText;
  l = i;
  k = orig->next;
  while (k) {
    j = new WordList;
    j->word = k->word;
    j->prevText = k->prevText;
    j->origText = k->origText;
    l->next = j;
    l = j;
    k = k->next;
  }
  l->next = NULL;
  return i;

}

Filler* CopyFillerList(Filler* orig)
{
  if (NULL == orig) {
    return NULL;
  }
  Filler* i = NULL;
  Filler* j = NULL;
  Filler* k = NULL;
  Filler* l = NULL;


  i = new Filler;
  i->filler = CopyWordList(orig->filler);
  l = i;
  k = orig->next;
  while (k) {
    j = new Filler;
    j->filler = CopyWordList(k->filler);
    l->next = j;
    l = j;
    k = k->next;
  }
  l->next = NULL;
  return i;
}


WordList* UnionWordLists(WordList* list1, WordList* list2)
{
  WordList* result = CopyWordList(list1);
  WordList* i = list2;
  WordList* j = NULL;
  while (i) {
    if (!WordListMember(i->word,list1)) {
      j = new WordList;
      j->word = i->word;
      j->prevText = i->prevText;
      j->origText = i->origText;
      j->next = result;
      result = j;
    }
    i = i->next;
  }
  return result;
}

void DestUnionWordLists(WordList*& list1, WordList* list2)
{
  WordList* temp = list1;
  WordList* i = list2;
  WordList* j = NULL;
  while (i) {
    if (!WordListMember(i->word,list1)) {
      j = new WordList;
      j->word = i->word;
      j->prevText = i->prevText;
      j->origText = i->origText;
      j->next = temp;
      temp = j;
    }
    i = i->next;
  }
  list1 = temp;
}

Filler* UnionFillerLists(Filler* list1, Filler* list2)
{
  if (!list1) {
    return CopyFillerList(list2);
  }
  Filler* result = CopyFillerList(list1);
  Filler* i = list2;
  Filler* j = NULL;
  while (i) {
    if (!FillerMember(i->filler,list1)) {
      j = new Filler;
      j->filler = CopyWordList(i->filler);
      j->next = result;
      result = j;
    }
    i = i->next;
  }
  return result;
}

Filler* IntersectionFillerLists(Filler* list1, Filler* list2)
{
  Filler* result = NULL;
  Filler* i = list2;
  Filler* j = NULL;
  while (i) {
    if (FillerMember(i->filler, list1)) {
      j = new Filler;
      j->filler = CopyWordList(i->filler);
      j->next = result;
      result = j;
    }
    i = i->next;
  }
  return result;
}

int WordListMember(char* word, WordList* list) 
{
  WordList* i = list;
  int found = 0;
  while (!found && i) {
    if (word == i->word) {
      found = 1;
    }
    i = i->next;
  }
  return found;
}

WordList* FillerMember(WordList* filler, Filler* fillerList)
{
  Filler* i = fillerList;
  WordList* found = NULL;
  while (!found && i) {
    if (EqualFillers(filler,i->filler)) {
      found = i->filler;
    }
      i = i->next;
  }
  return found;
}
  


