/*

Example of Skip List source code for C:

Skip Lists are a probabilistic alternative to balanced trees, as
described in the June 1990 issue of CACM and were invented by
William Pugh in 1987.

This file contains source code to implement a dictionary using
skip lists and a test driver to test the routines.

A couple of comments about this implementation:
  The routine randomLevel has been hard-coded to generate random
  levels using p=0.25. It can be easily changed.

  The insertion routine has been implemented so as to use the
  dirty hack described in the CACM paper: if a random level is
  generated that is more than the current maximum level, the
  current maximum level plus one is used instead.

  Levels start at zero and go up to MaxLevel (which is equal to
        (MaxNumberOfLevels-1).

The compile flag allowDuplicates determines whether or not duplicates
are allowed. If defined, duplicates are allowed and act in a FIFO manner.
If not defined, an insertion of a value already in the file updates the
previously existing binding.

BitsInRandom is defined to be the number of bits returned by a call to
random(). For most all machines with 32-bit integers, this is 31 bits
as currently set.

The routines defined in this file are:

  init: defines NIL and initializes the random bit source

  newList: returns a new, empty list

  freeList(l): deallocates the list l (along with any elements in l)

  randomLevel: Returns a random level

  insert(l,key,value): inserts the binding (key,value) into l. If
        allowDuplicates is undefined, returns true if key was newly
        inserted into the list, false if key already existed

  delete_(l,key): delete_s any binding of key from the l. Returns
        false if key was not defined.

  search(l,key,&value): Searches for key in l and returns true if found.
        If found, the value associated with key is stored in the
        location pointed to by &value

	*/
    
extern "C" {
 
// NEW
#include <stdlib.h>
#include <stdio.h>
#include "limits.h"

#define false 0
#define true 1
typedef char boolean;

// NEW
#define BitsInRandom 15

#define allowDuplicates

#define MaxNumberOfLevels 16
#define MaxLevel (MaxNumberOfLevels-1)
#define newNodeOfLevel(l) (node)malloc(sizeof(struct nodeStructure)+(l)*sizeof(node *))

typedef int keyType;
typedef int valueType;


typedef struct nodeStructure *node;

typedef struct nodeStructure{
         keyType key;
         valueType value;
         node forward[1]; /* variable sized array of forward pointers */
};

typedef struct listStructure{
        int level;        /* Maximum level of the list
                        (1 more than the number of levels in the list) */
        struct nodeStructure * header; /* pointer to header */
} * list;

node NIL;


#ifdef allowDuplicates
boolean delete_(list l, keyType key);
boolean search(list l, keyType key, valueType * valuePointer);
void insert(list l, keyType key,valueType value);
#else
boolean insert(list l, keyType key,valueType value), delete_(list l, keyType key), search(list l, keyType key, valueType * valuePointer);
#endif

int randomsLeft;
int randomBits;

void init() {
  NIL = newNodeOfLevel(0);

  // NEW
  NIL->key = INT_MAX;
  randomBits = rand();

  randomsLeft = BitsInRandom/2;
}

list newList() {
  list l;
  int i;

  l = (list)malloc(sizeof(struct listStructure));
  l->level = 0;
  l->header = newNodeOfLevel(MaxNumberOfLevels);
  for(i=0;i<MaxNumberOfLevels;i++) l->header->forward[i] = NIL;
  return(l);
}

void freeList(list l)
{
  register node p,q;
  p = l->header;
  do {
         q = p->forward[0];
    free(p);
    p = q; }
    while (p!=NIL);
  free(l);
}

int randomLevel()
{register int level = 0;
   register int b;
   do {
    b = randomBits&3;
         if (!b) level++;
	 randomBits>>=2;
         if (--randomsLeft == 0) {

	   //NEW
                  randomBits = rand();

        randomsLeft = BitsInRandom/2;
	 }
   } while (!b);
   return(level>MaxLevel ? MaxLevel : level);
}

#ifdef allowDuplicates
void insert(list l, keyType key,valueType value)
#else
boolean insert(list l, keyType key,valueType value)
#endif
{
  register int k;
  node update[MaxNumberOfLevels];
  register node p,q;

  p = l->header;
  k = l->level;
  do {
        while (q = p->forward[k], q->key < key) p = q;
        update[k] = p;
  } while(--k>=0);

#ifndef allowDuplicates
  if (q->key == key) {
        q->value = value;
        return(false);
  }
#endif

         k = randomLevel();
	 if (k>l->level) {
        k = ++l->level;
        update[k] = l->header;
	 }
    q = newNodeOfLevel(k);
    q->key = key;
    q->value = value;
    do {
        p = update[k];
        q->forward[k] = p->forward[k];
        p->forward[k] = q;
    } while(--k>=0);
#ifndef allowDuplicates
    return(true);
#endif
}

boolean delete_(list l, keyType key)
{
  register int k,m;
  node update[MaxNumberOfLevels];
  register node p,q;

  p = l->header;
  k = m = l->level;
  do {
        while (q = p->forward[k], q->key < key) p = q;
        update[k] = p;
  } while(--k>=0);

  if (q->key == key) {
        for(k=0; k<=m && (p=update[k])->forward[k] == q; k++)
          p->forward[k] = q->forward[k];
        free(q);
        while( l->header->forward[m] == NIL && m > 0 )
             m--;
        l->level = m;
        return(true);
  }
    else return(false);
}

boolean search(list l, keyType key, valueType * valuePointer)
{
  register int k;
  register node p,q;
  p = l->header;
  k = l->level;
  do while (q = p->forward[k], q->key < key) p = q;
      while (--k>=0);
  if (q->key != key) return(false);
  *valuePointer = q->value;
  return(true);
}

// NEW
#define sampleSize 100
/**
int main() {
  list l;
  register int i,k;
  keyType keys[sampleSize];
  valueType v;

  init();

  l= newList();

  for(k=0;k<sampleSize;k++) {

    // NEW
                keys[k]=rand();

                insert(l,keys[k],keys[k]);
  }

  printf("START\n");

  for(i=0;i<4;i++) {
    for(k=0;k<sampleSize;k++) {
                 if (!search(l,keys[k],&v)) printf("error in search#%d,#%d\n",i,k);
                 if (v != keys[k]) printf("search returned wrong value\n");
    }
    for(k=0;k<sampleSize;k++) {
                 if (! delete_(l,keys[k])) printf("error in delete_\n");

		 // NEW
                                keys[k] = rand();

                 insert(l,keys[k],keys[k]);
    }
  }

  printf("STOP\n");

  freeList(l);

  return 0;
}
**/

}


