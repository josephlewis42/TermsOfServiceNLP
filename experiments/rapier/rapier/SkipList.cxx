//-*-C++-*-

#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "utils.h"
#include "SkipList.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <ios>
using namespace std;

const int BitsInRandom = 15;

const int MaxNumberOfLevels = 16;
const int MaxLevel = MaxNumberOfLevels-1;

inline SkipListNode* newNodeOfLevel(int l) 
{
  return (SkipListNode *)malloc(sizeof(SkipListNode)+(l)*sizeof(SkipListNode **)); 
}

int randomsLeft;
int randomBits;

void InitSkipLists() {

  randomBits = rand();

  randomsLeft = BitsInRandom/2;
}

// SkipList routines

SkipList::SkipList() 
{
  m_nLevel = 0;
  m_ListHead = new SkipListNode; //newNodeOfLevel(MaxNumberOfLevels);
  m_ListHead->value = NULL;
  for (int i = 0; i < MaxNumberOfLevels; i++)
    m_ListHead->forward[i] = NULL;
}

SkipList::~SkipList() 
{
  SkipListNode* p = NULL;
  SkipListNode* q = NULL;
  
  p = m_ListHead;

  while (p) {
    q = p->forward[0];
    delete p->value;
    delete p;
    p = q;
  }
}


void SkipList::Insert(char* key, void* value)
{
  register int k;
  SkipListNode* update[MaxNumberOfLevels];
  SkipListNode* p = NULL;
  SkipListNode* q = NULL;
  for (int i = 0; i < MaxNumberOfLevels; i++) 
    update[i] = NULL;

  p = m_ListHead;
  k = m_nLevel;
  do {
        while ((q = p->forward[k]) && (0 <= strcmp(q->key, key))) p = q;
        update[k] = p;
  } while(--k>=0);

  k = this->RandomLevel();
  if (k>m_nLevel) {
    k = m_nLevel + 1;
    update[k] = m_ListHead;
  }

  q = new SkipListNode; //newNodeOfLevel(k);
  q->key = key;
  q->value = value;
  do {
    p = update[k];
    q->forward[k] = p->forward[k];
    p->forward[k] = q;
  } while(--k>=0);
}

int SkipList::Delete(char* key)
{
  int k,m;
  SkipListNode* update[MaxNumberOfLevels];
  SkipListNode* p = NULL;
  SkipListNode* q = NULL;
  for (int i = 0; i < MaxNumberOfLevels; i++)
    update[i] = NULL;

  p = m_ListHead;
  k = m_nLevel;
  m = m_nLevel;

  do {
        while ((q = p->forward[k]) && (0 < strcmp(q->key, key))) p = q;
        update[k] = p;
  } while(--k>=0);

  if (q->key == key) {
    SkipListNode* r = q;
    while(r && (r->key == key)) {
      for(k=0; k<=m && (p=update[k])->forward[k] == r; k++)
	p->forward[k] = r->forward[k];
      q = r->forward[0];
      delete r;
      r = q;
    }
    while( (m_ListHead->forward[m] == NULL) && (m > 0) )
      m--;
    m_nLevel = m;
    return(true);
  } else return(false);
}

int SkipList::Delete(char* key, void* value)
{
  int k,m;
  SkipListNode* update[MaxNumberOfLevels];
  SkipListNode* p = NULL;
  SkipListNode* q = NULL;
  for (int i = 0; i < MaxNumberOfLevels; i++)
    update[i] = NULL;

  p = m_ListHead;
  k = m_nLevel;

  do {
        while ((q = p->forward[k]) && (0 < strcmp(q->key, key))) p = q;
	while ((q = p->forward[k]) && (0 == strcmp(q->key, key)) &&
	       (q->value != value)) p = q;
        update[k] = p;
  } while(--k>=0);

  if ((q->key == key) && (q->value == value)) {
    for(k=0; k<=m && (p=update[k])->forward[k] == q; k++)
      p->forward[k] = q->forward[k];
    delete q;
    while( m_ListHead->forward[m] == NULL && m > 0 )
      m--;
    m_nLevel = m;
    return(true);
  } else 
    return(false);
}


DynamicPtrArray* SkipList::Search(char* key)
{
  int k;
  SkipListNode* p = NULL;
  SkipListNode* q = NULL;
  p = m_ListHead;
  k = m_nLevel;

  do 
    while ((q = p->forward[k]) && (0 < strcmp(q->key, key))) p = q;
      while (--k>=0);
  if (!q) return NULL;
  if (q->key != key) return(NULL);

  // create a new DynamicPtrArray, and collect the values in it
  DynamicPtrArray* valueArray = new DynamicPtrArray;
  while (q && (q->key == key)) {
    valueArray->Add(q->value);
    q = q->forward[0];
  }

  return valueArray;
}

void SkipList::InitializeIterator()
{
  m_IterPtr = m_ListHead;
}


void* SkipList::GetNextItem()
{
  m_IterPtr = m_IterPtr->forward[0];
  if (m_IterPtr) {
    return m_IterPtr->value;
  } else {
    return NULL;
  }
}


int SkipList::RandomLevel()
{
  int level = 0;
  int b;
   do {
    b = randomBits&3;

         if (!b) level++;
	 randomBits>>=2;
         if (--randomsLeft <= 0) {

	   //NEW
                  randomBits = rand();

		  randomsLeft = BitsInRandom/2;
	 }
   } while (!b);
   return(level>MaxLevel ? MaxLevel : level);
}







